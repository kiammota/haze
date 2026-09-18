#include "HazeServer.h"
#include "Context.h"
#include "HazeServerDispatcher.h"
#include "RawBuffer.h"
#include "logc/log.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include "stddef.h"

/* ---------------------------------------------------------- */
/* Conexão individual                                         */
/* ---------------------------------------------------------- */

typedef struct {
  uv_tcp_t handle;

  // Buffer persistente para lidar com framing TCP
  char *buffer;
  size_t buffer_len;
  size_t buffer_cap;
  const Context* ctx;
} HazeConn;

typedef struct {
  uv_write_t req;
  void *data_to_free;
  uv_stream_t *stream;
} haze_write_req_t;

static void haze_on_close(uv_handle_t *handle) {
  if (handle->data) {
    HazeConn *conn = (HazeConn *)handle->data;
    handle->data = NULL;

    if (conn->buffer) {
      free(conn->buffer);
    }
    free(conn);
  }
}

static void haze_on_alloc(uv_handle_t *handle, size_t suggested,
                          uv_buf_t *buf) {
  (void)handle;
  buf->base = malloc(suggested);
  buf->len = buf->base ? (unsigned int)suggested : 0;
}

static void haze_on_write_done(uv_write_t *req, int status) {
  haze_write_req_t *wr = (haze_write_req_t *)req;

  if (status < 0) {
    if (wr->stream && !uv_is_closing((uv_handle_t *)wr->stream)) {
      uv_close((uv_handle_t *)wr->stream, haze_on_close);
    }
  }

  if (wr->data_to_free) {
    free(wr->data_to_free);
  }

  free(wr);
}

static void _OnSignal(uv_signal_t *handle, int signum) {
  // Apenas o %d correspondente ao signum
  log_info("Signal %d received, stopping loop...", signum);

  if (handle && handle->loop) {
    uv_stop(handle->loop);
  }
}

int HazeServerSetupSignals(HazeServer *server) {
  if (!server)
    return -1;

  uv_signal_t *sig = malloc(sizeof(uv_signal_t));
  if (!sig)
    return -1;

  sig->data = (void *)1; // Marca o handle como alocado dinamicamente

  uv_signal_init(server->loop, sig);
  uv_signal_start(sig, _OnSignal, SIGINT);

  return 0;
}

void haze_send(uv_stream_t *stream, const void *data, size_t len) {
  haze_write_req_t *req = malloc(sizeof(*req));
  if (!req)
    return;

  void *data_copy = malloc(len);
  if (!data_copy) {
    free(req);
    return;
  }

  memcpy(data_copy, data, len);

  req->data_to_free = data_copy;
  req->stream = stream;

  uv_buf_t buf = uv_buf_init((char *)data_copy, (unsigned int)len);

  int rc = uv_write((uv_write_t *)req, stream, &buf, 1, haze_on_write_done);

  if (rc < 0) {
    free(req->data_to_free);
    free(req);
  }
}

static void haze_on_read(uv_stream_t *stream, ssize_t nread,
                         const uv_buf_t *buf) {
  HazeConn *conn = (HazeConn *)stream->data;

  if (nread < 0) {
    if (nread != UV_EOF) {
      log_warn("Read error: %s", uv_err_name((int)nread));
    }

    free(buf->base);
    uv_close((uv_handle_t *)stream, haze_on_close);
    return;
  }

  if (nread == 0) {
    free(buf->base);
    return;
  }

  size_t new_len = conn->buffer_len + (size_t)nread;

  if (new_len > conn->buffer_cap) {
    size_t new_cap = conn->buffer_cap == 0 ? 1024 : conn->buffer_cap;

    while (new_cap < new_len)
      new_cap *= 2;

    char *new_buffer = realloc(conn->buffer, new_cap);

    if (!new_buffer) {
      log_error("ON READ", "Failed to resize connection buffer");

      free(buf->base);
      uv_close((uv_handle_t *)stream, haze_on_close);
      return;
    }

    conn->buffer = new_buffer;
    conn->buffer_cap = new_cap;
  }

  memcpy(conn->buffer + conn->buffer_len, buf->base, (size_t)nread);

  conn->buffer_len = new_len;

  free(buf->base);

  /*
   * O servidor não interpreta o conteúdo.
   * Apenas entrega os bytes recebidos para a API.
   */
  while (conn->buffer_len > 0) {
    RawBuffer buffer = RawBufferInit(conn->buffer, conn->buffer_len);

    RawBuffer *response = HazeServerAPIDispatcher(conn->ctx, &buffer);

    if (!response) {
      
      fflush(stdout);
      log_error("critical fail on msgpack-rpc parser. client disconnected.");
      uv_close((uv_handle_t *)stream, haze_on_close);
      break;
    }

    haze_send(stream, RawBufferData(response), RawBufferLen(response));

    RawBufferFree(&response);

    size_t consumed = buffer.len;

    if (consumed == 0 || consumed > conn->buffer_len) {
      log_error("ON READ", "Invalid API buffer consumption");

      uv_close((uv_handle_t *)stream, haze_on_close);
      break;
    }

    conn->buffer_len -= consumed;

    if (conn->buffer_len > 0) {
      memmove(conn->buffer, conn->buffer + consumed, conn->buffer_len);
    }
  }

  if (conn->buffer_len > 4 * 1024 * 1024) {
    log_error("SERVER", "Connection buffer exceeded 4 MB");

    uv_close((uv_handle_t *)stream, haze_on_close);
  }
}
static void haze_on_connect(uv_stream_t *server, int status) {
  if (status < 0) {
    log_error("ON CONNECT", "connect error: %s", uv_strerror(status));
    return;
  }

  HazeServer *haze = (HazeServer *)server->data;

  HazeConn *conn = calloc(1, sizeof(HazeConn));
  if (!conn)
    return;

  conn->ctx = haze->ctx;

  int init_ret = uv_tcp_init(server->loop, &conn->handle);
  if (init_ret != 0) {
    free(conn);
    return;
  }

  conn->handle.data = conn;

  int accept_ret =
      uv_accept(server, (uv_stream_t *)&conn->handle);

  if (accept_ret != 0) {
    uv_close(
        (uv_handle_t *)&conn->handle,
        haze_on_close
    );
    return;
  }

  uv_tcp_nodelay(&conn->handle, 1);

  int read_ret = uv_read_start(
      (uv_stream_t *)&conn->handle,
      haze_on_alloc,
      haze_on_read
  );

  if (read_ret != 0) {
    uv_close(
        (uv_handle_t *)&conn->handle,
        haze_on_close
    );
  }
}

/* ---------------------------------------------------------- */
/* API pública                                                */
/* ---------------------------------------------------------- */

HazeServer *HazeServerNew(const char *addr, uint16_t port) {
  HazeServer *s = calloc(1, sizeof(HazeServer));
  if (!s)
    return NULL;

  s->loop = uv_loop_new();
  s->port = port;
  s->addr = strdup(addr ? addr : "127.0.0.1");

  uv_tcp_init(s->loop, &s->tcp);

  s->tcp.data = s;

  return s;
}

int HazeServerStart(const Context *ctx, HazeServer *s) {
  if (!ctx || !s)
    return UV_EINVAL;

  s->ctx = ctx;

  struct sockaddr_in bind_addr;
  uv_ip4_addr(s->addr, s->port, &bind_addr);

  int r = uv_tcp_bind(
      &s->tcp,
      (const struct sockaddr *)&bind_addr,
      0
  );

  if (r != 0)
    return r;

  return uv_listen(
      (uv_stream_t *)&s->tcp,
      512,
      haze_on_connect
  );
}

void HazeServerRun(HazeServer *s) {
  if (!s)
    return;
  uv_run(s->loop, UV_RUN_DEFAULT);
}

void HazeServerStop(HazeServer *s) {
  if (!s)
    return;
  uv_stop(s->loop);
}

static void _OnHandleClose(uv_handle_t *handle) {
  if (handle && handle->data == (void *)1) {
    free(handle); // Libera apenas se foi alocado separadamente via malloc
  }
}

static void _CloseWalkCb(uv_handle_t *handle, void *arg) {
  (void)arg;
  if (!uv_is_closing(handle)) {
    uv_close(handle, _OnHandleClose);
  }
}

void HazeServerFree(HazeServer **server_ptr) {
  if (!server_ptr || !*server_ptr)
    return;

  HazeServer *server = *server_ptr;

  if (server->loop) {
    uv_walk(server->loop, _CloseWalkCb, NULL);

    while (uv_loop_close(server->loop) == UV_EBUSY) {
      uv_run(server->loop, UV_RUN_ONCE);
    }

    free(server->loop);
    server->loop = NULL;
  }

  if (server->addr) { // Altere para o nome do campo se for server->host ou
                      // similar
    free((void *)server->addr);
    server->addr = NULL;
  }

  free(server);
  
  *server_ptr = NULL;
}

uint16_t HazeServerPort(HazeServer *s) {
  if (!s)
    return 0;
  return s->port;
}

const char *HazeServerAddress(HazeServer *s) {
  if (!s)
    return NULL;
  return s->addr;
}

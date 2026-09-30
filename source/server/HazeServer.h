#ifndef SERVER_DEC
#define SERVER_DEC

#include "Context.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <uv.h>

typedef struct HazeServer HazeServer;

typedef struct {
  uv_tcp_t handle;

  char *buffer;
  size_t buffer_len;
  size_t buffer_cap;
  const Context *ctx;
  HazeServer *server;
} HazeConn;

struct HazeServer {
  char *addr;
  uint16_t port;
  uv_loop_t *loop;
  uv_tcp_t tcp;
  const Context *ctx;

  HazeConn **connections;
  size_t conn_len;
  size_t conn_cap;
};

HazeServer *HazeServerNew(const char *addr, uint16_t port);
int HazeServerAddConnection(HazeServer *s, HazeConn *c);
void HazeServerRemoveConnection(HazeServer *s, HazeConn *c);
void HazeServerBroadcast(HazeServer *src, const void *data, size_t size);
void HazeServerFree(HazeServer **s);
int HazeServerStart(const Context *ctx, HazeServer *s);
void HazeServerStop(HazeServer *s);
void HazeServerRun(HazeServer *s);
uint16_t HazeServerPort(HazeServer *s);
const char *HazeServerAddress(HazeServer *s);
int HazeServerSetupSignals(HazeServer *s);

#endif

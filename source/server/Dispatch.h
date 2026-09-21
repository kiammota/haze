#ifndef DISPATCH
#define DISPATCH

#include "Context.h"
#include "RawBuffer.h"

typedef struct {
  bool ok;
  RawBuffer *response;
  RawBuffer *notification;
} DispatchBytes;


  DispatchBytes *ServerDispatch(const Context *ctx, const uv_tcp_t *connection,
                          RawBuffer *buffer);
#endif

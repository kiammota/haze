#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "Context.h"
#include "api/proto/Notification.h"
#include "api/proto/Request.h"
#include "api/proto/Response.h"
#include "uv.h"

typedef struct {
  Response *response;
  Notification *notification;
} DispatchResult;

DispatchResult DispatchRPCMessage(Context *ctx, uv_tcp_t *connection,
                                   Request *rq);

#endif

/*  ----------------
Protocol implementations follow the MSGPACK-RPC convention. 
Consult the specification for further details at:

https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md
-------------- */

#ifndef PROTO_NOTIFICATION_H
#define PROTO_NOTIFICATION_H

#include "msgpack/MessagePackRPC.h"
#include "RawBuffer.h"
#include "msgpack/Object.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  HazeServerRPCType type;   
  char *method;
  ObjectArray *params;
} Notification;

Notification *NotificationNew(void);
void NotificationFree(Notification **n);

void NotificationSetMethod(Notification *n, const char *method);

RawBuffer *NotificationMarshal(Notification *n);
Notification *NotificationUnmarshal(RawBuffer *b);

static inline const char *NotificationMethod(const Notification *n) {
  return n ? n->method : NULL;
}

static inline const ObjectArray *NotificationParams(const Notification *n) {
  return n ? n->params : NULL;
}

Notification *NotificationCreate(const char *method, Object **params, size_t param_count);

#endif 

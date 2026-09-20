/* HazeServerRequest.h */
#ifndef HAZE_SERVER_REQUEST_H
#define HAZE_SERVER_REQUEST_H

#include "msgpack/MessagePackRPC.h"
#include "RawBuffer.h"
#include "msgpack/Object.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


typedef struct {
  HazeServerRPCType type;
  uint32_t msgid;
  char *method;
  ObjectArray *parameters;
} Request;

Object* RequestParamGet(const Request *rq, uint32_t index);
bool RequestParamAppend(Request *rq, Object *param, uint32_t ind);
Object* RequestParamNew(void);
void RequestParamFree(Object** r);
static inline ObjectType RequestParamTypeGet(const Object* r) {
  return r->type;
}
static inline ObjectValue RequestParamValueGet(const Object* r) {
  return r->value;
}
Request *RequestUnmarshal(RawBuffer *b);
RawBuffer *RequestMarshal(Request *r);
Request *RequestNew(void);

void RequestSetMethod(Request *request, const char *method);

static inline uint32_t RequestMsgId(const Request *r) { return r->msgid; }
static inline const char *RequestMethod(const Request *r) { return r->method; }

void RequestFree(Request **request);

uint32_t RequestParamCount(const Request *r);

bool RequestParamIsType(const Request *r, ObjectType type,
                        uint32_t index);

const char* RequestPrint(const Request* r);

#endif

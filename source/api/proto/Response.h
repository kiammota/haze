#ifndef PROTO_RESPONSE_H
#define PROTO_RESPONSE_H

#include "RawBuffer.h"
#include "Result.h"
#include "audio/ResultAudio.h"
#include "msgpack/MessagePackRPC.h"
#include "msgpack/Object.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  HazeServerRPCType type;
  uint32_t msgid;

  Object *error;
  Object *result;
} Response;

Response *ResponseNew(void);
RawBuffer *ResponseMarshal(Response *s);
Response *ResponseUnmarshal(RawBuffer *b);

static inline const Object *ResponseResult(const Response *r) {
  return r ? r->result : NULL;
}

static inline const Object *ResponseError(const Response *r) {
  return r ? r->error : NULL;
}

static inline uint32_t ResponseMsgId(const Response *r) {
  return r ? r->msgid : 0;
}

bool ResponseFree(Response **response);

/* Setters genéricos para Object */
bool ResponseSetResultObject(Response *s, Object *result);
bool ResponseSetErrorObject(Response *s, Object *error);

bool ResponseSetMsgId(Response *s, uint32_t msgid);

/* Constructors utilitários */
Response *ResponseCreateString(uint32_t msgid, const char *result);
Response *ResponseCreateStrArrayResult(uint32_t msgid, const char **vec);
Response *ResponseCreateError(uint32_t msgid, const char *err);
Response *ResponseCreateNilResult(uint32_t msgid);
Response *ResponseCreateOk(uint32_t msgid);
Response *ResponseCreateResult(uint32_t msgid, Result res);
Response *ResponseCreateResultAudio(uint32_t msgid, ResultAudio res);
Response *ResponseCreateInt(uint32_t msgid, int64_t value);
char *ResponseToString(const Response *res);

#endif /* PROTO_RESPONSE_H */

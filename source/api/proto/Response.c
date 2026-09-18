#include "Response.h"
#include "RawBuffer.h"
#include "Result.h"
#include "logc/log.h"
#include "mpack/mpack-common.h"
#include "mpack/mpack-expect.h"
#include "mpack/mpack-reader.h"
#include "mpack/mpack-writer.h"
#include "msgpack/MessagePackRPC.h"
#include "msgpack/Object.h"
#include "strdup.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper estático para serializar qualquer Object no MPack Writer */
static void ObjectMarshalMPack(mpack_writer_t *writer, const Object *obj) {
  if (!obj) {
    mpack_write_nil(writer);
    return;
  }

  switch (obj->type) {
  case OBJ_NIL:
    mpack_write_nil(writer);
    break;

  case OBJ_BOOL:
    mpack_write_bool(writer, obj->value.bool_value);
    break;

  case OBJ_INT:
    mpack_write_int(writer, obj->value.int_value);
    break;

  case OBJ_UINT:
    mpack_write_uint(writer, obj->value.uint_value);
    break;

  case OBJ_FLOAT:
    mpack_write_float(writer, obj->value.float_value);
    break;

  case OBJ_DOUBLE:
    mpack_write_double(writer, obj->value.double_value);
    break;

  case OBJ_STR:
    if (obj->value.str_value) {
      mpack_write_str(writer, obj->value.str_value, (uint32_t)obj->size);
    } else {
      mpack_write_nil(writer);
    }
    break;

  case OBJ_BIN:
    if (obj->value.bin_value && RawBufferData(obj->value.bin_value)) {
      mpack_write_bin(writer, (const char *)RawBufferData(obj->value.bin_value),
                      (uint32_t)RawBufferLen(obj->value.bin_value));
    } else {
      mpack_write_nil(writer);
    }
    break;

  case OBJ_ARRAY: {
    ObjectArray *arr = obj->value.array_value;
    if (!arr) {
      mpack_write_nil(writer);
      break;
    }
    size_t len = ObjectArrayLen(arr);
    mpack_start_array(writer, (uint32_t)len);
    for (size_t i = 0; i < len; i++) {
      ObjectMarshalMPack(writer, ObjectArrayGet(arr, i));
    }
    mpack_finish_array(writer);
    break;
  }

  case OBJ_UND:
  default:
    mpack_write_nil(writer);
    break;
  }
}

/* Helper estático para deserializar uma tag MPack para um Object */
static Object *ObjectUnmarshalMPack(mpack_reader_t *reader) {
  mpack_tag_t tag = mpack_read_tag(reader);
  if (mpack_reader_error(reader) != mpack_ok) {

    return NULL;
  }

  switch (mpack_tag_type(&tag)) {
  case mpack_type_nil:
    return ObjectCreateNil();

  case mpack_type_bool:
    return ObjectCreateBool(mpack_tag_bool_value(&tag));

  case mpack_type_int:
    return ObjectCreateInt(mpack_tag_int_value(&tag));

  case mpack_type_uint:
    return ObjectCreateUInt(mpack_tag_uint_value(&tag));

  case mpack_type_float:
    return ObjectCreateFloat(mpack_tag_float_value(&tag));

  case mpack_type_double:
    return ObjectCreateDouble(mpack_tag_double_value(&tag));

  case mpack_type_str: {
    uint32_t len = mpack_tag_str_length(&tag);
    char *buff = malloc(len + 1);
    if (!buff)
      return NULL;
    mpack_read_bytes(reader, buff, len);
    buff[len] = '\0';
    mpack_done_str(reader);
    Object *obj = ObjectCreateStr(buff);
    free(buff);
    return obj;
  }

  case mpack_type_bin: {
    uint32_t len = mpack_tag_bin_length(&tag);
    void *buff = malloc(len);
    if (!buff)
      return NULL;
    mpack_read_bytes(reader, buff, len);
    mpack_done_bin(reader);

    Object *obj = ObjectNew();
    if (obj) {
      obj->type = OBJ_BIN;
      obj->value.bin_value = RawBufferNew(buff, len);
      obj->size = len;
    }
    free(buff);
    return obj;
  }

  case mpack_type_array: {
    uint32_t count = mpack_tag_array_count(&tag);
    ObjectArray *arr = ObjectArrayCreate();
    for (uint32_t i = 0; i < count; i++) {
      Object *child = ObjectUnmarshalMPack(reader);
      if (child) {
        ObjectArrayAppend(arr, child);
      }
    }
    mpack_done_array(reader);

    Object *obj = ObjectNew();
    if (obj) {
      obj->type = OBJ_ARRAY;
      obj->value.array_value = arr;
    }
    return obj;
  }

  default:
    return ObjectCreateNil();
  }
}

Response *ResponseNew(void) {
  Response *response = calloc(1, sizeof(Response));
  if (!response)
    return NULL;

  response->type = HAZE_RPC_RESPONSE;
  response->msgid = 0;
  response->error = NULL;
  response->result = NULL;

  return response;
}

bool ResponseSetResultObject(Response *s, Object *result) {
  if (!s)
    return false;

  if (s->result) {
    ObjectFree(&s->result);
  }
  s->result = result;
  return true;
}

bool ResponseSetErrorObject(Response *s, Object *error) {
  if (!s)
    return false;

  if (s->error) {
    ObjectFree(&s->error);
  }
  s->error = error;
  return true;
}

bool ResponseSetMsgId(Response *s, uint32_t msgid) {
  if (!s)
    return false;
  s->msgid = msgid;
  return true;
}

RawBuffer *ResponseMarshal(Response *s) {
  if (!s)
    return NULL;

  char *data = NULL;
  size_t size = 0;

  mpack_writer_t writer;
  mpack_writer_init_growable(&writer, &data, &size);

  /* Formato MsgPack-RPC Response: [type, msgid, error, result] */
  mpack_start_array(&writer, 4);

  mpack_write_u8(&writer, (uint8_t)s->type);
  mpack_write_u32(&writer, s->msgid);

  /* Error */
  ObjectMarshalMPack(&writer, s->error);

  /* Result */
  ObjectMarshalMPack(&writer, s->result);

  mpack_finish_array(&writer);

  mpack_error_t error = mpack_writer_destroy(&writer);
  if (error != mpack_ok) {
    log_error("ResponseMarshal failed: %s", mpack_error_to_string(error));
    if (data) {
      free(data);
    }
    return NULL;
  }

  RawBuffer *buffer = RawBufferNew(data, size);
  free(data);

  return buffer;
}

Response *ResponseUnmarshal(RawBuffer *b) {
    if (!b || RawBufferLen(b) == 0)
        return NULL;

    mpack_reader_t reader;
    mpack_reader_init_data(
        &reader,
        (const char *)RawBufferData(b),
        RawBufferLen(b)
    );

    Response *response = NULL;

    uint32_t count = mpack_expect_array(&reader);
    if (mpack_reader_error(&reader) != mpack_ok || count != 4)
        goto fail;

    response = ResponseNew();
    if (!response)
        goto fail;

    /* 1. type */
    response->type = (HazeServerRPCType)mpack_expect_u8(&reader);
    if (mpack_reader_error(&reader) != mpack_ok ||
        response->type != HAZE_RPC_RESPONSE)
        goto fail;

    /* 2. msgid */
    response->msgid = mpack_expect_u32(&reader);
    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    /* 3. error */
    response->error = ObjectUnmarshalMPack(&reader);
    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    if (response->error &&
        ObjectGetType(response->error) == OBJ_NIL) {
        ObjectFree(&response->error);
        response->error = NULL;
    }

    /* 4. result */
    response->result = ObjectUnmarshalMPack(&reader);
    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    if (response->result &&
        ObjectGetType(response->result) == OBJ_NIL) {
        ObjectFree(&response->result);
        response->result = NULL;
    }

    mpack_done_array(&reader);
    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    mpack_reader_destroy(&reader);
    return response;

fail:
    mpack_reader_destroy(&reader);

    if (response)
        ResponseFree(&response);

    return NULL;
}

bool ResponseFree(Response **response) {
  if (!response || !*response)
    return false;

  if ((*response)->error) {
    ObjectFree(&(*response)->error);
  }

  if ((*response)->result) {
    ObjectFree(&(*response)->result);
  }

  free(*response);
  *response = NULL;

  return true;
}

Response *ResponseCreateString(uint32_t msgid, const char *result) {
  Response *resp = ResponseNew();
  if (!resp)
    return NULL;

  ResponseSetMsgId(resp, msgid);
  ResponseSetResultObject(resp, ObjectCreateStr(result));

  return resp;
}

Response *ResponseCreateError(uint32_t msgid, const char *err) {
  Response *resp = ResponseNew();
  if (!resp)
    return NULL;

  resp->type = HAZE_RPC_RESPONSE;
  resp->msgid = msgid;
  if (err) {
    resp->error = ObjectCreateStr(err);
  } else {
    resp->error = ObjectCreateNil();
  }
  return resp;
}

Response *ResponseCreateStrArrayResult(uint32_t msgid, const char **values) {
  if (!values)
    return NULL;

  Response *resp = ResponseNew();
  if (!resp)
    return NULL;

  ObjectArray *arr = ObjectArrayCreate();
  if (!arr) {
    free(resp);
    return NULL;
  }

  size_t count = 0;
  while (values[count]) {
    ObjectArrayAppend(arr, ObjectCreateStr(values[count]));
    count++;
  }

  Object *array_obj = ObjectNew();
  if (!array_obj) {
    ObjectArrayFree(&arr);
    free(resp);
    return NULL;
  }
  array_obj->type = OBJ_ARRAY;
  array_obj->value.array_value = arr;

  ResponseSetMsgId(resp, msgid);
  ResponseSetResultObject(resp, array_obj);

  return resp;
}

Response *ResponseCreateNilResult(uint32_t msgid) {
  Response *resp = ResponseNew();
  ResponseSetMsgId(resp, msgid);
  ResponseSetResultObject(resp, ObjectCreateNil());
  return resp;
}

Response *ResponseCreateResult(uint32_t msgid, Result res) {
  Response *resp = ResponseNew();
  if (!resp)
    return NULL;

  ResponseSetMsgId(resp, msgid);

  if (ResultIsOk(res)) {
    ResponseSetResultObject(resp, ObjectCreateNil());
    return resp;
  }

  ResponseSetResultObject(resp, ObjectCreateStr(res.msg));
  return resp;
}

Response *ResponseCreateInt(uint32_t msgid, int64_t value) {
  Response *resp = ResponseNew();
  if (!resp)
    return NULL;

  ResponseSetMsgId(resp, msgid);

  ResponseSetResultObject(resp, ObjectCreateInt(value));
  return resp;
}

Response *ResponseCreateResultAudio(uint32_t msgid, ResultAudio res) {
  Response *r = res.success ? ResponseCreateNilResult(msgid)
                            : ResponseCreateError(msgid, res.msg);

  ResultAudioFree(&res);
  return r;
}

char *ResponseToString(const Response *res) {
    char *responseStr = (char *)malloc(1);

    char *strError = StrDup(ObjectGetValue(ResponseError(res)).str_value);
    char *strResult = NULL;

    const Object *resultObj = ResponseResult(res);

    switch (ObjectGetType(resultObj)) {
    case OBJ_NIL:
        strResult = StrDup("nil");
    case OBJ_STR:
        strResult = StrDup(ObjectGetValue(resultObj).str_value);
        break;
    case OBJ_UND:
        strResult = StrDup("und");
        break;
    case OBJ_BOOL:
        strResult = StrDup(
            ObjectGetValue(resultObj).bool_value ? "true" : "false"
        );
    case OBJ_DOUBLE:
    case OBJ_FLOAT:
    case OBJ_INT:
    case OBJ_UINT:
        strResult = StrDup("some integer");
        break;
    case OBJ_BIN:
        strResult = StrDup("<binary_encoded");
        break;
    default:
        strResult = StrDup("<undefined>");
    }

    sprintf(
        responseStr,
        "[%d,%d,%s,%s]",
        HAZE_RPC_RESPONSE,
        ResponseMsgId(res),
        strError,
        strResult
    );

    free(strError);
    free(strResult);

    return responseStr;
}

Response *ResponseCreateOk(uint32_t msgid) {

  Result res = {.msg = "", .success = true};
  return ResponseCreateResult(msgid, res);
}

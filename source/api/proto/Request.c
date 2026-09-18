#include "Request.h"
#include "logc/log.h"
#include "mpack/mpack-common.h"
#include "mpack/mpack-platform.h"
#include "mpack/mpack-reader.h"
#include "mpack/mpack-writer.h"
#include "RawBuffer.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Helper estático para serializar recursivamente qualquer Object no MPack Writer */
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

Request *RequestNew(void) {
  Request *request = calloc(1, sizeof(Request));
  if (!request) return NULL;

  request->type = HAZE_RPC_REQUEST;

  request->method = calloc(1, sizeof(char));
  if (!request->method) {
    free(request);
    return NULL;
  }

  request->parameters = ObjectArrayCreate();
  if (!request->parameters) {
    free(request->method);
    free(request);
    return NULL;
  }

  return request;
}

Object *RequestParamNew(void) {
  return ObjectNew();
}

void RequestParamFree(Object **r) {
  ObjectFree(r);
}

Object *RequestParamGet(const Request *r, uint32_t index) {
  if (!r || !r->parameters)
    return NULL;

  /* Retorna o ponteiro para a posição no ObjectArray */
  if (index < ObjectArrayLen(r->parameters)) {
    return r->parameters->objects[index];
  }

  return NULL;
}

bool RequestParamAppend(Request *rq, Object *param, uint32_t ind) {
  if (!rq || !rq->parameters || !param) {
    return false;
  }

  /* Se o índice for o próximo elemento da sequência, usamos o Append padrão */
  if (ind == ObjectArrayLen(rq->parameters)) {
    return ObjectArrayAppend(rq->parameters, param);
  }

  /* Se o índice for menor que o tamanho atual, substitui */
  if (ind < ObjectArrayLen(rq->parameters)) {
    if (rq->parameters->objects[ind] != NULL) {
      return false; // Evita sobrescrever parâmetros existentes sem liberar
    }
    rq->parameters->objects[ind] = param;
    return true;
  }

  /* Preenche lacunas com OBJ_NIL caso o índice informado seja maior que a len */
  while (ObjectArrayLen(rq->parameters) < ind) {
    ObjectArrayAppend(rq->parameters, ObjectCreateNil());
  }

  return ObjectArrayAppend(rq->parameters, param);
}

RawBuffer *RequestMarshal(Request *request) {
  if (!request) {
    return NULL;
  }

  char *data = NULL;
  size_t size = 0;
  mpack_writer_t writer;

  mpack_writer_init_growable(&writer, &data, &size);
  if (mpack_writer_error(&writer) != mpack_ok) {
    log_error("RequestMarshal init failed: %s",
                 mpack_error_to_string(mpack_writer_error(&writer)));
    return NULL;
  }

  /* Array principal do MessagePack-RPC: [type, msgid, method, params] */
  mpack_start_array(&writer, 4);

  mpack_write_uint(&writer, (uint64_t)request->type);
  mpack_write_uint(&writer, (uint64_t)request->msgid);

  if (request->method) {
    mpack_write_str(&writer, request->method, (uint32_t)strlen(request->method));
  } else {
    mpack_write_nil(&writer);
  }

  /* Sub-array de parâmetros */
  uint32_t param_count = RequestParamCount(request);
  mpack_start_array(&writer, param_count);

  for (uint32_t i = 0; i < param_count; i++) {
    Object *param = ObjectArrayGet(request->parameters, i);
    ObjectMarshalMPack(&writer, param);
  }

  mpack_finish_array(&writer); /* params */
  mpack_finish_array(&writer); /* request */

  mpack_error_t error = mpack_writer_destroy(&writer);
  if (error != mpack_ok) {
    log_error("RequestMarshal failed: %s", mpack_error_to_string(error));
    if (data) {
      MPACK_FREE(data);
    }
    return NULL;
  }

  RawBuffer *buffer = RawBufferNew(data, size);
  if (!buffer) {
    MPACK_FREE(data);
    return NULL;
  }

  return buffer;
}

Request *RequestUnmarshal(RawBuffer *b) {
  if (!b || !RawBufferLen(b))
    return NULL;

  mpack_reader_t reader;
  mpack_reader_init_data(&reader, RawBufferData(b), RawBufferLen(b));

  Request *request = NULL;
  mpack_tag_t tag;

  /* Array de envelope */
  tag = mpack_read_tag(&reader);
  if (mpack_tag_type(&tag) != mpack_type_array || mpack_tag_array_count(&tag) != 4) {
    goto fail;
  }

  request = RequestNew();

  /* Type */
  tag = mpack_read_tag(&reader);
  if (mpack_tag_type(&tag) == mpack_type_uint)
    request->type = (HazeServerRPCType)mpack_tag_uint_value(&tag);
  else if (mpack_tag_type(&tag) == mpack_type_int)
    request->type = (HazeServerRPCType)mpack_tag_int_value(&tag);
  else
    goto fail;

  if (request->type != HAZE_RPC_REQUEST) {
    goto fail;
  }

  /* MsgID */
  tag = mpack_read_tag(&reader);
  if (mpack_tag_type(&tag) == mpack_type_uint)
    request->msgid = (uint32_t)mpack_tag_uint_value(&tag);
  else if (mpack_tag_type(&tag) == mpack_type_int)
    request->msgid = (uint32_t)mpack_tag_int_value(&tag);
  else
    goto fail;

  /* Method */
  tag = mpack_read_tag(&reader);
  if (mpack_tag_type(&tag) != mpack_type_str) {
    goto fail;
  }

  uint32_t method_len = mpack_tag_str_length(&tag);
  const char *method_bytes = mpack_read_bytes_inplace(&reader, method_len);
  if (!method_bytes) {
    goto fail;
  }

  char *method_copy = malloc((size_t)method_len + 1);
  if (!method_copy) {
    goto fail;
  }
  memcpy(method_copy, method_bytes, method_len);
  method_copy[method_len] = '\0';

  RequestSetMethod(request, method_copy);
  free(method_copy);

  mpack_done_str(&reader);

  /* Params */
  mpack_tag_t param_arr = mpack_read_tag(&reader);
  if (mpack_tag_type(&param_arr) != mpack_type_array) {
    goto fail;
  }

  uint32_t count = mpack_tag_array_count(&param_arr);
  for (uint32_t i = 0; i < count; i++) {
    mpack_tag_t type_element = mpack_read_tag(&reader);

    switch (mpack_tag_type(&type_element)) {

    case mpack_type_nil:
      RequestParamAppend(request, ObjectCreateNil(), i);
      break;

    case mpack_type_bool: {
      bool v = mpack_tag_bool_value(&type_element);
      RequestParamAppend(request, ObjectCreateBool(v), i);
      break;
    }

    case mpack_type_int: {
      int64_t v = mpack_tag_int_value(&type_element);
      RequestParamAppend(request, ObjectCreateInt(v), i);
      break;
    }

    case mpack_type_uint: {
      uint64_t v = mpack_tag_uint_value(&type_element);
      RequestParamAppend(request, ObjectCreateUInt(v), i);
      break;
    }

    case mpack_type_float: {
      float v = mpack_tag_float_value(&type_element);
      RequestParamAppend(request, ObjectCreateFloat(v), i);
      break;
    }

    case mpack_type_double: {
      double v = mpack_tag_double_value(&type_element);
      RequestParamAppend(request, ObjectCreateDouble(v), i);
      break;
    }

    case mpack_type_str: {
      size_t len = mpack_tag_str_length(&type_element);
      char *buff = malloc(len + 1);
      mpack_read_bytes(&reader, buff, len);
      buff[len] = '\0';
      RequestParamAppend(request, ObjectCreateStr(buff), i);
      free(buff);
      break;
    }

    case mpack_type_bin: {
      size_t len = mpack_tag_bin_length(&type_element);
      void *buff = malloc(len);
      mpack_read_bytes(&reader, buff, len);

      Object *bin_obj = ObjectNew();
      bin_obj->type = OBJ_BIN;
      bin_obj->value.bin_value = RawBufferNew(buff, len);
      bin_obj->size = len;

      RequestParamAppend(request, bin_obj, i);
      free(buff);
      break;
    }

    default:
      break;
    }
  }

  mpack_done_array(&reader);

  /* Cálculo exato de bytes consumidos para avanço no buffer de streaming */
  const char *remaining_data = NULL;
  size_t remaining = mpack_reader_remaining(&reader, &remaining_data);
  b->len = RawBufferLen(b) - remaining;

  mpack_reader_destroy(&reader);
  return request;

fail:
  if (mpack_reader_error(&reader) != mpack_ok) {
    log_error("mpack_reader Error: %s",
                 mpack_error_to_string(mpack_reader_error(&reader)));
  }

  mpack_reader_destroy(&reader);

  if (request)
    RequestFree(&request);

  return NULL;
}

void RequestSetMethod(Request *request, const char *method) {
  if (!request)
    return;

  free(request->method);
  request->method = NULL;

  if (!method)
    return;

  size_t meth_len = strlen(method);
  char *temp = malloc(meth_len + 1);
  if (!temp)
    return;

  strcpy(temp, method);
  request->method = temp;
}

void RequestFree(Request **request) {
  if (!request || !*request)
    return;

  if ((*request)->method) {
    free((*request)->method);
    (*request)->method = NULL;
  }

  if ((*request)->parameters) {
    ObjectArrayFree(&((*request)->parameters));
  }

  free(*request);
  *request = NULL;
}

uint32_t RequestParamCount(const Request *r) {
  if (!r || !r->parameters)
    return 0;

  return (uint32_t)ObjectArrayLen(r->parameters);
}

bool RequestParamIsType(const Request *r, ObjectType type, uint32_t index) {
  if (!r || !r->parameters)
    return false;

  Object *param = ObjectArrayGet(r->parameters, index);
  if (!param)
    return false;

  return RequestParamTypeGet(param) == type;
}

const char *RequestPrint(const Request *r) {
  if (!r) return "";

  printf("[%d, %u, \"%s\", [", r->type, r->msgid, r->method ? r->method : "");

  uint32_t count = RequestParamCount(r);
  for (uint32_t i = 0; i < count; i++) {
    Object *param = ObjectArrayGet(r->parameters, i);
    if (!param) {
      printf("null");
    } else {
      switch (RequestParamTypeGet(param)) {
      case OBJ_INT:
        printf("%ld", (long)param->value.int_value);
        break;
      case OBJ_UINT:
        printf("%lu", (unsigned long)param->value.uint_value);
        break;
      case OBJ_STR:
        printf("\"%s\"", param->value.str_value ? param->value.str_value : "");
        break;
      case OBJ_BOOL:
        printf("%s", param->value.bool_value ? "true" : "false");
        break;
      case OBJ_FLOAT:
        printf("%f", param->value.float_value);
        break;
      case OBJ_DOUBLE:
        printf("%lf", param->value.double_value);
        break;
      default:
        printf("<object type %d>", param->type);
        break;
      }
    }

    if (i + 1 < count)
      printf(", ");
  }

  printf("]]\n");
  return "";
}

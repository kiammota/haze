#include "api/proto/Notification.h"
#include "msgpack/Object.h"
#include "RawBuffer.h"
#include "macros.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int TestNotificationNew(void) {
  TEST_LOG("NotificationNew");

  Notification *n = NotificationNew();

  TEST_ASSERT(n != NULL);
  TEST_ASSERT(n->type == HAZE_RPC_NOTIFICATION);
  TEST_ASSERT(n->method == NULL);
  TEST_ASSERT(n->params != NULL);
  TEST_ASSERT(ObjectArrayLen(n->params) == 0);

  NotificationFree(&n);

  TEST_ASSERT(n == NULL);

  TEST_PASS();
}

static int TestNotificationSetMethod(void) {
  TEST_LOG("NotificationSetMethod");

  Notification *n = NotificationNew();

  TEST_ASSERT(n != NULL);

  NotificationSetMethod(n, "session/changed");
  TEST_ASSERT(strcmp(NotificationMethod(n), "session/changed") == 0);

  NotificationSetMethod(n, "sample/loaded");
  TEST_ASSERT(strcmp(NotificationMethod(n), "sample/loaded") == 0);

  NotificationSetMethod(n, "");
  TEST_ASSERT(strcmp(NotificationMethod(n), "") == 0);

  NotificationFree(&n);

  TEST_PASS();
}

static int TestNotificationCreateEmpty(void) {
  TEST_LOG("NotificationCreateEmpty");

  Notification *n =
      NotificationCreate("session/ready", NULL, 0);

  TEST_ASSERT(n != NULL);
  TEST_ASSERT(strcmp(NotificationMethod(n), "session/ready") == 0);
  TEST_ASSERT(NotificationParams(n) != NULL);
  TEST_ASSERT(ObjectArrayLen(n->params) == 0);

  NotificationFree(&n);

  TEST_PASS();
}

static int TestNotificationCreateParams(void) {
  TEST_LOG("NotificationCreateParams");

  Object *params[4];

  params[0] = ObjectCreateStr("hello");
  params[1] = ObjectCreateInt(-123);
  params[2] = ObjectCreateUInt(456);
  params[3] = ObjectCreateBool(true);

  for (size_t i = 0; i < 4; i++)
    TEST_ASSERT(params[i] != NULL);

  Notification *n =
      NotificationCreate("test/method", params, 4);

  TEST_ASSERT(n != NULL);
  TEST_ASSERT(strcmp(NotificationMethod(n), "test/method") == 0);
  TEST_ASSERT(ObjectArrayLen(n->params) == 4);

  TEST_ASSERT(
      strcmp(
          ObjectGetStr(ObjectArrayGet(n->params, 0)),
          "hello") == 0);

  TEST_ASSERT(
      ObjectGetInt(ObjectArrayGet(n->params, 1)) == -123);

  TEST_ASSERT(
      ObjectGetUInt(ObjectArrayGet(n->params, 2)) == 456);

  TEST_ASSERT(
      ObjectGetBool(ObjectArrayGet(n->params, 3)) == true);

  for (size_t i = 0; i < 4; i++)
    ObjectFree(&params[i]);

  NotificationFree(&n);

  TEST_PASS();
}

static int TestNotificationMarshalEmpty(void) {
  TEST_LOG("NotificationMarshalEmpty");

  Notification *n =
      NotificationCreate("session/ready", NULL, 0);

  TEST_ASSERT(n != NULL);

  RawBuffer *buffer =
      NotificationMarshal(n);

  TEST_ASSERT(buffer != NULL);
  TEST_ASSERT(buffer->data != NULL);
  TEST_ASSERT(buffer->len > 0);

  Notification *decoded =
      NotificationUnmarshal(buffer);

  TEST_ASSERT(decoded != NULL);
  TEST_ASSERT(
      strcmp(
          NotificationMethod(decoded),
          "session/ready") == 0);

  TEST_ASSERT(
      ObjectArrayLen(decoded->params) == 0);

  NotificationFree(&decoded);
  RawBufferFree(&buffer);
  NotificationFree(&n);

  TEST_PASS();
}

static int TestNotificationRoundTripPrimitive(void) {
  TEST_LOG("NotificationRoundTripPrimitive");

  Object *params[8];

  params[0] = ObjectCreateNil();
  params[1] = ObjectCreateBool(true);
  params[2] = ObjectCreateBool(false);
  params[3] = ObjectCreateInt(-987654321);
  params[4] = ObjectCreateUInt(987654321);
  params[5] = ObjectCreateFloat(3.1415926f);
  params[6] = ObjectCreateDouble(6.283185307179586);
  params[7] = ObjectCreateStr("Hello Haze");

  for (size_t i = 0; i < 8; i++)
    TEST_ASSERT(params[i] != NULL);

  Notification *original =
      NotificationCreate(
          "test/all-types",
          params,
          8);

  TEST_ASSERT(original != NULL);

  RawBuffer *buffer =
      NotificationMarshal(original);

  TEST_ASSERT(buffer != NULL);

  Notification *decoded =
      NotificationUnmarshal(buffer);

  TEST_ASSERT(decoded != NULL);
  TEST_ASSERT(
      strcmp(
          NotificationMethod(decoded),
          "test/all-types") == 0);

  TEST_ASSERT(
      ObjectArrayLen(decoded->params) == 8);

  Object *obj;

  obj = ObjectArrayGet(decoded->params, 0);
  TEST_ASSERT(ObjectExpect(obj, OBJ_NIL));

  obj = ObjectArrayGet(decoded->params, 1);
  TEST_ASSERT(ObjectExpect(obj, OBJ_BOOL));
  TEST_ASSERT(ObjectGetBool(obj));

  obj = ObjectArrayGet(decoded->params, 2);
  TEST_ASSERT(ObjectExpect(obj, OBJ_BOOL));
  TEST_ASSERT(!ObjectGetBool(obj));

  obj = ObjectArrayGet(decoded->params, 3);
  TEST_ASSERT(ObjectExpect(obj, OBJ_INT));
  TEST_ASSERT(ObjectGetInt(obj) == -987654321);

  obj = ObjectArrayGet(decoded->params, 4);

  TEST_ASSERT(
      ObjectExpect(obj, OBJ_INT) ||
      ObjectExpect(obj, OBJ_UINT));

  if (ObjectExpect(obj, OBJ_INT))
    TEST_ASSERT(ObjectGetInt(obj) == 987654321);
  else
    TEST_ASSERT(ObjectGetUInt(obj) == 987654321);

  obj = ObjectArrayGet(decoded->params, 5);
  TEST_ASSERT(ObjectExpect(obj, OBJ_FLOAT));
  TEST_ASSERT(
      fabsf(
          ObjectGetFloat(obj) -
          3.1415926f) < 0.00001f);

  obj = ObjectArrayGet(decoded->params, 6);
  TEST_ASSERT(ObjectExpect(obj, OBJ_DOUBLE));
  TEST_ASSERT(
      fabs(
          ObjectGetDouble(obj) -
          6.283185307179586) < 0.000000001);

  obj = ObjectArrayGet(decoded->params, 7);
  TEST_ASSERT(ObjectExpect(obj, OBJ_STR));
  TEST_ASSERT(
      strcmp(
          ObjectGetStr(obj),
          "Hello Haze") == 0);

  NotificationFree(&decoded);
  RawBufferFree(&buffer);
  NotificationFree(&original);

  for (size_t i = 0; i < 8; i++)
    ObjectFree(&params[i]);

  TEST_PASS();
}

static int TestNotificationBinary(void) {
  TEST_LOG("NotificationBinary");

  static const uint8_t data[] = {
      0x00,
      0x01,
      0x02,
      0x7f,
      0x80,
      0xfe,
      0xff
  };

  /*
   * RawBuffer fica em uma única alocação.
   * Isso é compatível com o ObjectFree() atual,
   * que faz free() diretamente em bin_value.
   */
  RawBuffer *raw =
      malloc(sizeof(RawBuffer) + sizeof(data));

  TEST_ASSERT(raw != NULL);

  raw->data = raw + 1;
  raw->len = sizeof(data);

  memcpy(
      raw->data,
      data,
      sizeof(data));

  Object *obj = ObjectNew();

  TEST_ASSERT(obj != NULL);

  obj->type = OBJ_BIN;
  obj->value.bin_value = raw;
  obj->size = raw->len;

  Object *params[] = {
      obj
  };

  Notification *n =
      NotificationCreate(
          "test/bin",
          params,
          1);

  TEST_ASSERT(n != NULL);

  RawBuffer *buffer =
      NotificationMarshal(n);

  TEST_ASSERT(buffer != NULL);
  TEST_ASSERT(buffer->len > 0);

  Notification *decoded =
      NotificationUnmarshal(buffer);

  TEST_ASSERT(decoded != NULL);
  TEST_ASSERT(
      ObjectArrayLen(decoded->params) == 1);

  Object *decodedObj =
      ObjectArrayGet(decoded->params, 0);

  TEST_ASSERT(decodedObj != NULL);
  TEST_ASSERT(ObjectExpect(decodedObj, OBJ_BIN));

  RawBuffer *decodedRaw =
      ObjectGetBin(decodedObj);

  TEST_ASSERT(decodedRaw != NULL);
  TEST_ASSERT(decodedRaw->data != NULL);
  TEST_ASSERT(decodedRaw->len == sizeof(data));

  TEST_ASSERT(
      memcmp(
          decodedRaw->data,
          data,
          sizeof(data)) == 0);

  NotificationFree(&decoded);
  RawBufferFree(&buffer);
  NotificationFree(&n);

  /*
   * NotificationCreate copiou o Object.
   * O Object original ainda possui ownership de raw.
   */
  ObjectFree(&obj);

  TEST_PASS();
}

static int TestNotificationNestedArray(void) {
  TEST_LOG("NotificationNestedArray");

  Object *innerA =
      ObjectCreateInt(-100);

  Object *innerB =
      ObjectCreateUInt(100);

  Object *innerC =
      ObjectCreateStr("nested");

  TEST_ASSERT(innerA != NULL);
  TEST_ASSERT(innerB != NULL);
  TEST_ASSERT(innerC != NULL);

  ObjectArray *nestedArray =
      ObjectArrayCreate();

  TEST_ASSERT(nestedArray != NULL);

  TEST_ASSERT(
      ObjectArrayAppend(
          nestedArray,
          innerA));

  TEST_ASSERT(
      ObjectArrayAppend(
          nestedArray,
          innerB));

  TEST_ASSERT(
      ObjectArrayAppend(
          nestedArray,
          innerC));

  Object *nestedObject =
      ObjectNew();

  TEST_ASSERT(nestedObject != NULL);

  nestedObject->type = OBJ_ARRAY;
  nestedObject->value.array_value = nestedArray;
  nestedObject->size = ObjectArrayLen(nestedArray);

  Object *params[2];

  params[0] = nestedObject;
  params[1] = ObjectCreateBool(true);

  TEST_ASSERT(params[1] != NULL);

  Notification *original =
      NotificationCreate(
          "array/nested",
          params,
          2);

  TEST_ASSERT(original != NULL);

  RawBuffer *buffer =
      NotificationMarshal(original);

  TEST_ASSERT(buffer != NULL);

  Notification *decoded =
      NotificationUnmarshal(buffer);

  TEST_ASSERT(decoded != NULL);
  TEST_ASSERT(
      ObjectArrayLen(decoded->params) == 2);

  Object *decodedArray =
      ObjectArrayGet(
          decoded->params,
          0);

  TEST_ASSERT(decodedArray != NULL);
  TEST_ASSERT(
      ObjectExpect(
          decodedArray,
          OBJ_ARRAY));

  ObjectArray *decodedValues =
      ObjectGetArray(decodedArray);

  TEST_ASSERT(decodedValues != NULL);
  TEST_ASSERT(
      ObjectArrayLen(decodedValues) == 3);

  Object *a =
      ObjectArrayGet(decodedValues, 0);

  Object *b =
      ObjectArrayGet(decodedValues, 1);

  Object *c =
      ObjectArrayGet(decodedValues, 2);

  TEST_ASSERT(ObjectExpect(a, OBJ_INT));
  TEST_ASSERT(ObjectGetInt(a) == -100);

  TEST_ASSERT(
      ObjectExpect(b, OBJ_INT) ||
      ObjectExpect(b, OBJ_UINT));

  if (ObjectExpect(b, OBJ_INT))
    TEST_ASSERT(ObjectGetInt(b) == 100);
  else
    TEST_ASSERT(ObjectGetUInt(b) == 100);

  TEST_ASSERT(ObjectExpect(c, OBJ_STR));
  TEST_ASSERT(
      strcmp(
          ObjectGetStr(c),
          "nested") == 0);

  Object *flag =
      ObjectArrayGet(
          decoded->params,
          1);

  TEST_ASSERT(ObjectExpect(flag, OBJ_BOOL));
  TEST_ASSERT(ObjectGetBool(flag));

  ObjectFree(&innerA);
  ObjectFree(&innerB);
  ObjectFree(&innerC);
  ObjectFree(&params[1]);

  /*
   * nestedObject possui nestedArray.
   * Portanto ele libera tudo internamente.
   */
  ObjectFree(&nestedObject);

  NotificationFree(&decoded);
  RawBufferFree(&buffer);
  NotificationFree(&original);

  TEST_PASS();
}

static int TestNotificationIntegerBoundaries(void) {
  TEST_LOG("NotificationIntegerBoundaries");

  Object *params[8];

  params[0] = ObjectCreateInt(INT64_MIN);
  params[1] = ObjectCreateInt(-1);
  params[2] = ObjectCreateInt(0);
  params[3] = ObjectCreateInt(INT64_MAX);

  params[4] = ObjectCreateUInt(0);
  params[5] = ObjectCreateUInt(1);
  params[6] = ObjectCreateUInt(UINT64_MAX - 1);
  params[7] = ObjectCreateUInt(UINT64_MAX);

  for (size_t i = 0; i < 8; i++)
    TEST_ASSERT(params[i] != NULL);

  Notification *n =
      NotificationCreate(
          "test/boundaries",
          params,
          8);

  TEST_ASSERT(n != NULL);

  RawBuffer *buffer =
      NotificationMarshal(n);

  TEST_ASSERT(buffer != NULL);

  Notification *decoded =
      NotificationUnmarshal(buffer);

  TEST_ASSERT(decoded != NULL);
  TEST_ASSERT(
      ObjectArrayLen(decoded->params) == 8);

  Object *obj;

  obj = ObjectArrayGet(decoded->params, 0);
  TEST_ASSERT(ObjectExpect(obj, OBJ_INT));
  TEST_ASSERT(
      ObjectGetInt(obj) == INT64_MIN);

  obj = ObjectArrayGet(decoded->params, 1);
  TEST_ASSERT(ObjectExpect(obj, OBJ_INT));
  TEST_ASSERT(ObjectGetInt(obj) == -1);

  obj = ObjectArrayGet(decoded->params, 2);

  TEST_ASSERT(
      ObjectExpect(obj, OBJ_INT) ||
      ObjectExpect(obj, OBJ_UINT));

  if (ObjectExpect(obj, OBJ_INT))
    TEST_ASSERT(ObjectGetInt(obj) == 0);
  else
    TEST_ASSERT(ObjectGetUInt(obj) == 0);

  obj = ObjectArrayGet(decoded->params, 3);

  TEST_ASSERT(
      ObjectExpect(obj, OBJ_INT) ||
      ObjectExpect(obj, OBJ_UINT));

  if (ObjectExpect(obj, OBJ_INT))
    TEST_ASSERT(
        ObjectGetInt(obj) == INT64_MAX);
  else
    TEST_ASSERT(
        ObjectGetUInt(obj) == INT64_MAX);

  obj = ObjectArrayGet(decoded->params, 4);

  TEST_ASSERT(
      ObjectExpect(obj, OBJ_INT) ||
      ObjectExpect(obj, OBJ_UINT));

  if (ObjectExpect(obj, OBJ_INT))
    TEST_ASSERT(ObjectGetInt(obj) == 0);
  else
    TEST_ASSERT(ObjectGetUInt(obj) == 0);

  obj = ObjectArrayGet(decoded->params, 5);

  TEST_ASSERT(
      ObjectExpect(obj, OBJ_INT) ||
      ObjectExpect(obj, OBJ_UINT));

  if (ObjectExpect(obj, OBJ_INT))
    TEST_ASSERT(ObjectGetInt(obj) == 1);
  else
    TEST_ASSERT(ObjectGetUInt(obj) == 1);

  obj = ObjectArrayGet(decoded->params, 6);

  TEST_ASSERT(ObjectExpect(obj, OBJ_UINT));
  TEST_ASSERT(
      ObjectGetUInt(obj) == UINT64_MAX - 1);

  obj = ObjectArrayGet(decoded->params, 7);

  TEST_ASSERT(ObjectExpect(obj, OBJ_UINT));
  TEST_ASSERT(
      ObjectGetUInt(obj) == UINT64_MAX);

  NotificationFree(&decoded);
  RawBufferFree(&buffer);
  NotificationFree(&n);

  for (size_t i = 0; i < 8; i++)
    ObjectFree(&params[i]);

  TEST_PASS();
}

static int TestNotificationManyParams(void) {
  TEST_LOG("NotificationManyParams");

  enum {
    COUNT = 1024
  };

  Object *params[COUNT];

  for (size_t i = 0; i < COUNT; i++) {
    switch (i % 6) {
    case 0:
      params[i] =
          ObjectCreateInt(-(int64_t)i);
      break;

    case 1:
      params[i] =
          ObjectCreateUInt((uint64_t)i * 100);
      break;

    case 2:
      params[i] =
          ObjectCreateBool((i % 2) == 0);
      break;

    case 3:
      params[i] =
          ObjectCreateStr("massive-test");
      break;

    case 4:
      params[i] =
          ObjectCreateFloat(
              (float)i / 10.0f);
      break;

    case 5:
      params[i] =
          ObjectCreateNil();
      break;
    }

    TEST_ASSERT(params[i] != NULL);
  }

  Notification *original =
      NotificationCreate(
          "test/massive",
          params,
          COUNT);

  TEST_ASSERT(original != NULL);

  RawBuffer *buffer =
      NotificationMarshal(original);

  TEST_ASSERT(buffer != NULL);
  TEST_ASSERT(buffer->len > 0);

  Notification *decoded =
      NotificationUnmarshal(buffer);

  TEST_ASSERT(decoded != NULL);

  TEST_ASSERT(
      ObjectArrayLen(decoded->params) == COUNT);

  for (size_t i = 0; i < COUNT; i++) {
    Object *obj =
        ObjectArrayGet(
            decoded->params,
            i);

    TEST_ASSERT(obj != NULL);

    switch (i % 6) {
    case 0:
      TEST_ASSERT(
          ObjectExpect(obj, OBJ_INT) ||
          ObjectExpect(obj, OBJ_UINT));

      if (i == 0) {
        if (ObjectExpect(obj, OBJ_INT))
          TEST_ASSERT(
              ObjectGetInt(obj) == 0);
        else
          TEST_ASSERT(
              ObjectGetUInt(obj) == 0);
      } else {
        TEST_ASSERT(
            ObjectExpect(obj, OBJ_INT));

        TEST_ASSERT(
            ObjectGetInt(obj) == -(int64_t)i);
      }
      break;

    case 1:
      TEST_ASSERT(
          ObjectExpect(obj, OBJ_INT) ||
          ObjectExpect(obj, OBJ_UINT));

      if (ObjectExpect(obj, OBJ_INT))
        TEST_ASSERT(
            ObjectGetInt(obj) ==
            (int64_t)i * 100);
      else
        TEST_ASSERT(
            ObjectGetUInt(obj) ==
            (uint64_t)i * 100);
      break;

    case 2:
      TEST_ASSERT(
          ObjectExpect(obj, OBJ_BOOL));

      TEST_ASSERT(
          ObjectGetBool(obj) ==
          ((i % 2) == 0));
      break;

    case 3:
      TEST_ASSERT(
          ObjectExpect(obj, OBJ_STR));

      TEST_ASSERT(
          strcmp(
              ObjectGetStr(obj),
              "massive-test") == 0);
      break;

    case 4:
      TEST_ASSERT(
          ObjectExpect(obj, OBJ_FLOAT));

      TEST_ASSERT(
          fabsf(
              ObjectGetFloat(obj) -
              ((float)i / 10.0f)
          ) < 0.0001f);
      break;

    case 5:
      TEST_ASSERT(
          ObjectExpect(obj, OBJ_NIL));
      break;
    }
  }

  NotificationFree(&decoded);
  RawBufferFree(&buffer);
  NotificationFree(&original);

  for (size_t i = 0; i < COUNT; i++)
    ObjectFree(&params[i]);

  TEST_PASS();
}

static int TestNotificationOwnership(void) {
  TEST_LOG("NotificationOwnership");

  Object *param =
      ObjectCreateStr("original");

  TEST_ASSERT(param != NULL);

  Object *params[] = {
      param
  };

  Notification *n =
      NotificationCreate(
          "ownership/test",
          params,
          1);

  TEST_ASSERT(n != NULL);

  ObjectFree(&param);

  Object *stored =
      ObjectArrayGet(
          n->params,
          0);

  TEST_ASSERT(stored != NULL);
  TEST_ASSERT(
      ObjectExpect(stored, OBJ_STR));

  TEST_ASSERT(
      strcmp(
          ObjectGetStr(stored),
          "original") == 0);

  NotificationFree(&n);

  TEST_PASS();
}

static int TestNotificationInvalidInput(void) {
  TEST_LOG("NotificationInvalidInput");

  Notification *n =
      NotificationNew();

  TEST_ASSERT(n != NULL);

  TEST_ASSERT(
      NotificationMarshal(NULL) == NULL);

  TEST_ASSERT(
      NotificationUnmarshal(NULL) == NULL);

  NotificationSetMethod(NULL, "hello");

  RawBuffer empty =
      RawBufferInit(NULL, 0);

  TEST_ASSERT(
      NotificationUnmarshal(&empty) == NULL);

  NotificationSetMethod(n, NULL);

  TEST_ASSERT(
      NotificationMethod(n) == NULL);

  NotificationFree(&n);

  TEST_PASS();
}

static int TestNotificationCreateInvalidParams(void) {
  TEST_LOG("NotificationCreateInvalidParams");

  Notification *empty =
      NotificationCreate(
          NULL,
          NULL,
          0);

  TEST_ASSERT(empty != NULL);

  NotificationFree(&empty);

  Object *nil =
      ObjectCreateNil();

  TEST_ASSERT(nil != NULL);

  Object *params[] = {
      nil
  };

  TEST_ASSERT(
      NotificationCreate(
          NULL,
          params,
          1) == NULL);

  ObjectFree(&nil);

  Object *invalid[] = {
      NULL
  };

  TEST_ASSERT(
      NotificationCreate(
          "test/null",
          invalid,
          1) == NULL);

  TEST_PASS();
}

static int TestNotificationMalformedData(void) {
  TEST_LOG("NotificationMalformedData");

  uint8_t invalid1[] = {
      0x00
  };

  uint8_t invalid2[] = {
      0x93,
      0x02
  };

  uint8_t invalid3[] = {
      0x93,
      0x02,
      0xa4,
      't',
      'e',
      's',
      't'
  };

  uint8_t invalid4[] = {
      0x93,
      0x01,
      0xa4,
      't',
      'e',
      's',
      't',
      0x90
  };

  uint8_t invalid5[] = {
      0x92,
      0x02,
      0xa4,
      't',
      'e',
      's',
      't'
  };

  RawBuffer b1 =
      RawBufferInit(
          invalid1,
          sizeof(invalid1));

  RawBuffer b2 =
      RawBufferInit(
          invalid2,
          sizeof(invalid2));

  RawBuffer b3 =
      RawBufferInit(
          invalid3,
          sizeof(invalid3));

  RawBuffer b4 =
      RawBufferInit(
          invalid4,
          sizeof(invalid4));

  RawBuffer b5 =
      RawBufferInit(
          invalid5,
          sizeof(invalid5));

  TEST_ASSERT(
      NotificationUnmarshal(&b1) == NULL);

  TEST_ASSERT(
      NotificationUnmarshal(&b2) == NULL);

  TEST_ASSERT(
      NotificationUnmarshal(&b3) == NULL);

  TEST_ASSERT(
      NotificationUnmarshal(&b4) == NULL);

  TEST_ASSERT(
      NotificationUnmarshal(&b5) == NULL);

  TEST_PASS();
}

static int TestNotificationLongMethod(void) {
  TEST_LOG("NotificationLongMethod");

  char method[1024];

  for (size_t i = 0; i < sizeof(method) - 1; i++)
    method[i] = 'a';

  method[sizeof(method) - 1] = '\0';

  Notification *n =
      NotificationCreate(
          method,
          NULL,
          0);

  TEST_ASSERT(n != NULL);

  RawBuffer *buffer =
      NotificationMarshal(n);

  TEST_ASSERT(buffer != NULL);

  Notification *decoded =
      NotificationUnmarshal(buffer);

  TEST_ASSERT(decoded != NULL);

  TEST_ASSERT(
      strcmp(
          NotificationMethod(decoded),
          method) == 0);

  NotificationFree(&decoded);
  RawBufferFree(&buffer);
  NotificationFree(&n);

  TEST_PASS();
}

static int TestNotificationRepeatedRoundTrip(void) {
  TEST_LOG("NotificationRepeatedRoundTrip");

  for (size_t i = 0; i < 100; i++) {
    Object *params[3];

    params[0] =
        ObjectCreateInt(-(int64_t)i);

    params[1] =
        ObjectCreateUInt((uint64_t)i);

    params[2] =
        ObjectCreateStr("iteration");

    TEST_ASSERT(params[0] != NULL);
    TEST_ASSERT(params[1] != NULL);
    TEST_ASSERT(params[2] != NULL);

    Notification *n =
        NotificationCreate(
            "test/repeated",
            params,
            3);

    TEST_ASSERT(n != NULL);

    RawBuffer *buffer =
        NotificationMarshal(n);

    TEST_ASSERT(buffer != NULL);

    Notification *decoded =
        NotificationUnmarshal(buffer);

    TEST_ASSERT(decoded != NULL);
    TEST_ASSERT(
        ObjectArrayLen(decoded->params) == 3);

    /*
     * i == 0 pode voltar como UINT.
     * Valores negativos continuam INT.
     */
    Object *negative =
        ObjectArrayGet(
            decoded->params,
            0);

    TEST_ASSERT(
        ObjectExpect(negative, OBJ_INT) ||
        ObjectExpect(negative, OBJ_UINT));

    if (i == 0) {
      if (ObjectExpect(negative, OBJ_INT))
        TEST_ASSERT(
            ObjectGetInt(negative) == 0);
      else
        TEST_ASSERT(
            ObjectGetUInt(negative) == 0);
    } else {
      TEST_ASSERT(
          ObjectExpect(negative, OBJ_INT));

      TEST_ASSERT(
          ObjectGetInt(negative) ==
          -(int64_t)i);
    }

    Object *positive =
        ObjectArrayGet(
            decoded->params,
            1);

    TEST_ASSERT(
        ObjectExpect(positive, OBJ_INT) ||
        ObjectExpect(positive, OBJ_UINT));

    if (ObjectExpect(positive, OBJ_INT))
      TEST_ASSERT(
          ObjectGetInt(positive) ==
          (int64_t)i);
    else
      TEST_ASSERT(
          ObjectGetUInt(positive) ==
          (uint64_t)i);

    Object *text =
        ObjectArrayGet(
            decoded->params,
            2);

    TEST_ASSERT(
        ObjectExpect(text, OBJ_STR));

    TEST_ASSERT(
        strcmp(
            ObjectGetStr(text),
            "iteration") == 0);

    NotificationFree(&decoded);
    RawBufferFree(&buffer);
    NotificationFree(&n);

    for (size_t j = 0; j < 3; j++)
      ObjectFree(&params[j]);
  }

  TEST_PASS();
}

int main(void) {
  TEST_LOG("Running Notification tests");

  int failed = 0;

  failed += TestNotificationNew() != 0;
  failed += TestNotificationSetMethod() != 0;
  failed += TestNotificationCreateEmpty() != 0;
  failed += TestNotificationCreateParams() != 0;
  failed += TestNotificationMarshalEmpty() != 0;
  failed += TestNotificationRoundTripPrimitive() != 0;
  failed += TestNotificationBinary() != 0;
  failed += TestNotificationNestedArray() != 0;
  failed += TestNotificationIntegerBoundaries() != 0;
  failed += TestNotificationManyParams() != 0;
  failed += TestNotificationOwnership() != 0;
  failed += TestNotificationInvalidInput() != 0;
  failed += TestNotificationCreateInvalidParams() != 0;
  failed += TestNotificationMalformedData() != 0;
  failed += TestNotificationLongMethod() != 0;
  failed += TestNotificationRepeatedRoundTrip() != 0;

  if (failed == 0) {
    TEST_LOG("All Notification tests passed");
    return 0;
  }

  TEST_LOG("Some Notification tests failed");
  return 1;
}

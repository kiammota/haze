#include "api/proto/Response.h"
#include "msgpack/Object.h"
#include "macros.h"
#include <stdint.h>

/* ============================================================
 * TestResponseNewFree
 * ============================================================ */
static int TestResponseNewFree(void) {
  TEST_LOG("ResponseNewFree");

  Response *res = ResponseNew();
  TEST_ASSERT(res != NULL);

  TEST_ASSERT(ResponseResult(res) == NULL);
  TEST_ASSERT(ResponseError(res) == NULL);
  TEST_ASSERT(ResponseMsgId(res) == 0);

  bool ok = ResponseFree(&res);
  TEST_ASSERT(ok == true);
  TEST_ASSERT(res == NULL);

  /* Double free */
  ResponseFree(&res);
  TEST_ASSERT(res == NULL);

  /* Free de NULL */
  Response *null_res = NULL;
  ResponseFree(&null_res);
  TEST_ASSERT(null_res == NULL);

  TEST_PASS();
}

/* ============================================================
 * TestResponseMsgId
 * ============================================================ */
static int TestResponseMsgId(void) {
  TEST_LOG("ResponseMsgId");

  Response *res = ResponseNew();
  TEST_ASSERT(res != NULL);

  TEST_ASSERT(ResponseMsgId(res) == 0);

  TEST_ASSERT(ResponseSetMsgId(res, 42) == true);
  TEST_ASSERT(ResponseMsgId(res) == 42);

  TEST_ASSERT(ResponseSetMsgId(res, 0) == true);
  TEST_ASSERT(ResponseMsgId(res) == 0);

  TEST_ASSERT(ResponseSetMsgId(res, UINT32_MAX) == true);
  TEST_ASSERT(ResponseMsgId(res) == UINT32_MAX);

  /* NULL safety */
  TEST_ASSERT(ResponseSetMsgId(NULL, 123) == false);
  TEST_ASSERT(ResponseMsgId(NULL) == 0);

  ResponseFree(&res);
  TEST_PASS();
}

/* ============================================================
 * TestResponseSetResult / SetError (ownership)
 * ============================================================ */
static int TestResponseSetObjects(void) {
  TEST_LOG("ResponseSetObjects");

  Response *res = ResponseNew();
  TEST_ASSERT(res != NULL);

  /* ---- Result com ObjectCreateStr ---- */
  Object *result_obj = ObjectCreateStr("result_value");
  TEST_ASSERT(result_obj != NULL);
  TEST_ASSERT(ObjectGetType(result_obj) == OBJ_STR);

  bool ok = ResponseSetResultObject(res, result_obj);
  TEST_ASSERT(ok == true);
  TEST_ASSERT(ResponseResult(res) != NULL);

  /* Detecta ownership */
  bool took_ownership = (ResponseResult(res) == result_obj);

  if (!took_ownership) {
    ObjectFree(&result_obj);
  }

  /* Ainda deve estar vivo */
  TEST_ASSERT(ResponseResult(res) != NULL);
  TEST_ASSERT(ObjectGetType(ResponseResult(res)) == OBJ_STR);

  /* ---- Error ---- */
  Object *error_obj = ObjectCreateStr("error_value");
  TEST_ASSERT(error_obj != NULL);

  ok = ResponseSetErrorObject(res, error_obj);
  TEST_ASSERT(ok == true);
  TEST_ASSERT(ResponseError(res) != NULL);

  bool error_took = (ResponseError(res) == error_obj);
  if (!error_took) {
    ObjectFree(&error_obj);
  }

  TEST_ASSERT(ResponseError(res) != NULL);

  /* Limpar campos com NULL */
  TEST_ASSERT(ResponseSetResultObject(res, NULL) == true);
  TEST_ASSERT(ResponseResult(res) == NULL);

  TEST_ASSERT(ResponseSetErrorObject(res, NULL) == true);
  TEST_ASSERT(ResponseError(res) == NULL);

  /* Set em Response NULL */
  TEST_ASSERT(ResponseSetResultObject(NULL, NULL) == false);
  TEST_ASSERT(ResponseSetErrorObject(NULL, NULL) == false);

  ResponseFree(&res);
  TEST_PASS();
}

/* ============================================================
 * TestResponseCreateHelpers
 * ============================================================ */
static int TestResponseCreateHelpers(void) {
  TEST_LOG("ResponseCreateHelpers");

  /* CreateString */
  Response *r1 = ResponseCreateString(10, "hello world");
  TEST_ASSERT(r1 != NULL);
  TEST_ASSERT(ResponseMsgId(r1) == 10);
  TEST_ASSERT(ResponseResult(r1) != NULL);
  TEST_ASSERT(ResponseError(r1) == NULL);
  TEST_ASSERT(ObjectGetType(ResponseResult(r1)) == OBJ_STR);
  ResponseFree(&r1);

  /* string NULL */
  Response *r1b = ResponseCreateString(11, NULL);
  if (r1b) ResponseFree(&r1b);

  /* CreateError */
  Response *r2 = ResponseCreateError(20, "something went wrong");
  TEST_ASSERT(r2 != NULL);
  TEST_ASSERT(ResponseMsgId(r2) == 20);
  TEST_ASSERT(ResponseError(r2) != NULL);
  TEST_ASSERT(ResponseResult(r2) == NULL);
  ResponseFree(&r2);

  Response *r2b = ResponseCreateError(21, NULL);
  if (r2b) ResponseFree(&r2b);

  /* CreateNilResult */
  Response *r3 = ResponseCreateNilResult(30);
  TEST_ASSERT(r3 != NULL);
  TEST_ASSERT(ResponseMsgId(r3) == 30);
  ResponseFree(&r3);

  /* CreateInt */
  Response *r4 = ResponseCreateInt(40, -12345);
  TEST_ASSERT(r4 != NULL);
  TEST_ASSERT(ResponseMsgId(r4) == 40);
  TEST_ASSERT(ResponseResult(r4) != NULL);
  TEST_ASSERT(ObjectGetType(ResponseResult(r4)) == OBJ_INT);
  TEST_ASSERT(ObjectGetInt((Object*)ResponseResult(r4)) == -12345);
  ResponseFree(&r4);

  Response *r4b = ResponseCreateInt(41, INT64_MAX);
  TEST_ASSERT(r4b != NULL);
  ResponseFree(&r4b);

  Response *r4c = ResponseCreateInt(42, INT64_MIN);
  TEST_ASSERT(r4c != NULL);
  ResponseFree(&r4c);

  /* CreateOk */
  Response *r5 = ResponseCreateOk(50);
  TEST_ASSERT(r5 != NULL);
  TEST_ASSERT(ResponseMsgId(r5) == 50);
  ResponseFree(&r5);

  /* CreateStrArrayResult */
  const char *vec[] = {"one", "two", "three", NULL};
  Response *r6 = ResponseCreateStrArrayResult(60, vec);
  TEST_ASSERT(r6 != NULL);
  TEST_ASSERT(ResponseMsgId(r6) == 60);
  TEST_ASSERT(ResponseResult(r6) != NULL);
  ResponseFree(&r6);

  Response *r6b = ResponseCreateStrArrayResult(61, NULL);
  if (r6b) ResponseFree(&r6b);

  /* CreateResult / CreateResultAudio (smoke) */
  Result dummy_result = {0};
  Response *r7 = ResponseCreateResult(70, dummy_result);
  if (r7) ResponseFree(&r7);

  ResultAudio dummy_audio = {0};
  Response *r8 = ResponseCreateResultAudio(80, dummy_audio);
  if (r8) ResponseFree(&r8);

  TEST_PASS();
}

/* ============================================================
 * TestResponseMarshalUnmarshal
 * ============================================================ */
static int TestResponseMarshalUnmarshal(void) {
  TEST_LOG("ResponseMarshalUnmarshal");

  /* Caso 1: result string */
  Response *orig = ResponseCreateString(100, "marshal_test");
  TEST_ASSERT(orig != NULL);

  RawBuffer *buf = ResponseMarshal(orig);
  TEST_ASSERT(buf != NULL);

  Response *restored = ResponseUnmarshal(buf);
  TEST_ASSERT(restored != NULL);
  TEST_ASSERT(ResponseMsgId(restored) == 100);
  TEST_ASSERT(ResponseResult(restored) != NULL);
  TEST_ASSERT(ResponseError(restored) == NULL);
  TEST_ASSERT(ObjectGetType(ResponseResult(restored)) == OBJ_STR);

  ResponseFree(&restored);
  ResponseFree(&orig);

  /* Caso 2: error */
  Response *err = ResponseCreateError(200, "error_payload");
  TEST_ASSERT(err != NULL);

  buf = ResponseMarshal(err);
  TEST_ASSERT(buf != NULL);

  restored = ResponseUnmarshal(buf);
  TEST_ASSERT(restored != NULL);
  TEST_ASSERT(ResponseMsgId(restored) == 200);
  TEST_ASSERT(ResponseError(restored) != NULL);

  ResponseFree(&restored);
  ResponseFree(&err);

  /* Caso 3: vazia */
  Response *empty = ResponseNew();
  TEST_ASSERT(empty != NULL);
  ResponseSetMsgId(empty, 300);

  buf = ResponseMarshal(empty);
  if (buf) {
    restored = ResponseUnmarshal(buf);
    if (restored) {
      TEST_ASSERT(ResponseMsgId(restored) == 300);
      ResponseFree(&restored);
    }
  }
  ResponseFree(&empty);

  /* Caso 4: Unmarshal NULL */
  Response *bad = ResponseUnmarshal(NULL);
  TEST_ASSERT(bad == NULL);

  TEST_PASS();
}

/* ============================================================
 * TestResponseToString
 * ============================================================ */
static int TestResponseToString(void) {
  TEST_LOG("ResponseToString");

  Response *res = ResponseCreateString(1, "print_me");
  TEST_ASSERT(res != NULL);

  char *str = ResponseToString(res);
  TEST_ASSERT(str != NULL);

  ResponseFree(&res);

  /* ToString de NULL */
  char *null_str = ResponseToString(NULL);
  (void)null_str;

  TEST_PASS();
}

/* ============================================================
 * TestResponseStress
 * ============================================================ */
static int TestResponseStress(void) {
  TEST_LOG("ResponseStress");

  for (int i = 0; i < 80; i++) {
    Response *r = NULL;

    switch (i % 5) {
      case 0:
        r = ResponseCreateString((uint32_t)i, "stress_str");
        break;
      case 1:
        r = ResponseCreateInt((uint32_t)i, i * 10);
        break;
      case 2:
        r = ResponseCreateError((uint32_t)i, "stress_err");
        break;
      case 3:
        r = ResponseCreateNilResult((uint32_t)i);
        break;
      default:
        r = ResponseCreateOk((uint32_t)i);
        break;
    }

    TEST_ASSERT(r != NULL);
    TEST_ASSERT(ResponseMsgId(r) == (uint32_t)i);

    /* Marshal + Unmarshal roundtrip */
    RawBuffer *buf = ResponseMarshal(r);
    if (buf) {
      Response *copy = ResponseUnmarshal(buf);
      if (copy) {
        TEST_ASSERT(ResponseMsgId(copy) == (uint32_t)i);
        ResponseFree(&copy);
      }
    }

    ResponseFree(&r);
  }

  TEST_PASS();
}

/* ============================================================
 * TestResponseObjectTypes (vários tipos de Object)
 * ============================================================ */
static int TestResponseObjectTypes(void) {
  TEST_LOG("ResponseObjectTypes");

  Response *res = ResponseNew();
  TEST_ASSERT(res != NULL);

  /* Bool */
  Object *b = ObjectCreateBool(true);
  TEST_ASSERT(b != NULL);
  ResponseSetResultObject(res, b);
  if (ResponseResult(res) != b) ObjectFree(&b);
  TEST_ASSERT(ResponseResult(res) != NULL);
  TEST_ASSERT(ObjectGetType(ResponseResult(res)) == OBJ_BOOL);

  /* Int */
  Object *i = ObjectCreateInt(-999);
  ResponseSetResultObject(res, i);
  if (ResponseResult(res) != i) ObjectFree(&i);
  TEST_ASSERT(ObjectGetType(ResponseResult(res)) == OBJ_INT);

  /* UInt */
  Object *u = ObjectCreateUInt(123456789ULL);
  ResponseSetResultObject(res, u);
  if (ResponseResult(res) != u) ObjectFree(&u);
  TEST_ASSERT(ObjectGetType(ResponseResult(res)) == OBJ_UINT);

  /* Float */
  Object *f = ObjectCreateFloat(3.14f);
  ResponseSetResultObject(res, f);
  if (ResponseResult(res) != f) ObjectFree(&f);
  TEST_ASSERT(ObjectGetType(ResponseResult(res)) == OBJ_FLOAT);

  /* Double */
  Object *d = ObjectCreateDouble(2.71828);
  ResponseSetResultObject(res, d);
  if (ResponseResult(res) != d) ObjectFree(&d);
  TEST_ASSERT(ObjectGetType(ResponseResult(res)) == OBJ_DOUBLE);

  /* Nil */
  Object *n = ObjectCreateNil();
  ResponseSetResultObject(res, n);
  if (ResponseResult(res) != n) ObjectFree(&n);
  TEST_ASSERT(ObjectGetType(ResponseResult(res)) == OBJ_NIL);

  /* Str */
  Object *s = ObjectCreateStr("final");
  ResponseSetResultObject(res, s);
  if (ResponseResult(res) != s) ObjectFree(&s);
  TEST_ASSERT(ObjectGetType(ResponseResult(res)) == OBJ_STR);

  ResponseFree(&res);
  TEST_PASS();
}

/* ============================================================
 * main
 * ============================================================ */
int main(void) {
  TEST_LOG("Running HazeServerResponse tests");

  int failed = 0;

  failed += TestResponseNewFree() != 0;
  failed += TestResponseMsgId() != 0;
  failed += TestResponseSetObjects() != 0;
  failed += TestResponseCreateHelpers() != 0;
  failed += TestResponseMarshalUnmarshal() != 0;
  failed += TestResponseToString() != 0;
  failed += TestResponseStress() != 0;
  failed += TestResponseObjectTypes() != 0;

  if (failed == 0) {
    TEST_LOG("All HazeServerResponse tests passed");
    return 0;
  }

  TEST_LOG("Some HazeServerResponse tests failed");
  return 1;
}

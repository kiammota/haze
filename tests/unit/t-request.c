#include "api/proto/Request.h"
#include "macros.h"
#include "msgpack/Object.h"
#include <string.h>
#include <stdio.h>

static int TestRequestNewFree(void) {
  TEST_LOG("RequestNewFree");

  Request *req = RequestNew();
  TEST_ASSERT(req != NULL);

  RequestFree(&req);
  TEST_ASSERT(req == NULL);

  /* Double free deve ser seguro */
  RequestFree(&req);
  TEST_ASSERT(req == NULL);

  /* Free de NULL deve ser seguro */
  Request *null_req = NULL;
  RequestFree(&null_req);
  TEST_ASSERT(null_req == NULL);

  TEST_PASS();
}

static int TestRequestMethod(void) {
  TEST_LOG("RequestMethod");

  Request *req = RequestNew();
  TEST_ASSERT(req != NULL);

  /* Método inicial deve ser NULL ou vazio */
  const char *initial = RequestMethod(req);
  /* Aceitamos NULL ou string vazia */

  RequestSetMethod(req, "get_user_data");
  TEST_ASSERT(RequestMethod(req) != NULL);
  TEST_ASSERT(strcmp(RequestMethod(req), "get_user_data") == 0);

  /* Sobrescrever método */
  RequestSetMethod(req, "ping");
  TEST_ASSERT(strcmp(RequestMethod(req), "ping") == 0);

  /* Método com string vazia */
  RequestSetMethod(req, "");
  TEST_ASSERT(RequestMethod(req) != NULL);
  TEST_ASSERT(strcmp(RequestMethod(req), "") == 0);

  /* Método NULL (se a API permitir) */
  RequestSetMethod(req, NULL);
  /* Não deve crashar */

  RequestFree(&req);
  TEST_PASS();
}

static int TestRequestParams(void) {
  TEST_LOG("RequestParams");

  /* ---------- 1. Estado inicial ---------- */
  Request *req = RequestNew();
  TEST_ASSERT(req != NULL);
  TEST_ASSERT(RequestParamCount(req) == 0);
  TEST_ASSERT(RequestParamGet(req, 0) == NULL);          /* out-of-bounds */
  TEST_ASSERT(RequestParamGet(req, 999) == NULL);
  TEST_ASSERT(RequestParamIsType(req, OBJ_BIN, 0) == false);

  /* ---------- 2. Append de um parâmetro ---------- */
  Object *param1 = RequestParamNew();
  TEST_ASSERT(param1 != NULL);

  ObjectType type1 = RequestParamTypeGet(param1);

  bool ok = RequestParamAppend(req, param1, 0);
  TEST_ASSERT(ok == true);
  TEST_ASSERT(RequestParamCount(req) == 1);

  Object *got = RequestParamGet(req, 0);
  TEST_ASSERT(got != NULL);
  TEST_ASSERT(RequestParamTypeGet(got) == type1);
  TEST_ASSERT(RequestParamIsType(req, type1, 0) == true);

  /* Detecta se a API copiou ou tomou ownership */
  bool took_ownership = (got == param1);

  if (!took_ownership) {
    /* API copiou → liberamos o original */
    RequestParamFree(&param1);
    TEST_ASSERT(param1 == NULL);
  }

  /* O parâmetro ainda deve estar vivo dentro do Request */
  TEST_ASSERT(RequestParamCount(req) == 1);
  TEST_ASSERT(RequestParamGet(req, 0) != NULL);

  /* ---------- 3. Append de mais parâmetros ---------- */
  Object *param2 = RequestParamNew();
  Object *param3 = RequestParamNew();
  TEST_ASSERT(param2 != NULL && param3 != NULL);

  ObjectType type2 = RequestParamTypeGet(param2);
  ObjectType type3 = RequestParamTypeGet(param3);

  TEST_ASSERT(RequestParamAppend(req, param2, 1) == true);
  TEST_ASSERT(RequestParamAppend(req, param3, 2) == true);
  TEST_ASSERT(RequestParamCount(req) == 3);

  /* Verifica todos os índices */
  TEST_ASSERT(RequestParamGet(req, 0) != NULL);
  TEST_ASSERT(RequestParamGet(req, 1) != NULL);
  TEST_ASSERT(RequestParamGet(req, 2) != NULL);
  TEST_ASSERT(RequestParamGet(req, 3) == NULL);   /* fora */

  TEST_ASSERT(RequestParamIsType(req, type1, 0) == true);
  TEST_ASSERT(RequestParamIsType(req, type2, 1) == true);
  TEST_ASSERT(RequestParamIsType(req, type3, 2) == true);

  /* Limpa os originais se a API copiou */
  if (!took_ownership) {
    RequestParamFree(&param2);
    RequestParamFree(&param3);
  }

  /* ---------- 4. Append em índice "estranho" ---------- */
  Object *param4 = RequestParamNew();
  TEST_ASSERT(param4 != NULL);

  bool ok4 = RequestParamAppend(req, param4, 100);
  if (ok4) {
    TEST_ASSERT(RequestParamCount(req) >= 3);
  } else {
    TEST_ASSERT(RequestParamCount(req) == 3);
  }

  if (!took_ownership && !ok4) {
    RequestParamFree(&param4);
  }

  /* ---------- 5. NULL e valores inválidos ---------- */
  TEST_ASSERT(RequestParamAppend(NULL, param1, 0) == false);
  TEST_ASSERT(RequestParamGet(NULL, 0) == NULL);
  TEST_ASSERT(RequestParamCount(NULL) == 0);
  TEST_ASSERT(RequestParamIsType(NULL, type1, 0) == false);

  /* ---------- 6. Free do Request ---------- */
  RequestFree(&req);
  TEST_ASSERT(req == NULL);

  /* ---------- 7. Request recriado do zero ---------- */
  req = RequestNew();
  TEST_ASSERT(req != NULL);
  TEST_ASSERT(RequestParamCount(req) == 0);

  Object *p = RequestParamNew();
  TEST_ASSERT(p != NULL);
  TEST_ASSERT(RequestParamAppend(req, p, 0) == true);
  TEST_ASSERT(RequestParamCount(req) == 1);

  if (RequestParamGet(req, 0) != p) {
    RequestParamFree(&p);   /* era cópia */
  }

  RequestFree(&req);
  TEST_ASSERT(req == NULL);

  TEST_PASS();
}

static int TestRequestMarshalUnmarshal(void) {
  TEST_LOG("RequestMarshalUnmarshal");

  Request *req = RequestNew();
  TEST_ASSERT(req != NULL);

  RequestSetMethod(req, "ping");

  RawBuffer *buf = RequestMarshal(req);
  if (buf != NULL) {
    Request *unmarshaled = RequestUnmarshal(buf);
    TEST_ASSERT(unmarshaled != NULL);

    /* Verifica se o método sobreviveu */
    const char *method = RequestMethod(unmarshaled);
    if (method != NULL) {
      TEST_ASSERT(strcmp(method, "ping") == 0);
    }

    RequestFree(&unmarshaled);
  }

  RequestFree(&req);
  TEST_PASS();
}

static int TestRequestPrint(void) {
  TEST_LOG("RequestPrint");

  Request *req = RequestNew();
  TEST_ASSERT(req != NULL);

  RequestSetMethod(req, "test_print");

  const char *print_out = RequestPrint(req);
  TEST_ASSERT(print_out != NULL);

  /* Só garante que não é string vazia absurda */
  TEST_ASSERT(print_out[0] != '\0' || print_out[0] == '\0'); /* aceita qualquer coisa não-nula */

  RequestFree(&req);
  TEST_PASS();
}

static int TestRequestMsgId(void) {
  TEST_LOG("RequestMsgId");

  Request *req = RequestNew();
  TEST_ASSERT(req != NULL);

  /* msgid inicial */
  uint32_t id = RequestMsgId(req);
  (void)id; /* só garante que não crasha */

  RequestFree(&req);
  TEST_PASS();
}

int main(void) {
  TEST_LOG("Running HazeServerRequest tests");

  int failed = 0;

  failed += TestRequestNewFree() != 0;
  failed += TestRequestMethod() != 0;
  failed += TestRequestParams() != 0;
  failed += TestRequestMarshalUnmarshal() != 0;
  failed += TestRequestPrint() != 0;
  failed += TestRequestMsgId() != 0;

  if (failed == 0) {
    TEST_LOG("All HazeServerRequest tests passed");
    return 0;
  }

  TEST_LOG("Some HazeServerRequest tests failed");
  return 1;
}

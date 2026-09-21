#include "api/Dispatcher.h"
#include "Context.h"
#include "api/functions/FnSampleList.h"
#include "api/functions/FnSession.h"
#include "api/proto/Request.h"
#include "api/proto/Response.h"
#include "audio/AudioEngine.h"
#include "audio/SampleList.h"
#include "msgpack/Object.h"
#include "session/Session.h"
#include "uv.h"

#include <stdint.h>
#include <string.h>

DispatchResult DispatchRPCMessage(const Context *ctx, uv_tcp_t *connection,
                                  Request *rq) {
  if (!ctx || !rq)
    return (DispatchResult){
        .response = ResponseCreateError(0, "Invalid context or request."),
        .notification = NULL};

  const AudioEngine *eng = ContextGetAudioEngine(ctx);
  const Session *session = ContextGetSession(ctx);
  const SampleList *sampleList = SessionGetSampleList(session);

  const char *method_name = RequestMethod(rq);
  uint32_t msgid = RequestMsgId(rq);

  if (!method_name)
    return (DispatchResult){
        .response = ResponseCreateError(msgid, "Method not specified."),
        .notification = NULL};

  if (strcmp(method_name, "test/ping") == 0) {
    if (RequestParamCount(rq) != 0)
      return (DispatchResult){
          .response = ResponseCreateError(
              msgid, "Pong, but those arguments weren't necessary."),
          .notification = NULL};

    return (DispatchResult){.response = ResponseCreateString(msgid, "pong!"),
                            .notification = NULL};
  }

  if (strcmp(method_name, "session/create") == 0) {
    if (RequestParamCount(rq) != 1)
      return (DispatchResult){
          .response = ResponseCreateError(msgid, "Expected 1 parameter."),
          .notification = NULL};

    Object *obj = RequestParamGet(rq, 0);

    if (!ObjectExpect(obj, OBJ_STR))
      return (DispatchResult){
          .response = ResponseCreateError(msgid, "Expected a string."),
          .notification = NULL};

    return (DispatchResult){
        .response = ResponseCreateResult(
            msgid, FnSessionCreate(session, eng, ObjectGetStr(obj))),
        .notification = NULL};
  }

  if (strcmp(method_name, "session/get_name") == 0) {
    if (RequestParamCount(rq) != 0)
      return (DispatchResult){
          .response = ResponseCreateError(msgid, "Expected 0 parameters."),
          .notification = NULL};

    return (DispatchResult){
        .response = ResponseCreateString(msgid, FnSessionGetName(session)),
        .notification = NULL};
  }

  if (strcmp(method_name, "session/get_working_time") == 0) {
    if (RequestParamCount(rq) != 0)
      return (DispatchResult){
          .response = ResponseCreateError(msgid, "Expected 0 parameters."),
          .notification = NULL};

    return (DispatchResult){
        .response =
            ResponseCreateInt(msgid, (int64_t)FnSessionGetWorkingTime(session)),
        .notification = NULL};
  }

  if (strcmp(method_name, "samplelist/import") == 0) {
    if (RequestParamCount(rq) != 1)
      return (DispatchResult){
          .response = ResponseCreateError(msgid, "Expected 1 parameter."),
          .notification = NULL};

    Object *obj = RequestParamGet(rq, 0);

    if (!ObjectExpect(obj, OBJ_STR))
      return (DispatchResult){
          .response = ResponseCreateError(msgid, "Expected a string."),
          .notification = NULL};

    SampleImportPayload *p = malloc(sizeof(SampleImportPayload));
    p->path = strdup(ObjectGetStr(obj));

    Job *job = malloc(sizeof(Job));
    job->_connection = connection;
    job->_msgid = msgid;
    job->_type = JOB_SAMPLELIST_ADD;
    job->_payload = p;

    JobQueuePush(ContextGetRequestQueue(ctx), job);

    return (DispatchResult){.response = NULL,
                            .notification =
                                NULL}; // "recebido, processando em outro lugar"
  }

  if (strcmp(method_name, "samplelist/remove") == 0) {
    if (RequestParamCount(rq) != 1)
      return (DispatchResult){
          .response = ResponseCreateError(msgid, "Expected 1 parameter."),
          .notification = NULL};

    Object *obj = RequestParamGet(rq, 0);

    if (!ObjectExpect(obj, OBJ_STR))
      return (DispatchResult){
          .response = ResponseCreateError(msgid, "Expected a string."),
          .notification = NULL};

    return (DispatchResult){
        .response = ResponseCreateResultAudio(
            msgid, FnSampleListRemoveSample(sampleList, ObjectGetStr(obj))),
        .notification = NULL};
  }

  if (strcmp(method_name, "samplelist/play") == 0) {
    if (RequestParamCount(rq) != 1)
      return (DispatchResult){
          .response = ResponseCreateError(msgid, "Expected 1 parameter."),
          .notification = NULL};

    Object *obj = RequestParamGet(rq, 0);

    if (!ObjectExpect(obj, OBJ_STR))
      return (DispatchResult){
          .response = ResponseCreateError(msgid, "Expected a string."),
          .notification = NULL};

    return (DispatchResult){
        .response = ResponseCreateResultAudio(
            msgid, FnSamplePlay(sampleList, ObjectGetStr(obj))),
        .notification = NULL};
  }

  return (DispatchResult){.response =
                              ResponseCreateError(msgid, "Method not found."),
                          .notification = NULL};
}

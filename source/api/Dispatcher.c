#include "api/Dispatcher.h"
#include "Context.h"
#include "api/functions/FnSampleList.h"
#include "api/functions/FnSession.h"
#include "api/proto/Request.h"
#include "api/proto/Response.h"
#include "audio/AudioEngine.h"
#include "audio/Sample.h"
#include "audio/SampleList.h"
#include "msgpack/Object.h"
#include "session/Session.h"
#include "uv.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

DispatchResult DispatchRPCMessage(const Context *ctx,
                                  const uv_tcp_t *connection,
                                  const Request *rq) {
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

  // -- samplelist

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
    const char *path = ObjectGetStr(obj);

    Job *job = malloc(sizeof(Job));
    job->_connection = connection;
    job->_msgid = msgid;
    job->_type = JOB_SAMPLELIST_ADD;
    job->_payload = path;

    JobQueuePush(ContextGetRequestQueue(ctx), job);

    return (DispatchResult){.response = NULL, .notification = NULL};
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

  if (strcmp(method_name, "samplelist/get_sample") == 0) {
    if (RequestParamCount(rq) != 1) {
      return (DispatchResult){
          .response = ResponseCreateError(msgid, "Expected sample name."),
          .notification = NULL};
    }

    Object *name = RequestParamGet(rq, 0);
    if (!ObjectExpect(name, OBJ_STR)) {
      return (DispatchResult){.response =
                                  ResponseCreateError(msgid, "Expected string"),
                              .notification = NULL};
    }

    Sample *sample = SampleListGetSampleByName(sampleList, ObjectGetStr(name));
    Object *idkey = ObjectCreateStr("id");
    Object *sample_name_key = ObjectCreateStr("sample_name");
    Object *volumekey = ObjectCreateStr("volume");
    Object *pitchkey = ObjectCreateStr("pitch");
    Object *is_playingkey = ObjectCreateStr("is_playing");
    Object *sample_ratekey = ObjectCreateStr("sample_rate");
    Object *durationkey = ObjectCreateStr("duration");

    ObjectMapTable *map_table = ObjectMapTableCreate();
    ObjectMapTableSet(map_table, idkey, ObjectCreateUInt(SampleGetId(sample)));
    ObjectMapTableSet(map_table, sample_name_key,
                      ObjectCreateStr(SampleGetName(sample)));
    ObjectMapTableSet(map_table, volumekey,
                      ObjectCreateFloat(SampleGetVolume(sample)));
    ObjectMapTableSet(map_table, pitchkey,
                      ObjectCreateFloat(SampleGetPitch(sample)));
    ObjectMapTableSet(map_table, is_playingkey,
                      ObjectCreateBool(SampleIsPlaying(sample)));
    ObjectMapTableSet(map_table, sample_ratekey,
                      ObjectCreateFloat(SampleGetSampleRate(sample)));
    ObjectMapTableSet(map_table, durationkey,
                      ObjectCreateFloat(SampleGetDuration(sample)));

    Response* res = ResponseNew();
  }
}

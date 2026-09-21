#include "Context.h"
#include "HazeMacros.h"
#include "JobQueue.h"
#include "audio/AudioEngine.h"
#include "fs/InstanceRegistry.h"
#include "fs/Paths.h"
#include "logc/log.h"

#include <stdlib.h>

Context *ContextNew(void) {
  Context *ctx = calloc(1, sizeof(Context));
  if (!ctx) {
    log_error("[context] allocation failed");
    return NULL;
  }

  log_debug("[context] initializing");

  ctx->_audioEngine = AudioEngineNew();
  if (!ctx->_audioEngine) {
    log_error("[context] audio engine failed");
    goto fail;
  }
  log_debug("[context] audio ready");

  ctx->_session = SessionNew(NULL, ctx->_audioEngine);
  if (!ctx->_session) {
    log_error("[context] session failed");
    goto fail;
  }
  log_debug("[context] session ready");

  ctx->_paths = PathsNew();
  if (!ctx->_paths) {
    log_error("[context] paths failed");
    goto fail;
  }
  log_debug("[context] paths ready");

  ctx->_instanceRegFile = InstanceRegistryNew(ctx->_session, ctx->_paths, 7192);
  if (!ctx->_instanceRegFile) {
    log_error("[context] registry failed");
    goto fail;
  }
  log_debug("[context] registry ready");

  ctx->_requests = JobQueueNew();
  if (!ctx->_requests) {
    log_error("[context] request queue failed");
    goto fail;
  }

  ctx->_results = JobQueueNew();
  if (!ctx->_results) {
    log_error("[context] result queue failed");
    goto fail;
  }

  log_debug("[context] ready");
  return ctx;

fail:
  ContextFree(&ctx);
  return NULL;
}

void ContextFree(Context **ctx) {
  PTR_FREE_ASSERT(ctx);

  log_debug("[context] shutting down");

  if ((*ctx)->_results)
    JobQueueFree(&(*ctx)->_results);

  if ((*ctx)->_requests)
    JobQueueFree(&(*ctx)->_requests);

  if ((*ctx)->_instanceRegFile)
    InstanceRegistryFree(&(*ctx)->_instanceRegFile);

  if ((*ctx)->_paths)
    PathsFree(&(*ctx)->_paths);

  if ((*ctx)->_session)
    SessionFree(&(*ctx)->_session);

  if ((*ctx)->_audioEngine)
    AudioEngineFree(&(*ctx)->_audioEngine);

  free(*ctx);
  *ctx = NULL;

  log_debug("[context] destroyed");
}

const Session *ContextGetSession(const Context *ctx) { return ctx->_session; }

const Paths *ContextGetPaths(const Context *ctx) { return ctx->_paths; }

const InstanceReg *ContextGetInstanceRegistry(const Context *ctx) {
  return ctx->_instanceRegFile;
}

const AudioEngine *ContextGetAudioEngine(const Context *ctx) {
  return ctx->_audioEngine;
}

const JobQueue *ContextGetRequestQueue(const Context *ctx) {
  return ctx->_requests;
}

const JobQueue *ContextGetResultQueue(const Context *ctx) {
  return ctx->_results;
}

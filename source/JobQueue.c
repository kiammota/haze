#include "JobQueue.h"
#include "logc/log.h"

#include <stdlib.h>
#include <string.h>

static void JobQueueAsyncCallback(uv_async_t *handle) {
  (void)handle;
}

static void JobQueueCloseCallback(uv_handle_t *handle) {
  JobQueue *jq = handle->data;

  if (!jq)
    return;

  free(jq->jobs);
  uv_mutex_destroy(&jq->lock);
  free(jq);

  log_debug("[jobqueue] destroyed");
}

JobQueue *JobQueueNew(void) {
  JobQueue *jq = calloc(1, sizeof(JobQueue));

  if (!jq) {
    log_error("[jobqueue] allocation failed");
    return NULL;
  }

  log_debug("[jobqueue] initializing");

  if (uv_mutex_init(&jq->lock) != 0) {
    log_error("[jobqueue] mutex initialization failed");
    free(jq);
    return NULL;
  }

  int rc = uv_async_init(
    uv_default_loop(),
    &jq->awake,
    JobQueueAsyncCallback
  );

  if (rc != 0) {
    log_error(
      "[jobqueue] async initialization failed: %s",
      uv_strerror(rc)
    );

    uv_mutex_destroy(&jq->lock);
    free(jq);
    return NULL;
  }

  jq->awake.data = jq;

  log_debug("[jobqueue] ready");

  return jq;
}

void JobQueueFree(JobQueue **jq) {
  if (!jq || !*jq)
    return;

  JobQueue *queue = *jq;
  *jq = NULL;

  log_debug("[jobqueue] closing (%zu jobs)", queue->len);

  uv_close(
    (uv_handle_t *)&queue->awake,
    JobQueueCloseCallback
  );
}

Result JobQueuePush(JobQueue *jq, Job *job) {
  if (!jq || !job)
    return ResultMsgE("invalid argument");

  uv_mutex_lock(&jq->lock);

  if (jq->len >= jq->capacity) {
    size_t newCapacity =
      jq->capacity == 0 ? 4 : jq->capacity * 2;

    Job **newJobs = realloc(
      jq->jobs,
      sizeof(Job *) * newCapacity
    );

    if (!newJobs) {
      uv_mutex_unlock(&jq->lock);
      log_error("[jobqueue] resize failed");
      return ResultMsgE("out of memory");
    }

    jq->jobs = newJobs;
    jq->capacity = newCapacity;
  }

  jq->jobs[jq->len++] = job;

  size_t size = jq->len;

  uv_mutex_unlock(&jq->lock);

  int rc = uv_async_send(&jq->awake);

  if (rc != 0) {
    log_warn(
      "[jobqueue] wake failed: %s",
      uv_strerror(rc)
    );
  }

  log_debug("[jobqueue] push (size=%zu)", size);

  return ResultOk();
}

Result JobQueuePop(JobQueue *jq, Job **outJob) {
  if (!jq || !outJob)
    return ResultMsgE("invalid argument");

  *outJob = NULL;

  uv_mutex_lock(&jq->lock);

  if (jq->len == 0) {
    uv_mutex_unlock(&jq->lock);
    return ResultMsgE("queue is empty");
  }

  *outJob = jq->jobs[0];

  if (jq->len > 1) {
    memmove(
      &jq->jobs[0],
      &jq->jobs[1],
      sizeof(Job *) * (jq->len - 1)
    );
  }

  jq->len--;

  size_t size = jq->len;

  uv_mutex_unlock(&jq->lock);

  log_debug("[jobqueue] pop (size=%zu)", size);

  return ResultOk();
}

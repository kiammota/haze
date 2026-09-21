#ifndef THREAD_BRIDGE_H
#define THREAD_BRIDGE_H

#include "Result.h"
#include <stddef.h>
#include <stdint.h>
#include <uv.h>

typedef enum {
  JOB_SAMPLELIST_ADD,
  JOB_SAMPLELIST_REMOVE,
} JobType;

typedef struct {
  uint32_t _msgid;
  uv_tcp_t *_connection;
  JobType _type;
  void *_payload;
} Job;

typedef struct {
  uv_mutex_t lock;
  size_t capacity;
  size_t len;
  Job **jobs;
  uv_async_t awake;
} JobQueue;

JobQueue *JobQueueNew(void);
void JobQueueFree(JobQueue **jq);

Result JobQueuePush(JobQueue *jq, Job *job);        
Result JobQueuePop(JobQueue *jq, Job **outJob); 

static inline size_t JobQueueSize(const JobQueue *jq) {
  return jq ? jq->len : 0;
}

#endif

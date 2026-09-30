#ifndef AUDIO_THREAD_H
#define AUDIO_THREAD_H

#include "JobQueue.h"
#include "Result.h"
#include <uv.h>

typedef struct Context Context;

typedef struct AudioThread {
  uv_thread_t thread;
  uv_loop_t loop;

  JobQueue *incoming;   
  uv_async_t *mainWake; 

  const Context *ctx;
} AudioThread;

AudioThread *AudioThreadNew(const Context *ctx, uv_async_t *mainWake);
Result AudioThreadStart(AudioThread *at);
void AudioThreadStop(AudioThread *at);
void AudioThreadFree(AudioThread **at);

JobQueue *AudioThreadGetQueue(const AudioThread *at);

#endif

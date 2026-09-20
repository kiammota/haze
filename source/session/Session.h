#ifndef SESSION_H
#define SESSION_H

#include "audio/AudioEngine.h"
#include "audio/ChannelList.h"
#include "audio/SampleList.h"
#include <stdbool.h>
#include <time.h>

typedef struct {
  char *session_name;
  char *project_path;
  time_t created_at;
  time_t working_time;
  SampleList *SampleList;
  ChannelList *ChannelList;
} Session;

Session *SessionNew(const char *SessionName, const AudioEngine* eng);
void SessionFree(Session **s);
bool SessionSetName(Session *s, const char *SessionName);
static inline const char *SessionGetName(const Session *s) {
  if (!s) return "";
  if (!s->session_name) return "";
  return s->session_name;
}
static inline time_t SessionGetWorkingTime(const Session *s) {
  return time(NULL) - s->created_at;
}
static inline time_t SessionGetCreatedAt(const Session *s) {
  return s->created_at;
}

const SampleList *SessionGetSampleList(const Session *s);
const ChannelList *SessionGetChannelList(const Session *s);

#endif

#include "Session.h"
#include "HazeMacros.h"
#include "audio/AudioEngine.h"
#include "audio/ChannelList.h"
#include "audio/SampleList.h"
#include "logc/log.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NAME_MAX 15
#define CLARA_NAME 67 + 76 + 65 + 82 + 65
static char *session_name_random(void) {
  srand(CLARA_NAME ^ (unsigned int)time(NULL));
  const char *first[NAME_MAX] = {
      "goofball", "biscuit", "wombat", "noodle", "badger",
      "gizmo",    "doofus",  "gecko",  "rascal", "walrus",
      "dingo",    "muppet",  "turnip", "hobbit", "chimp",
  };

  const char *second[NAME_MAX] = {
      "flabby", "haughty",  "clumsy", "grumpy", "sneaky",
      "goofy",  "sluggish", "silly",  "cheeky", "wobbly",
      "cranky", "greedy",   "shabby", "lousy",  "spunky",
  };

  int first_v = rand() % NAME_MAX;
  int second_v = rand() % NAME_MAX;
  int tamanho = strlen(first[first_v]) + 1 + strlen(second[second_v]) + 1;

  char *name = malloc(tamanho);
  snprintf(name, tamanho, "%s_%s", first[first_v], second[second_v]);
  return name;
}

Session *SessionNew(const char *session_name, const AudioEngine* eng) {
  Session *s = malloc(sizeof(Session));
  log_debug("SESSION Initializated.");

  if (!s)
    return NULL;

  s->created_at = time(NULL);
  s->project_path = NULL;

  if (session_name == NULL)
    s->session_name = session_name_random();
  else
    s->session_name = strdup(session_name);

  s->SampleList = SampleListNew();
  log_debug("SESSION SampleList started.");
  s->ChannelList = ChannelListNew(eng);
  log_debug("SESSION ChannelList started.");
   
  return s;
}

bool SessionSetName(Session *s, const char *SessionName) {
  free(s->session_name);
  s->session_name = strdup(SessionName);
  return s->session_name != NULL;
}

const SampleList *SessionGetSampleList(const Session *s) {
  return s->SampleList;
}

void SessionFree(Session **s) {
  PTR_FREE_ASSERT(s);
  free((*s)->session_name);
  free((*s)->project_path);
  SampleListFree(&(*s)->SampleList);
  free((*s));
  *s = NULL;
}

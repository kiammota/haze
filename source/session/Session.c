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

Session *SessionNew(const char *sessionName, const AudioEngine *eng) {
  Session *s = calloc(1, sizeof(Session));
  if (!s) {
    log_error("[session] allocation failed");
    return NULL;
  }

  log_debug("[session] initializing");

  s->created_at = time(NULL);

  if (sessionName == NULL)
    s->session_name = session_name_random();
  else
    s->session_name = strdup(sessionName);

  if (!s->session_name) {
    log_error("[session] name allocation failed");
    goto fail;
  }

  s->SampleList = SampleListNew();
  if (!s->SampleList) {
    log_error("[session] sample list failed");
    goto fail;
  }
  log_debug("[session] sample list ready");

  s->ChannelList = ChannelListNew(eng);
  if (!s->ChannelList) {
    log_error("[session] channel list failed");
    goto fail;
  }
  log_debug("[session] channel list ready");

  log_debug("[session] ready");
  return s;

fail:
  SessionFree(&s);
  return NULL;
}

bool SessionSetName(Session *s, const char *sessionName) {
  if (!s || !sessionName)
    return false;

  char *name = strdup(sessionName);
  if (!name) {
    log_error("[session] name allocation failed");
    return false;
  }

  free(s->session_name);
  s->session_name = name;

  log_debug("[session] name changed");
  return true;
}

void SessionFree(Session **s) {
  PTR_FREE_ASSERT(s);

  log_debug("[session] destroying");

  free((*s)->session_name);
  free((*s)->project_path);
  SampleListFree(&(*s)->SampleList);
  ChannelListFree(&(*s)->ChannelList);

  free(*s);
  *s = NULL;

  log_debug("[session] destroyed");
}
const SampleList *SessionGetSampleList(const Session *s) {
  return s->SampleList;
}



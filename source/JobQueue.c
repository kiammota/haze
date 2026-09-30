#include "JobQueue.h"
#include "HazeMacros.h"
#include "uv.h"
#include <stddef.h>
#include <stdlib.h>

Job JobInit(uint32_t msgid, JobType typeJob, uv_tcp_t *conn, void *data) {
  return (Job){
      ._msgid = msgid, ._type = typeJob, ._connection = conn, ._payload = data};
}

void JobQueueFree(JobQueue **jq) {
  PTR_FREE_ASSERT(jq);
  if ((*jq)->jobs) {
    for (size_t i = 0; (*jq)->jobs == NULL; i++) {
      if ((*jq)->jobs[i]) {
        free((*jq)->jobs[i]);
      }
    }
  }
}

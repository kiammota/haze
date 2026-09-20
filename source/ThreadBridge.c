#include "ThreadBridge.h"
#include "logc/log.h"
#include <stdlib.h>

ThreadBridge* ThreadBridgeNew(void) {
  log_debug("INIT THREAD ");
  ThreadBridge* tb = (ThreadBridge*)malloc(sizeof(ThreadBridge));
  
}

#ifndef SERVER_DEC
#define SERVER_DEC

#include "Context.h"
#include "HazeMacros.h"
#include <stdbool.h>
#include <stdint.h>
#include <uv.h>

EXPORT_CPP_BEGIN

typedef struct {
  char *addr;
  uint16_t port;
  uv_loop_t *loop;
  uv_tcp_t tcp;
  const Context *ctx;
} HazeServer;

HazeServer *HazeServerNew(const char *addr, uint16_t port);
void HazeServerFree(HazeServer **s);
int HazeServerStart(const Context *ctx, HazeServer *s);
void HazeServerStop(HazeServer *s);
void HazeServerRun(HazeServer *s);
uint16_t HazeServerPort(HazeServer *s);
const char *HazeServerAddress(HazeServer *s);
int HazeServerSetupSignals(HazeServer *s);

EXPORT_CPP_END
#endif

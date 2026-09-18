#ifndef HAZE_SERVER_MIDDLEWARE_H
#define HAZE_SERVER_MIDDLEWARE_H

#include <stddef.h>
#include <string.h>

typedef enum {
  HAZE_MW_REJECT,
  HAZE_MW_NEXT

} HazeServerMiddlewareResult;

HazeServerMiddlewareResult HazeServerMiddlewareIsHttp(const void *data, size_t len);

#endif

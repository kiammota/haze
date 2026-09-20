#include "HazeServerMiddleware.h"

HazeServerMiddlewareResult HazeServerMiddlewareIsHttp(const void *data, size_t len)
{
    if (!data || len < 4) return HAZE_MW_NEXT;

    const char *methods[] = {
        "GET ", "POST", "PUT ", "HEAD",
        "DELE", "PATC", "OPTI", NULL
    };

    for (int i = 0; methods[i]; i++) {
        if (memcmp(data, methods[i], 4) == 0)
            return HAZE_MW_REJECT;
    }

    return HAZE_MW_NEXT;
}

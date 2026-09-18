#include "strdup.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

char *StrDup(const char *str) {
    if (!str)
        return NULL;

    size_t len = strlen(str) + 1;
    char *copy = malloc(len);

    if (!copy)
        return NULL;

    memcpy(copy, str, len);
    return copy;
}

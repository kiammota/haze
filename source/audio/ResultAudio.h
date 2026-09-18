// ResultAudio.h
#ifndef RESULT_AUDIO_H
#define RESULT_AUDIO_H


#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static inline char *ResultAudioStrdup(const char *src) {
    size_t len = strlen(src) + 1;
    char *copy = malloc(len);

    if (!copy)
        return NULL;

    memcpy(copy, src, len);
    return copy;
}

typedef struct {
    bool success;
    const char *msg;
    bool owned;   // true = msg foi alocado, precisa de ResultAudioFree
} ResultAudio;

static inline ResultAudio ResultAudioOk(void) {
    return (ResultAudio){ .success = true, .msg = NULL, .owned = false };
}

// pra mensagem fixa, string literal — sem alocação, sem custo
static inline ResultAudio ResultAudioErr(const char *msg) {
    return (ResultAudio){ .success = false, .msg = msg, .owned = false };
}

// pra mensagem com detalhe dinâmico — aloca, precisa de free depois
static inline ResultAudio ResultAudioErrF(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return (ResultAudio){ .success = false, .msg = ResultAudioStrdup(buf), .owned = true };
}

static inline bool ResultAudioIsOk(ResultAudio r) {
    return r.success;
}

static inline void ResultAudioFree(ResultAudio *r) {
    if (r->owned && r->msg) {
        free((char *)r->msg);
        r->msg = NULL;
        r->owned = false;
    }
}

#endif

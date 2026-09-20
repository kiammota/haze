#ifndef THREAD_BRIDGE_H
#define THREAD_BRIDGE_H

#include <uv.h>
#include <stddef.h>

typedef struct PlaylistJob PlaylistJob;   // definido em outro header, só precisamos do tipo aqui
typedef struct JobResult JobResult;

typedef struct {
    PlaylistJob *jobs;
    size_t       quantity;
    size_t       capacity;
    uv_mutex_t   lock;
    uv_async_t   wake;      // campainha: mora no loop de quem CONSOME essa fila
} JobQueue;

typedef struct {
    JobResult  *results;
    size_t      quantity;
    size_t      capacity;
    uv_mutex_t  lock;
    uv_async_t  wake;       // campainha: mora no loop de quem CONSOME essa fila
} ResultQueue;

typedef struct {
    JobQueue    requests;   // Thread A escreve, Thread B lê
    ResultQueue results;    // Thread B escreve, Thread A lê
} ThreadBridge;

ThreadBridge *ThreadBridgeNew(void);
void ThreadBridgeFree(ThreadBridge **bridge);

#endif

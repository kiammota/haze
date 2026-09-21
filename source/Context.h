#ifndef CONTEXT_H
#define CONTEXT_H

#include "JobQueue.h"
#include "audio/AudioEngine.h"
#include "fs/InstanceRegistry.h"
#include "fs/Paths.h"
#include "session/Session.h"

/**
 * @brief Global context of a Haze instance.
 *
 * Holds the main resources owned by the current Haze process
 * and provides a single context through which they can be accessed.
 */
typedef struct {
  Session* _session;
  Paths* _paths;
  InstanceReg* _instanceRegFile;
  AudioEngine* _audioEngine;
  JobQueue* _requests;
  JobQueue* _results;
} Context;

/**
 * @brief Creates a new global context.
 *
 * Initializes the resources owned by the Haze instance.
 *
 * @return A pointer to the created context, or NULL on failure.
 */
Context* ContextNew(void);

/**
 * @brief Frees a global context.
 *
 * Releases all resources owned by the context and then frees
 * the context itself.
 *
 * @param gi Pointer to the context pointer.
 */
void ContextFree(Context** gi);

/**
 * @brief Gets the session associated with the context.
 *
 * @param gi Global context.
 * @return A constant pointer to the session.
 */
const Session* ContextGetSession(const Context* gi);

/**
 * @brief Gets the paths used by the Haze instance.
 *
 * @param gi Global context.
 * @return A constant pointer to the paths.
 */
const Paths* ContextGetPaths(const Context* gi);

/**
 * @brief Gets the instance registry associated with the context.
 *
 * @param gi Global context.
 * @return A constant pointer to the instance registry.
 */
const InstanceReg* ContextGetInstanceRegistry(const Context* gi);

const AudioEngine* ContextGetAudioEngine(const Context* ctx);

const JobQueue* ContextGetRequestQueue(const Context* ctx);
const JobQueue* ContextgetResultQueue(const Context* ctx);
#endif

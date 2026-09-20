#ifndef RAW_BUFFER_H
#define RAW_BUFFER_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief A simple container for raw binary data.
 */
typedef struct {
  void *data;
  size_t len;
} RawBuffer;

/**
 * @brief Creates a new raw buffer.
 *
 * @param data Pointer to the initial data.
 * @param len Size of the data in bytes.
 *
 * @return A pointer to the newly created RawBuffer, or NULL if allocation fails.
 *
 * @note Ownership of @p data should be defined by the implementation.
 */
RawBuffer *RawBufferNew(void *data, size_t len);

static inline RawBuffer RawBufferInit(void* data, size_t len)  {
  RawBuffer rb = {.data = data, .len = len};
  return rb;
}

/**
 * @brief Frees a raw buffer and its associated data.
 *
 * The buffer pointer is set to NULL after being freed.
 *
 * @param bf Address of the RawBuffer pointer to free.
 */
void RawBufferFree(RawBuffer **bf);

bool RawBufferAppend(RawBuffer*bf, void* data, size_t len);

char* RawBufferToString(RawBuffer* bf);

/**
 * @brief Returns the data stored in the buffer.
 *
 * @param b The buffer to access.
 *
 * @return A pointer to the buffer data, or NULL if unavailable.
 */
const void *RawBufferData(RawBuffer *b);

/**
 * @brief Returns the size of the buffer in bytes.
 *
 * @param b The buffer to access.
 *
 * @return The size of the stored data.
 */
size_t RawBufferLen(RawBuffer *b);

/**
 * @brief Replaces the data stored in the buffer.
 *
 * @param b The buffer to modify.
 * @param data Pointer to the new data.
 * @param len Size of the new data in bytes.
 *
 * @return true if the operation succeeds, otherwise false.
 *
 * @note Ownership of @p data should be defined by the implementation.
 */
bool RawBufferSetData(RawBuffer *b, void *data, size_t len);
/**
 * @brief Clears the memory contents of the buffer with zeros.
 *
 * @param b The buffer to clear.
 *
 * @return true if the buffer contents were successfully cleared, otherwise false.
 */
bool RawBufferClear(RawBuffer* b);

/**
 * @brief Checks whether the buffer is NULL or contains no data.
 *
 * A buffer is considered null or empty when the pointer itself is NULL,
 * its data pointer is NULL, or its size is zero.
 *
 * @param b The buffer to check.
 *
 * @return true if the buffer is NULL or empty, otherwise false.
 */
static inline bool RawBufferIsNullOrEmpty(RawBuffer* b)
{
    if (b == NULL)
        return true;

    return b->data == NULL || b->len == 0;
}

RawBuffer* RawBufferDup(RawBuffer* bf);

#endif

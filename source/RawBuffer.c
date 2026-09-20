#include "RawBuffer.h"
#include "HazeMacros.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RawBuffer *RawBufferNew(void *data, size_t len) {
  RawBuffer *buffer = malloc(sizeof(RawBuffer));

  if (!buffer)
    return NULL;

  buffer->data = NULL;
  buffer->len = 0;

  if (data && len > 0) {
    buffer->data = malloc(len);

    if (!buffer->data) {
      free(buffer);
      return NULL;
    }

    memcpy(buffer->data, data, len);
    buffer->len = len;
  }

  return buffer;
}

void RawBufferFree(RawBuffer **b) {
  PTR_FREE_ASSERT(b);
  free((*b)->data);
  (*b)->len = 0;
  free(*b);
  *b = NULL;
}

bool RawBufferAppend(RawBuffer *bf, void *data, size_t len) {
  if (!bf || !bf->data)
    return false;
  if (!len)
    return false;
  void *new_data = realloc(bf->data, bf->len + len);
  bf->data = new_data;
  bf->len += len;
  memcpy((unsigned char *)bf->data + bf->len, data, len);
  return true;
}

const void *RawBufferData(RawBuffer *b) {
  if (!b)
    return NULL;
  return b->data;
}
size_t RawBufferLen(RawBuffer *b) {
  if (!b)
    return 0;
  return b->len;
}

bool RawBufferSetData(RawBuffer *b, void *data, size_t len) {
  if (!b)
    return false;
  void *new_data = realloc(b->data, len);
  if (!new_data && len > 0) {
    return false;
  }

  // Agora que o espaço está garantido, copiamos os bytes do dado de origem
  if (data && len > 0) {
    memcpy(new_data, data, len);
  }

  b->data = new_data;
  b->len = len;
  return true;
}

bool RawBufferClear(RawBuffer *b) {
  if (!b || !b->data || b->len == 0) {
    return false;
  }

  memset(b->data, 0, b->len);
  return true;
}

char *RawBufferToString(RawBuffer *bf) {
  char *str = NULL;

  snprintf(str, bf->len, "(rb)[%d]", bf->data);
  return str;
}

RawBuffer* RawBufferDup(RawBuffer* bf) {
  RawBuffer* newBuff = RawBufferNew(bf->data, bf->len);
  if (!newBuff) return NULL;
  return newBuff;

}

#ifndef DECODER_H
#define DECODER_H

#include "RawBuffer.h"
#include "msgpack/Object.h"
#include "msgpack/Types.h"
#include <stddef.h>
typedef struct {
  RawBuffer* buff;
  size_t pos;
} Decoder;

Decoder* DecoderNew(RawBuffer* buff);
void DecoderFree(Decoder** dc);
bool DecoderExpect(Decoder* d, TypeEnum type);
TypeEnum DecoderGetType(Decoder* d);
Object* DecoderGetObject(Decoder* d);

#endif

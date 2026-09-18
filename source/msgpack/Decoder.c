// #include "Decoder.h"
// #include "HazeMacros.h"
// #include "RawBuffer.h"
// #include "msgpack/Object.h"
// #include "msgpack/Types.h"
// #include <stdint.h>
// #include <stdlib.h>
// #include <sys/types.h>
//
// Decoder *DecoderNew(RawBuffer *buff) {
//   if (RawBufferIsNullOrEmpty(buff)) {
//     return NULL;
//   }
//   Decoder *dc = (Decoder *)malloc(sizeof(Decoder));
//   dc->buff = RawBufferDup(buff);
//   dc->pos = 0;
//   return dc;
// }
//
// void DecoderFree(Decoder **dc) {
//   PTR_FREE_ASSERT(dc)
//
//   RawBufferFree(&(*dc)->buff);
//   free(*dc);
//   *dc = NULL;
// }
//
// bool DecoderExpect(Decoder *d, TypeEnum type) {
//   if (!d)
//     return false;
//   if (!d->buff)
//     return false;
//
//   const uint8_t *data = RawBufferData(d->buff);
//
//   if (d->pos >= RawBufferLen(d->buff))
//     return false;
//
//   uint8_t tag = data[d->pos];
//   bool res = tag == type;
//   if (res)
//     d->pos += 1;
//   return res;
// }
//
// TypeEnum DecoderGetType(Decoder *d) {
//   if (!d || RawBufferIsNullOrEmpty(d->buff))
//     return 0;
//
//   if (d->pos >= RawBufferLen(d->buff))
//     return 0;
//
//   const uint8_t *data = RawBufferData(d->buff);
//   uint8_t tag = data[d->pos];
//
//   return (TypeEnum)tag;
// }
//
// static bool DecoderCanRead(Decoder *d, size_t n) {
//     if (!d || !d->buff)
//         return false;
//
//     return d->pos <= RawBufferLen(d->buff) &&
//            n <= RawBufferLen(d->buff) - d->pos;
// }
//
// static bool DecoderReadU8(Decoder *d, uint8_t *out) {
//     if (!DecoderCanRead(d, 1))
//         return false;
//
//     const uint8_t *data = RawBufferData(d->buff);
//
//     *out = data[d->pos];
//     d->pos++;
//
//     return true;
// }
//
// static bool DecoderReadU32(Decoder *d, uint32_t *out) {
//     if (!DecoderCanRead(d, 4))
//         return false;
//
//     const uint8_t *data = RawBufferData(d->buff);
//
//     *out = ((uint32_t)data[d->pos] << 24) |
//            ((uint32_t)data[d->pos + 1] << 16) |
//            ((uint32_t)data[d->pos + 2] << 8) |
//            ((uint32_t)data[d->pos + 3]);
//
//     d->pos += 4;
//
//     return true;
// }
//
// static bool DecoderReadU64(Decoder *d, uint64_t *out) {
//     if (!DecoderCanRead(d, 8))
//         return false;
//
//     const uint8_t *data = RawBufferData(d->buff);
//
//     *out = ((uint64_t)data[d->pos] << 56) |
//            ((uint64_t)data[d->pos + 1] << 48) |
//            ((uint64_t)data[d->pos + 2] << 40) |
//            ((uint64_t)data[d->pos + 3] << 32) |
//            ((uint64_t)data[d->pos + 4] << 24) |
//            ((uint64_t)data[d->pos + 5] << 16) |
//            ((uint64_t)data[d->pos + 6] << 8) |
//            ((uint64_t)data[d->pos + 7]);
//
//     d->pos += 8;
//
//     return true;
// }
//
// static bool DecoderReadU16(Decoder *d, uint16_t *out) {
//     if (!DecoderCanRead(d, 2))
//         return false;
//
//     const uint8_t *data = RawBufferData(d->buff);
//
//     *out = ((uint16_t)data[d->pos] << 8) |
//            ((uint16_t)data[d->pos + 1]);
//
//     d->pos += 2;
//
//     return true;
// }
//
// Object *DecoderGetObject(Decoder *d) {
//
//   if (!d || RawBufferIsNullOrEmpty(d->buff))
//     return 0;
//
//   TypeEnum type = DecoderGetType(d);
//
//   switch (type) {
//     case Type_Nil:
//
//
//   }
//
//
//
//
// }

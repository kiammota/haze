#ifndef HAZESAMPLE_DEC
#define HAZESAMPLE_DEC

#include "audio/AudioEngine.h"
#include "audio/ResultAudio.h"
#include "miniaudio/miniaudio.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


typedef struct {
  uint64_t id;
  ma_sound handle;
  ma_decoder decoder;
  uint8_t *buf;
  size_t buf_size;
  float volume;
  float pitch;
  float duration;
  ma_uint32 sample_rate;
  bool is_playing;
  char *sample_name;
} Sample;

Sample *SampleNew(void);
void SampleFree(Sample **s);

ResultAudio SampleRename(Sample *s, const char *newName);
ResultAudio SampleInit(Sample *s, const AudioEngine *eng, const char *sample_name,
                  const uint8_t *data, size_t size);
ResultAudio SampleInitFromFile(Sample *s, const AudioEngine* eng, const char *path);
ResultAudio SampleSeek(Sample *s, double seconds);
ResultAudio SampleSetVolume(Sample *s, float v);
ResultAudio SamplePlay(Sample *s);
ResultAudio SampleStop(Sample *s);

/* getters */
bool SampleIsPlaying(Sample *s);
float SampleGetVolume(Sample *s);
float SampleGetPitch(Sample *s);
float SampleGetDuration(Sample *s);
float SampleGetSampleRate(Sample *s);
float SampleGetCursor(Sample *s);
uint64_t SampleGetId(Sample* s);
const char *SampleGetName(Sample *s);

#endif

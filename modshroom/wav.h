#ifndef WAV_H
#define WAV_H
// uses only 8-bit samples, mono
int writeAmigaWav(uint8_t *source, int length, uint8_t *filename, int sample_rate);
#endif

#ifndef UTIL_H
#define UTIL_H

//#define FALSE 0
//#define TRUE 1

uint64_t dumpArrayToDisk(uint8_t *data, uint32_t length, uint8_t *output_path);
uint64_t getFilesize(FILE *fp);

uint8_t *intToBin(int a, uint8_t *buffer, int buf_size);
uint8_t *intToBin2(int a, uint8_t *buffer, int buf_size);
// these write three bytes to output
uint8_t *charToHex(uint8_t a, uint8_t *output);
uint8_t *charToNiceHex(uint8_t a, uint8_t *output);
// AMIGAA
short swapBytes(short in);

#endif

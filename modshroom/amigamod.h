// stupid ancient obsolete include guard relic cruft
#ifndef AMIGAMOD_H
#define AMIGAMOD_H

#define MOD_CHANNEL_SIZE 256	// amigamod channel size in byets
#define MOD_NOTE_SIZE 4

#define MOD_SAMPLE_NAME_MAX_LENGTH 23	// 22 chars and the null terminator
#define MODULE_TYPE_UNKNOWN 69
#define MODULE_TYPE_4CHAN 0
#define MODULE_TYPE_8CHAN 1
#define MODULE_TYPE_28CHAN 2
#define MOD_ROWS 64
#define MOD_CHANNELS 4	// we don't need more this time

#define MAX_SAMPLES 31

#define EMPTY_NOTE_VALUE -999 // just a magic number I made up

#include <stdint.h>

typedef struct Sample {
	uint8_t name[22];
	uint16_t length;	// Converted to little-endian, needs to be multiplied by two
	int8_t finetune;
	uint8_t volume;	// values 0-64 are used
	uint16_t repeat;	// converted to little-endian
	uint16_t repeat_length;	// converted to little-endian, needs to be multiplied by two
} Sample;

// not actual amiga datatypes
typedef struct Note {
	int16_t pitch;		// semitones from C-4
	uint8_t instrument;
	uint8_t command;
	uint8_t parameters;
} Note;

// only for 4 channel modules
typedef struct Track {
	Note notes[4];
} Track;

// for 8 channel modules
typedef struct Track8 {
	Note notes[8];
} Track8;

typedef struct Pattern {
	Track tracks[64];
} Pattern;

// pattern of an 8 channel module
typedef struct Pattern8 {
	Track tracks[64];
} Pattern8;

//
// some intermediate form crap
//

typedef struct Songinfo {
	uint8_t name[256];
	uint8_t bpm;
	uint8_t pattern_amount;
	uint8_t length;	// song length in patterns
	uint8_t orderlist[256];	// MOD pattern length is inside the range 1-128
	uint8_t channels;
	uint8_t rows;	// rows per pattern, usually 64
} Songinfo;

typedef struct Instrument {
	// amigamod fields
	uint8_t name[256];
	uint16_t length;
	int8_t finetune;
	int8_t octave; // octave offset from C-4
	uint8_t volume;
	uint16_t repeat;
	uint16_t repeat_length;	// converted to little-endian, needs to be multiplied by two
	// additional fields
	uint8_t envelope;
	int8_t pan;
} Instrument;

typedef struct Instrumentinfo {
	uint8_t instrument_amount;
	Instrument instruments[256];	// usually only 31 is in use
} Instrumentinfo;


// amiga stuff again

int getModuleType(uint8_t *magic);
int loadSampleInfo(uint8_t *moduledata, Sample *sample_info, int sample_num);	// TODO make these return the destination instead of an int
void loadOrderData(uint8_t *moduledata, uint8_t *song_order, int song_data_offset, int song_order_num);
int getHighestOrder(uint8_t *moduledata, uint8_t *song_order, int song_order_num);
int dumpProtrackerSamples(uint8_t *moduledata, Sample *sample_info, int sample_data_offset);
// parses note to custom data presentation format
int parseNote(uint8_t *notedata, Note *output);
void printNote(Note *note);
int periodToSemitone(int period);
uint8_t *getNoteName(int semitones, uint8_t *output);
// make sure output has 20 bytes available
uint8_t * loadModuleName(uint8_t *moduledata, uint8_t *output);
uint8_t * loadModuleMagic(uint8_t *moduledata, uint8_t *output);

uint32_t getChannelAmount(uint8_t *moduledata);
void printOrderlist(uint8_t *song_order, uint8_t song_length);
void printSamples(Sample *sample_info, uint8_t sample_amount);

// intermediate form util functions

void setDefaultInstrumentValues(Instrument *ins);
uint32_t loadInstruments(Sample *sample_info, Instrumentinfo *sinstruments);
void printPattern(Note *synthnotes,Songinfo *ssong, uint32_t pattern);

#endif

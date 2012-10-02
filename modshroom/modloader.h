#ifndef MODLOADER_H
#define MODLOADER_H

#include "amigamod.h"

#define TRACKER_CHANNELS 4	// these values are hardcoded for now
#define TRACKER_ROWS 64

#define PATTERN_DEBUG 0
//#define EMPTY_NOTE_PITCH -999 //defined in amigamod.h

// not a real tracker format, just an intermediate form
typedef struct TrackerNote {
	int pitch;	// semitone offset from C4
	int instrument;
	int command;
	int parameters;
} TrackerNote;

typedef struct TrackerInstrument {
	char name[22];
	int volume;
	int finetune;
} TrackerInstrument;

typedef struct TrackerPattern {
	int rows;
	int channels;
	TrackerNote * notedata;	// allocated dynamically
} TrackerPattern;

typedef struct TrackerSong {
	char name[22];
	int length;
	int orderlist[256];
	int patterns;
	TrackerPattern * patterndata;
} TrackerSong;

int transferNoteData(TrackerNote& tnote, Note& nnote);
int loadAmigamod(TrackerSong& song, char * input_path);
int freeSong(TrackerSong& song);
void printSong(TrackerSong& song );
void printTrackerNote(TrackerNote& note, int detail);


#endif
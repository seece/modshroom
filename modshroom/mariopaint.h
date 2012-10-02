#ifndef MARIOPAINT_H
#define MARIOPAINT_H

#include "modloader.h"

#define ZSH_SONG_DATA_OFFSET 0x15F8
#define ZSH_TEMPO_OFFSET 0x1830
#define ZSH_DATA_LENGTH 576

typedef struct Mariosong {
	char header[4];
	short version;
	char compressed ;
	char songname[32];
	char author[32];
	char preferSHI[32];	// not supported
	char songdata[ZSH_DATA_LENGTH];
	char tempo;	// values range from 0x00 to 0x9F
	//char loop;	// 0 or 1
	//short length;	// STILL A MYSTERY :O
} Mariosong;

void prepareMariosong(Mariosong& msong);
int loadSHO(Mariosong& msong, char* input_path);
int saveSHO(Mariosong& msong, char* output_path);
void printMariosong(Mariosong& msong);
int convertToMarioSongData(TrackerSong& song, Mariosong& sho);

#endif
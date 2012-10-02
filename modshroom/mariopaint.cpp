#include <stdio.h>
#include <string.h>
#include "modloader.h"
#include "mariopaint.h"

void prepareMariosong(Mariosong& msong) {
	msong.header[0] = 's';
	msong.header[1] = 'h';
	msong.header[2] = 'r';
	msong.header[3] = 'o';

	// zero the memory first, we don't want random data to be saved to a file
	memset(&msong.author, 0, 32);
	memset(&msong.songname, 0, 32);
	memset(&msong.preferSHI, 0, 32);

	sprintf(msong.author, "unknown");
	sprintf(msong.songname, "untitled");
	sprintf(msong.preferSHI, "\0");

	msong.tempo = 150;
	msong.version = (short)2;
	msong.compressed = 0;
}

int loadSHO(Mariosong& msong, char* input_path) {
	FILE *fp;
	fp = fopen(input_path, "rb");

	if (fp == NULL) {
		printf("Loading error!\n");
		//fclose(fp);
		return 1;
	}

	fread(msong.header, sizeof(char), 4, fp);		// should be shro
	fread(&msong.version, sizeof(short), 1, fp);	// should be 2
	fread(&msong.compressed, sizeof(char), 1, fp);
	fread(msong.songname, sizeof(char), 32, fp);
	fread(msong.author, sizeof(char), 32, fp);
	fread(msong.preferSHI, sizeof(char), 32, fp);

	fread(msong.songdata, sizeof(char), ZSH_DATA_LENGTH, fp);
	fread(&msong.tempo, sizeof(char), 1, fp);

	fclose(fp);

	return 0;
}

int saveSHO(Mariosong& msong, char* output_path) {
	FILE *fp;
	fp = fopen(output_path, "wb");

	if (fp == NULL) {
		printf("Save error!\n");
		//fclose(fp);
		return 1;
	}

	fwrite(msong.header, sizeof(char), 4, fp);
	fwrite(&msong.version, sizeof(short), 1, fp);
	fwrite(&msong.compressed, sizeof(char), 1, fp);
	fwrite(msong.songname, sizeof(char), 32, fp);
	fwrite(msong.author, sizeof(char), 32, fp);
	fwrite(msong.preferSHI, sizeof(char), 32, fp);

	fwrite(msong.songdata, sizeof(char), ZSH_DATA_LENGTH, fp);
	fwrite(&msong.tempo, sizeof(char), 1, fp);	

	fclose(fp);

	return 0;
}

void printMariosong(Mariosong& msong) {
	printf("Header:\t%c%c%c%c\n", msong.header[0],msong.header[1],msong.header[2],msong.header[3]);
	printf("Version:\t%d\n", msong.version);
	printf("Compressed:\t%d\n", msong.compressed);
	printf("Name:\t%s\n", msong.songname);
	printf("Author:\t%s\n", msong.author);
	printf("SHI:\t%s\n", msong.preferSHI);
	printf("Tempo:\t%d\n", msong.tempo);
}

// only fills in the relevant data, so you can fill in the fields to the sho beforehand
int convertToMarioSongData(TrackerSong& song, Mariosong& sho) {
	// only takes the notes from the first three channels
	// effects are discarded
	// notes with invalid pitches (must be between B3 and B4) are discarded

	// we could use the protracker looping command as a marker to end the song though
	// but that's just an idea

	// we create an array we use to map semitone offset from B3 to mariopaint notes
	// there might be a more clever way but this works OK!
	// 0xFF == invalid note
	//                 B     C     C#     D    D#     E      F     F#   G     G#     A    A#
	char noteLUT[] = {0x01, 0x02, 0xFF, 0x03, 0x0FF, 0x04, 0x05, 0xFF, 0x06, 0xFF, 0x07, 0xFF,
	//				    B     C    C#    D     D#    E     F     F#    G
					  0x08, 0x09, 0xFF, 0x0A, 0xFF, 0x0B, 0x0C, 0xFF, 0x0D};
	
	
	//bool done = false;
	bool print_errors = true;
	bool error = false;
	int sho_offset = 0;
	char notebytes[2];
	notebytes[0] = 0x00;
	notebytes[1] = 0x00;

	// fill the song data initially with 0xFFDF
	for (int i=0;i<ZSH_DATA_LENGTH-1;i=i+2) {
		sho.songdata[i] = 0xFF;
		sho.songdata[i+1] = 0xDF;
	}

	for (int i=0;i<song.length;i++) {
		int pattern_id = song.orderlist[i];
		TrackerPattern pattern = song.patterndata[pattern_id];

		for (int r=0;r<pattern.rows;r++) {
			for (int u=0;u<3;u++) {

				char finalpitch = 0xFF;	
				error = false;
				int offset = r * pattern.channels + u;
				TrackerNote note = pattern.notedata[offset];

				if (note.pitch == EMPTY_NOTE_VALUE) {
					// the note is empty
					notebytes[0] = 0xFF;
					notebytes[1] = 0xDF;
				} else {
					// lets start from A3 (440hz)

					int newpitch = note.pitch + 1;	 // values must be >= 0 (from B3)
					char instrument = note.instrument - 1;

					//printf("%d: 0x%02x\n", r, (unsigned int)instrument);

					//printf("%d: 0x%02x\n", r, (unsigned int)newpitch);
					
					if (newpitch < 0 || newpitch > 20) {
						error = true;
					} else {
						finalpitch = noteLUT[newpitch];
					}

					// special cases for the conga & hihat instrument
					// instrument 13 is a conga between B3 and G4 and an open hihat otherwise
					// conga
					if (instrument == 0x0C && !error) {
						// restrict the instrument pitch so that the sample stays correct
						if (finalpitch > 0x06) {
							finalpitch = 0x06;
						}
					}

					// hihat
					if (instrument == 0x0F && !error) {
						instrument = 0x0C;
						if (finalpitch < 0x07) {
							finalpitch = 0x07;
						}
					}

					if (instrument < 0x00 || instrument > 0x0E) {
						instrument = 0x00;

						if (print_errors) {
							printf("Warning: invalid instrument value at pattern %d, row %d, channel %d!\n", pattern_id, r, u);
						}
					}

					if (finalpitch == 0xFF || finalpitch < 0x01) {
						// invalid pitch, write empty row
						notebytes[0] = 0xFF;
						notebytes[1] = 0xDF;

						if (print_errors) {
							printf("Warning: invalid note ignored at pattern %d, row %d, channel %d!\n", pattern_id, r, u);
						}
					} else {
						// note value is OK!
						notebytes[0] = finalpitch;
						notebytes[1] = instrument;
					}			
				}
	

				sho.songdata[sho_offset] = notebytes[0];
				sho.songdata[sho_offset + 1] = notebytes[1];

				sho_offset = sho_offset + 2;	// two bytes per note

				if (sho_offset >= ZSH_DATA_LENGTH) {
					return 0;	// oh yeah we did it, and we ran out of song data space
				}
			}
		}
	}

	return 0;
}
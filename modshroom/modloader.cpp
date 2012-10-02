
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "util.h"
#include "modloader.h"
#include "amigamod.h"

int loadAmigamod(TrackerSong& song, char * input_path) {
	int sz;
	int flag_verbose = 0;

	FILE *fp;
    fp = fopen(input_path, "rb");

    if (fp == NULL) {
			printf("File %s not found!", input_path);
			//fclose(fp);
			return 5;	// I/O error
    }

	sz = getFilesize(fp);	// let's hope the filesize fits in a 32bit int

	//uint8_t moduledata[sz];	// the whole module
	uint8_t * moduledata = (uint8_t *) malloc(sz * sizeof(uint8_t));

	fread(moduledata, sizeof(char), sz, fp);
    fclose(fp);

    // Initialize the needed variables

	uint8_t modulename[20];	// well, uh
	uint8_t initials[5];		// the magic code
	int song_data_offset;
	uint8_t song_length;		// song length in patterns (the length of the order list)
	int pattern_amount;
	int max_orderlist_entries = 128;
	int sample_amount = MAX_SAMPLES;

	// data structures needed for export
	Songinfo ssong;
	Instrumentinfo sinstruments;

	// init the instrument variables
	for (int i=0;i<31;i++) {
		setDefaultInstrumentValues(&(sinstruments.instruments[i]));
	}

	loadModuleName(moduledata, ssong.name);
	printf("Song name: %s\n", ssong.name);

	int channels = getChannelAmount(moduledata);
	if (channels == -1) {
		printf("Error: Invalid module file!\n");
		return -1;
	}

	// load the sample information (not actual PCM audio)
	//sinstruments.instrument_amount = sample_amount; // TODO add check to see how many instruments are actually used
	sinstruments.instrument_amount = channels;	
	//Sample sample_info[sample_amount];
	Sample * sample_info = (Sample *) malloc(sample_amount * sizeof(Sample));
	loadSampleInfo(moduledata, sample_info, sample_amount);
	loadInstruments(sample_info, &sinstruments);

	if (flag_verbose) {
		printSamples(sample_info, sample_amount);
	}

	song_data_offset = 20 + sizeof(Sample) * sample_amount;

	memcpy(&song_length, &moduledata[song_data_offset + 0], 1);

	//dprint(song_length);

	uint8_t song_order[256];
	loadOrderData(moduledata, song_order, song_data_offset, max_orderlist_entries);
	pattern_amount = getHighestOrder(moduledata, song_order, max_orderlist_entries);
	pattern_amount++;	// atomim's rule (515-3)

	// are we even using these ssong structs anywhere?
	ssong.rows = 64;
	ssong.channels = channels;
	ssong.length = song_length;
	ssong.pattern_amount = pattern_amount;
	ssong.bpm = 120;	// read this from the module somehow?
	memcpy(ssong.orderlist, song_order, max_orderlist_entries);

	if (flag_verbose) {
		printOrderlist(song_order, song_length);
	}


	// now load the notes
	
	int pattern_data_offset = song_data_offset + 2 + 128 + 4;	// this should be 1084d for a MOD

	//int note_data;
	//uint8_t note_data[channels];	// each note is 32-bits long
	uint8_t * note_data = (uint8_t *) malloc(channels * sizeof(uint8_t));	// each note is 4*8 = 32 bits long

	// allocate memory for the notes to be exported
	//Note synthnotes[channels*MOD_ROWS*pattern_amount];
//	Note * synthnotes = (Note *) malloc(channels*MOD_ROWS*pattern_amount*sizeof(Note));
	// synthnotes stores all the notes as a big list, not as individual patterns
	Note * synthnotes = (Note *) malloc(channels*MOD_ROWS*song_length*sizeof(Note));
	// initialize the Note array
	uint32_t note_array_length = sizeof(Note) * channels*MOD_ROWS*pattern_amount;
	memset(synthnotes, 9, note_array_length);

	printf("Patterns : %d\n", pattern_amount);

	// load pattern data
	/*
	for (int i=0;i<pattern_amount;i++) {
		int offset = pattern_data_offset+ i*MOD_CHANNEL_SIZE*channels;
		for (int r=0;r<64;r++) {	// loop through rows

			for (int u=0;u<channels;u++) {

				// void * memcpy ( void * destination, const void * source, size_t num );
				memcpy(note_data, &moduledata[offset + u*MOD_NOTE_SIZE + r*MOD_NOTE_SIZE*channels], 4);

				Note current_note;
				parseNote(note_data, &current_note);

				uint32_t note_array_offset = (i*MOD_ROWS*channels) + r*channels + u;

				synthnotes[note_array_offset] = current_note;

				if (PATTERN_DEBUG) {
					if (i<=selected_pattern) {
						uint8_t nname[4];
						getNoteName(current_note.pitch, nname);
						printf("%s", nname);
						int blaablaa = 0;

						printf("  ");
					}
				}

			}

			if (i<=selected_pattern && PATTERN_DEBUG) {
				printf("\n");
			}
		}

		if (PATTERN_DEBUG) {
			printf("\n");
		}

	}
	*/

	//
	// transfer the loaded data to TrackerSong struct
	//

	// NOTE:
	// a the moment, the instrument/sample data is completely ignored!

	// show the content of this pattern (if debug is enabled)
	// actually order list position
	int selected_pattern = 5;

	//strcpy(song.name, (char *) modulename); 
	loadModuleName(moduledata, (uint8_t *) song.name);
	song.length = song_length;
	song.patterns = pattern_amount;

	//printf("song name: %s\n", song.name);

	// copy the orderlist
	for (int i=0;i<song_length;i++) {
		song.orderlist[i] = song_order[i];
	}

	// the TrackerPatterns are not big since they just store a pointer to the note data
	int song_data_size = song.length * sizeof(TrackerPattern);
	song.patterndata = (TrackerPattern *) malloc(song_data_size);

	// copy the actual note data

	for (int i=0;i<song_length;i++) {
		int pattern_id = song_order[i];	// peek the correct pattern from the order list
		int offset = pattern_data_offset+ pattern_id*MOD_CHANNEL_SIZE*channels;

		// init the generic tracker format storage
		song.patterndata[i].channels = MOD_CHANNELS;
		song.patterndata[i].rows = MOD_ROWS;
		int pattern_note_amount = song.patterndata[i].channels * song.patterndata[i].rows;
		song.patterndata[i].notedata = (TrackerNote *) malloc(pattern_note_amount * sizeof(TrackerNote));

		//printf("pattern: %d, %d\n", i, pattern_id);
		// loop through rows
		for (int r=0;r<64;r++) {	

			for (int u=0;u<channels;u++) {
				// void * memcpy ( void * destination, const void * source, size_t num );
				memcpy(note_data, &moduledata[offset + u*MOD_NOTE_SIZE + r*MOD_NOTE_SIZE*channels], 4);	// each note is 4 bytes

				Note current_note;
				parseNote(note_data, &current_note);	// extract values from the nasty amiga format
				//printNote(&current_note);

				//uint32_t note_array_offset = (i*MOD_ROWS*channels) + r*channels + u;
				//synthnotes[note_array_offset] = current_note;	// a relic, not used

				TrackerNote new_note;
				// just convert the notedata to a new format, sorry about this mess
				transferNoteData(new_note, current_note);

				int notedata_offset = r*channels + u; // offset INSIDE the pattern
				song.patterndata[i].notedata[notedata_offset] = new_note;

				if (PATTERN_DEBUG) {
					if (i<=selected_pattern) {
						uint8_t nname[4];
						getNoteName(current_note.pitch, nname);
						printf("%s", nname);
						//printf("%04d",current_note.pitch);
						int blaablaa = 0;

						printf("  ");
					}
				}

			}

			if (i<=selected_pattern && PATTERN_DEBUG) {
				printf("\n");
			}
		}

		if (PATTERN_DEBUG) {
			printf("\n");
		}

	}


	// loop through the orderlist
	/*
	for (int i=0;i<song_length;i++) {
		int current_pattern_id = song.orderlist[i];

		song.patterndata[i].channels = MOD_CHANNELS;
		song.patterndata[i].rows = MOD_ROWS;
		int total_notes = song.patterndata[i].channels * song.patterndata[i].rows;
		song.patterndata[i].notedata = (TrackerNote *) malloc(total_notes * sizeof(TrackerNote));

		int channels = song.patterndata[i].channels;
		int rows = song.patterndata[i].rows;

		for (int u=0;u<channels;u++) {
			for (int r=0;r<rows;r++) {
				int note_array_offset = (i*rows*channels) + r*channels + u;
				Note current_note = synthnotes[note_array_offset];
				//printf( "%d\n", current_note.pitch );
				//printNote(&current_note);
			}
		}
		
	}
	*/

	free(moduledata);
	free(sample_info);
	free(note_data);
	free(synthnotes);

	return 0;
}

int freeSong(TrackerSong& song) {

	for (int i=0;i<song.length;i++) {
		free(song.patterndata[i].notedata);
	}

	free(song.patterndata);
	return 0;
}

int transferNoteData(TrackerNote& tnote, Note& nnote) {
	tnote.pitch = nnote.pitch;
	tnote.instrument = nnote.instrument;
	tnote.command = nnote.command;
	tnote.parameters = nnote.parameters;
	return 0;
}

void printSong(TrackerSong& song ) {
	printf("name: %s\n", song.name);
	printf("length: %d, patterns: %d\n", song.length, song.patterns);

	printf("Order list:\n");
	for (int i=0;i<song.length;i++) {
		printf("  %02d: %d\n", i, song.orderlist[i]);
	}

	for (int i=0;i<song.length;i++) {
		int pattern_id = song.orderlist[i];
		TrackerPattern pattern = song.patterndata[pattern_id];
		// DEBUG
		//if (i > 5) { return; };

		printf("order %d: pattern: %d\n", i, pattern_id);

		for (int r=0;r<pattern.rows;r++) {
			for (int u=0;u<pattern.channels;u++) {
				int offset = r * pattern.channels + u;
				TrackerNote note = pattern.notedata[offset];

				printTrackerNote(note, 0);
			}

			printf("\n");
		}
	}
}

void printTrackerNote(TrackerNote& note, int detail) {
	if (detail == 0) {
		char notename[4];
		getNoteName(note.pitch, (uint8_t *) notename);
		printf("%s ", notename);
	} else {

	}
}
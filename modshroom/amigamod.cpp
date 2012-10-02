#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "wav.h"
//#include "debug.h"
#include "util.h"
#include "amigamod.h"

uint8_t * loadModuleName(uint8_t *moduledata, uint8_t *output) {
	memcpy(output, moduledata, 20);

	return output;
}

uint8_t * loadModuleMagic(uint8_t *moduledata, uint8_t *output) {
	memcpy(output, &moduledata[1080], 4);
	output[4] = '\0';

	return output;
}

int getModuleType(uint8_t *magic) {
	if (strcmp("M.K.", (char*)magic) == 0) {
		return MODULE_TYPE_4CHAN;
	} else if (strcmp("8CHN", (char*)magic) == 0) {
		return MODULE_TYPE_8CHAN;
	} else if (strcmp("28CH", (char*)magic) == 0) {
		return MODULE_TYPE_28CHAN;
	}

	return MODULE_TYPE_UNKNOWN;
}

int loadSampleInfo(uint8_t *moduledata, Sample *sample_info, int sample_num) {
	int offset, i;
	short amigaword;

	for (i=0;i<sample_num;i++) {
		offset = 20 + i*sizeof(Sample) + 0;		// name
		memcpy(&sample_info[i].name, &moduledata[offset], 22);

		offset = 20 + i*sizeof(Sample) + 22;	// sample length
		memcpy(&amigaword, &moduledata[offset], 2);	// copy to temp variable
		sample_info[i].length = swapBytes(amigaword);

		offset = 20 + i*sizeof(Sample) + 24;	// finetune
		memcpy(&sample_info[i].finetune, &moduledata[offset], 1);

		offset = 20 + i*sizeof(Sample) + 25;	// volume
		memcpy(&sample_info[i].volume, &moduledata[offset], 1);

		offset = 20 + i*sizeof(Sample) + 26;	// repeat
		memcpy(&amigaword, &moduledata[offset], 2);
		sample_info[i].repeat = swapBytes(amigaword);

		offset = 20 + i*sizeof(Sample) + 28;	// repeat length
		memcpy(&amigaword, &moduledata[offset], 2);	// copy to temp variable
		sample_info[i].repeat_length = swapBytes(amigaword);

	}

	return 0;
}

void loadOrderData(uint8_t *moduledata, uint8_t *song_order, int song_data_offset, int song_order_num) {
	int i;

	for (i=0;i<song_order_num;i++) {
		song_order[i] = moduledata[song_data_offset + 2 + i];
	}
}

int getHighestOrder(uint8_t *moduledata, uint8_t *song_order, int song_order_num) {
	int pattern_num = 0;
	int i;

	for (i=0;i<song_order_num;i++) {

		if (song_order[i] > pattern_num)	// find the highest pattern id in the list
			pattern_num = (int)song_order[i];
	}

	return pattern_num;
}

int dumpProtrackerSamples(uint8_t *moduledata, Sample *sample_info, int sample_data_offset) {
	int i, u, offset;
	int sample_num = MAX_SAMPLES;	// always 31 for proper amiga mods! (unless crappy amiga module)
	//dump all the samples
	for (i=0;i<sample_num;i++) {
		if (sample_info[i].length != 0)  {
			uint8_t sample_name[MOD_SAMPLE_NAME_MAX_LENGTH];
			sprintf((char*) sample_name, "output/sample%.2d.wav", i);
			offset = 0;
			for (u=0;u<i;u++) {
				offset += sample_info[u].length * 2;
			}

			// C-3 is 16574Hz for PAL machines
			writeAmigaWav(&moduledata[sample_data_offset + offset], sample_info[i].length*2, sample_name , 16574);
		}
	}

	return 0;
}

// take four bytes of raw notedata and output a clean struct
int parseNote(uint8_t *notedata, Note *output) {

	int16_t note = 0;
	uint8_t instrument = 0;
	uint8_t command = 0;
	uint8_t parameters = 0;

	instrument = (notedata[0] & 0xF0) | ((notedata[2] & 0xF0) >> 4);
	command = (notedata[2] & 0x0F);
	parameters = notedata[3];

	int16_t n = (notedata[1] & 0x00FF) | (notedata[0] << 8);
	n = n & 0x0FFF;	// mask out the first nibble, this is a 12-bit value
	note = periodToSemitone(n);

	output->command = command;
	output->instrument = instrument;
	output->pitch = note;
	output->parameters = parameters;

	return 0;
}

void printNote(Note *note) {
	uint8_t notename[4];
	uint8_t instrumenth[3];	// some hex values
	uint8_t commandh[3];
	uint8_t parametersh[3];
	getNoteName(note->pitch, notename);
	charToNiceHex(note->instrument, instrumenth);
	charToNiceHex(note->command, commandh);
	charToNiceHex(note->parameters, parametersh);

	printf("%s %s %s %s", notename, instrumenth, commandh, parametersh);
	//printf("%.2d %s %s %s", note->pitch, instrumenth, commandh, parametersh);
}

int periodToSemitone(int period) {
	int i;
	// starting from C-1
	int table[] = {	856,808,762,720,678,640,604,570,538,508,480,453,
					428,404,381,360,339,320,302,285,269,254,240,226,
					214,202,190,180,170,160,151,143,135,127,120,113};

	for (i=0;i<3*12;i++) {
		if (table[i]==period) {
			int semitone_offset = i - 12;	// root note is C-4
			return semitone_offset;
		}
	}

	int ret = EMPTY_NOTE_VALUE;

	return ret;
}

//outputs 4 bytes
// takes semitones as parameters, NOT amiga periods
uint8_t *getNoteName(int semitones, uint8_t *output) {
	int note = semitones % 12;
	int octave = semitones / 12 + 4;
	uint8_t result[4];

	if (semitones < 0) {
		note = (12+(semitones % 12)) % 12;
		octave = (4*12+semitones)/12;
	}

	if (semitones != EMPTY_NOTE_VALUE) {

		result[1] = '-';
		result[2] = '0' + octave;

		switch (note) {
			case 0:
				result[0] = 'C'; break;
			case 1:
				result[0] = 'C';
				result[1] = '#'; break;
			case 2:
				result[0] = 'D'; break;
			case 3:
				result[0] = 'D';
				result[1] = '#'; break;
			case 4:
				result[0] = 'E'; break;
			case 5:
				result[0] = 'F'; break;
			case 6:
				result[0] = 'F';
				result[1] = '#'; break;
			case 7:
				result[0] = 'G'; break;
			case 8:
				result[0] = 'G';
				result[1] = '#'; break;
			case 9:
				result[0] = 'A'; break;
			case 10:
				result[0] = 'A';
				result[1] = '#'; break;
			case 11:
				result[0] = 'B'; break;
			default:
				result[0] = 'X';
				result[1] = 'X';
				break;
		}

	} else {
		// empty note
		result[0] = '-';
		result[1] = '-';
		result[2] = '-';
	}

	result[3]='\0';

	memcpy(output, result, 4*sizeof(uint8_t));

	return output;
}

uint32_t getChannelAmount(uint8_t *moduledata) {
	uint8_t initials[5];		// the magic code
	uint32_t channels;

	loadModuleMagic(moduledata, initials);

	if (getModuleType(initials) == MODULE_TYPE_4CHAN) {
		printf("Detected a 4 channel amigamod.\n");
		channels = 4;
	} else if (getModuleType(initials) == MODULE_TYPE_8CHAN) {
		printf("Detected a 8 channel amigamod.\n");
		channels = 8;
	} else {
		printf("Unknown module magic! Aborting...\n");
		channels = -1;
	}

	return channels;
}

void printOrderlist(uint8_t *song_order, uint8_t song_length) {
	uint32_t i;

	puts("Song order list:");
	for (i=0;i<song_length;i++) {
		printf(" %d", song_order[i]);
		if (i < song_length-1) {
				printf(",");
		}
	}
	printf("\n");
}

void printSamples(Sample *sample_info, uint8_t sample_amount) {
	uint32_t i;

	printf("Samples:\n");
	for (i=0;i<sample_amount;i++) {
		printf("%d.\t%s\n", i, sample_info[i].name);
		//dprints("%d", sample_info[i].length);
	}
}

//
// intermediate form functions
//

void setDefaultInstrumentValues(Instrument *ins) {
	strcpy((char *) ins->name, "instrument");
	ins->envelope = 0;
	ins->finetune = 0;
	ins->length = 0;
	ins->octave = 0;
	ins->pan = 0;
	ins->repeat = 0;
	ins->repeat_length = 0;
	ins->volume = 64;	// 040
}


uint32_t loadInstruments(Sample *sample_info, Instrumentinfo *sinstruments) {
	uint32_t i;

	for (i=0;i<sinstruments->instrument_amount;i++) {
		strcpy((char*) sinstruments->instruments[i].name, (char*) sample_info[i].name);
		sinstruments->instruments[i].volume = sample_info[i].volume;
		sinstruments->instruments[i].finetune = sample_info[i].finetune;
		sinstruments->instruments[i].length = sample_info[i].length;
		sinstruments->instruments[i].repeat = sample_info[i].repeat;
		sinstruments->instruments[i].repeat_length = sample_info[i].repeat_length;
	}

	return 0;
}

void printPattern(Note *synthnotes, Songinfo *ssong, uint32_t pattern) {
	uint32_t p = pattern;
	uint32_t channels = ssong->channels;
	uint32_t r, c;

	for (r=0;r<MOD_ROWS;r++) {	// loop through rows
		for (c=0;c<channels;c++) {
			Note current_note = synthnotes[(p*MOD_ROWS*channels) + r*channels + c];
			uint8_t nname[4];
			getNoteName(current_note.pitch, nname);

			printf("%s", nname);

			printf("  ");
		}

		printf("\n");

	}

	printf("\n");

}
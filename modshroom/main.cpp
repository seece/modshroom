
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mariopaint.h"
#include "amigamod.h"
#include "modloader.h"

void printUsage();
void waitKey();

int main(int argc, const char* argv[] ) {

	printf("modshroom v0.55\n");
	printf("all n00bs\n\n");

	char input_path[1024];	// a buffer overflow just waiting to happen...
	char output_path[1024];
	int tempo = 150;	// the default tempo
	char author[512];
	author[0]='\0';

	if (argc < 2 || argc > 5) {
		printUsage();
		waitKey();
		return 0;
	}

	strcpy(input_path, argv[1]);

	// we are given also the output path
	if (argc > 2) {
		strcpy(output_path, argv[2]);
	} else {
		char extension[] = ".sho";
		strcpy(output_path, argv[1]);
		strcat(output_path, extension);
	}

	if (argc > 3) {
		tempo = atoi(argv[3]);
		printf("Tempo: %d\n", tempo);
	} else {
		printf("Using the default tempo of %d\n", tempo);
	}

	if (argc > 4) {
		strcpy(author, argv[4]);
		author[31] = '\0';
	}

	//printf("argument : %s\n", argv[1]);

	TrackerSong song;
	if (loadAmigamod(song, input_path) == 0) {
		printf("Author: %s\n", author);
		printf("Amigamod %s loaded successfully.\n", input_path);
		printf("\nConverting...\n");

		Mariosong newSHO;
		prepareMariosong(newSHO);
		int conversionResult = convertToMarioSongData(song, newSHO);

		if (conversionResult == 0) {
			strcpy(newSHO.songname, song.name);
			newSHO.tempo = (char)tempo;	
			memcpy(newSHO.author, author, 32);
			saveSHO(newSHO, output_path);

			//printf("Author: %s\n", newSHO.author);

			printf("SHO saved successfully to %s\n", output_path);
		} else {
			printf("Conversion error!\n");
		}
	} else {
		printf("Error: Couldn't load amigamod %s!\n", input_path);
	}

	waitKey();
	return 0;
}

void printUsage() {
	printf("Usage: modshroom.exe input.mod [output.sho] [tempo] [author]\n");
}

void waitKey() {
	printf("\nPress enter to quit.\n");
	char a = getchar();
}
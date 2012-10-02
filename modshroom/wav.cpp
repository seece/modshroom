#include <stdio.h>
#include <stdint.h>

// uses only 8-bit samples, mono
int writeAmigaWav(uint8_t *source, int length, uint8_t *filename, int sample_rate)
{
// specs: https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
	int i;
	int sub_chunk_size = 16;	// 16 for pcm

	short audio_format = 1;
	short num_channels = 1;
	short bits_per_sample = 8;
	//int sample_rate	// already defined
	int byte_rate = sample_rate*num_channels*bits_per_sample/8;
	short block_align = num_channels*bits_per_sample/8;
	int chunk_size = 4 + (8 + 16) + (8 + length*num_channels*(bits_per_sample/8));

	char riff_string[] = {'R', 'I', 'F', 'F'};
	char wave_string[] = {'W', 'A', 'V', 'E'};
	char data_string[] = {'d', 'a', 't', 'a'};
	char chunk_id[]		={'f', 'm', 't', ' '};

	int data_chunk_size = length;

	FILE *sampleFile;
	sampleFile = fopen((char *)filename, "wb");

	int first = fwrite(riff_string, 1, 4, sampleFile);

	/*
	if (WAV_DEBUG) {
		 dprint(chunk_size);
		 dprint(first);
	}
	*/

	fwrite(&chunk_size, 4, 1, sampleFile);
	fwrite(wave_string, 1, 4, sampleFile);

	fwrite(chunk_id, 1, 4, sampleFile);
	fwrite(&sub_chunk_size, 4, 	1, 	sampleFile);
	fwrite(&audio_format, 	2, 	1, 	sampleFile);
	fwrite(&num_channels, 	2, 	1, 	sampleFile);
	fwrite(&sample_rate, 	4, 	1, 	sampleFile);
	fwrite(&byte_rate, 		4, 	1, 	sampleFile);
	fwrite(&block_align, 	2, 	1, 	sampleFile);
	fwrite(&bits_per_sample,2, 	1, 	sampleFile);
	fwrite(data_string, 	1, 	4, 	sampleFile);

	fwrite(&data_chunk_size, 4, 1, 	sampleFile);
	for (i=0;i<length;i++) {
		uint8_t a = 0;
		a = source[i] + 128;
		fwrite(&a, 1, 1, sampleFile);
	}

	fclose(sampleFile);

	return 0;
}

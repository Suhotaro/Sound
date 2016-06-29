#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>

typedef struct
{
    unsigned char chunckId[4];
    uint32_t chunckSize;
    unsigned char format[4];

    unsigned char subchunk1Id[4];
    uint32_t subchunk1Size;

    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    unsigned char subchunk2Id[4];
    uint32_t subchunk2Size;
} WavHeader;

int main (int argc, char *argv[])
{
    WavHeader wav;
    char chunckId[5];
    char format[5];
    char subchunk1Id[5];
    char subchunk2Id[5];
    char *file;
    int fd;
    int size;

    if (argc == 1)
        file = "../wav_files/test.wav";
    else
        file = argv[1];

    printf("\n");
    printf("open file: %s\n\n", file);

    memset(chunckId, 0, sizeof(chunckId));
    memset(format, 0, sizeof(format));
    memset(subchunk1Id, 0, sizeof(subchunk1Id));
    memset(subchunk2Id, 0, sizeof(subchunk2Id));

    fd = open(file, O_RDWR);
    if (fd < 1)
    {
        printf("error: open\n");
        exit(1);
    }

    size = read(fd, &wav, sizeof(wav));
    if (size != sizeof(wav))
    {
        printf("error: read\n");
        exit(1);
    }

    memcpy(chunckId, wav.chunckId, sizeof(wav.chunckId));
    memcpy(format, wav.format, sizeof(wav.format));
    memcpy(subchunk1Id, wav.subchunk1Id, sizeof(wav.subchunk1Id));
    memcpy(subchunk2Id, wav.subchunk2Id, sizeof(wav.subchunk2Id));

    printf("file \"%s\" info:\n"
           "  chunckId: %s      \n"
    	   "  chunckSize: %d    \n"
     	   "  format: %s        \n"
     	   "  subchunk1Id: %s   \n"
     	   "  subchunk1Size: %d \n"
     	   "  audioFormat: %d   \n"
     	   "  numChannels: %d   \n"
     	   "  sampleRate: %d    \n"
     	   "  byteRate: %d      \n"
    	   "  blockAlign: %d    \n"
    	   "  bitsPerSample: %d \n"
      	   "  subchunk2Id: %s   \n"
      	   "  subchunk2Size: %d \n\n",
    	   file,
		   chunckId, wav.chunckSize,
		   format, subchunk1Id,
		   wav.subchunk1Size, wav.audioFormat,
		   wav.numChannels, wav.sampleRate,
		   wav.byteRate, wav.blockAlign,
		   wav.bitsPerSample, subchunk2Id,
		   wav.subchunk2Size);
    
    return 0;
}

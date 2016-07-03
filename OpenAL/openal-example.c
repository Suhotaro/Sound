/*
 * OpenAL example
 *
 * Copyright(C) Florian Fainelli <f.fainelli@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#define BACKEND "alut"
#define DX 10


#define TEST_ERROR(_msg)		\
	error = alGetError();		\
	if (error != AL_NO_ERROR) {	\
		fprintf(stderr, _msg "\n");	\
		return -1;		\
	}

int main(int argc, char **argv)
{
	ALboolean enumeration;
	const ALCchar *devices;
	const ALCchar *defaultDeviceName = argv[1];
	int ret;
	char *bufferData;
	ALCdevice *device;
	ALvoid *data;
	ALCcontext *context;
	ALsizei size, freq;
	ALenum format;
	ALuint buffer, source;
	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	ALboolean loop = AL_FALSE;
	ALCenum error;
	ALint source_state;

	fprintf(stdout, "Using " BACKEND " as audio backend\n");

	alutInit(NULL, NULL);
	TEST_ERROR("init Alut");


	/* listener */
	alListener3f(AL_POSITION, 0, 0, 0);
	TEST_ERROR("listener position");

    alListener3f(AL_VELOCITY, 0, 0, 0);
	TEST_ERROR("listener velocity");

	alListenerfv(AL_ORIENTATION, listenerOri);
	TEST_ERROR("listener orientation");

	alGenSources((ALuint)1, &source);
	TEST_ERROR("source generation");

	/* source */
	alSourcef(source, AL_PITCH, 1);
	TEST_ERROR("source pitch");

	alSourcef(source, AL_GAIN, 1);
	TEST_ERROR("source gain");

	alSource3f(source, AL_POSITION, -8, 0, 0);
	TEST_ERROR("source position");

	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	TEST_ERROR("source velocity");

	alSourcei(source, AL_LOOPING, AL_FALSE);
	TEST_ERROR("source looping");

	alGenBuffers(1, &buffer);
	TEST_ERROR("buffer generation");

	alutLoadWAVFile("../wav_files/bs1.wav", &format, &data, &size, &freq, &loop);
	TEST_ERROR("loading wav file");
	printf("buffer data:\n"
			"  format -> %d\n"
			"  size -> %d\n"
			"  freq -> %d\n",
			format, size, freq);

	alBufferData(buffer, format, data, size, freq);
	TEST_ERROR("buffer copy");

	alSourcei(source, AL_BUFFER, buffer);
	TEST_ERROR("buffer binding");

	alSourcePlay(source);
	TEST_ERROR("source playing");

	alGetSourcei(source, AL_SOURCE_STATE, &source_state);
	TEST_ERROR("source state get");

	int dx = -DX;
	int left = 0, right = 1;
	printf("dx=%d\n", dx);

	while(source_state == AL_PLAYING)
	{
		if (left == 1)
		{
			dx -= 1;
			printf("dx=%d\n", dx);

			alSource3f(source, AL_POSITION, -dx, 0, dx);

			if (dx == -DX)
			{
				right = 1;
				left = 0;
			}
		}
		else if (right == 1)
		{
			dx += 1;
			printf("dx=%d\n", dx);

			alSource3f(source, AL_POSITION, -dx, 0, dx);

			if (dx == DX)
			{
				right = 0;
				left = 1;
			}
		}

		alGetSourcei(source, AL_SOURCE_STATE, &source_state);
		sleep(1);
	}


	/* exit context */
	alDeleteSources(1, &source);
	alDeleteBuffers(1, &buffer);
	device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);

	return 0;
}

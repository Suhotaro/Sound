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

#define RETURNIFERROR(_msg)		\
	error = alGetError();		\
	if (error != AL_NO_ERROR) {	\
		fprintf(stderr, "ERROR: "_msg "\n");	\
		return;		\
	}

#define RETURNVALIFERROR(val, _msg)		\
	error = alGetError();		\
	if (error != AL_NO_ERROR) {	\
		fprintf(stderr, "ERROR: "_msg "\n");	\
		return val;		\
	}

typedef struct
{
	ALfloat orientation[6];

	struct
	{
		ALfloat x;
		ALfloat y;
		ALfloat z;
	} position, velocity;
} Listener, *ListenerPtr;


typedef struct
{
	ALuint id;
	ALuint source;

	ALint state;

	ALfloat pitch;
	ALfloat gain;
	ALboolean loop;

	struct
	{
		ALfloat x;
		ALfloat y;
		ALfloat z;
	} position, velocity;
} Source, *SourcePtr;

typedef struct
{
	ALuint buffer;
	ALuint id;
	ALbyte *file;

	ALvoid *data;
    ALenum format;
    ALsizei size, freq;
    ALboolean loop;
} Buffer, *BufferPtr;


ListenerPtr listener_create(ALfloat *orientation,
		                    int px, int py, int pz,
							int vx, int vy, int vz)
{
	ALCenum error;
	int i = 0;

	/* TODO: Add NULL checks */
	ListenerPtr l = calloc(1, sizeof(Listener));

	l->position.x = px;
	l->position.y = py;
	l->position.z = pz;

	l->velocity.x = vx;
	l->velocity.y = vy;
	l->velocity.z = vz;

	for (i = 0 ; i < 6; i++)
		l->orientation[i] = orientation[i];

	alListener3f(AL_POSITION, l->position.x,
			     l->position.y, l->position.z);
	RETURNIFERROR("listener position");

    alListener3f(AL_VELOCITY, l->velocity.x,
    		     l->velocity.y, l->velocity.z);
	RETURNIFERROR("listener velocity");

	alListenerfv(AL_ORIENTATION, l->orientation);
	RETURNIFERROR("listener orientation");

	return l;
}

void listener_delete(ListenerPtr l)
{
	free(l);
}

SourcePtr source_create(int id, ALfloat pitch, ALfloat gain,
		                int px, int py, int pz,
						int vx, int vy, int vz)
{
	ALCenum error;

	/* TODO: Add NULL checks */
	SourcePtr s = calloc(1, sizeof(Source));

	s->id = id;

	s->position.x = px;
	s->position.y = py;
	s->position.z = pz;

	s->velocity.x = vx;
	s->velocity.y = vy;
	s->velocity.z = vz;

	s->pitch = pitch;
	s->gain = gain;
	s->loop = 0;

	s->state = 0;

	alGenSources(s->id, &s->source);
	RETURNIFERROR("source generation");
	/* source */
	alSourcef(s->source, AL_PITCH, s->pitch);
	RETURNIFERROR("source pitch");

	alSourcef(s->source, AL_GAIN, s->gain);
	RETURNIFERROR("source gain");

	alSource3f(s->source, AL_POSITION,
			   s->position.x, s->position.y,
			   s->position.z);
	RETURNIFERROR("source position");

	alSource3f(s->source, AL_VELOCITY,
			   s->velocity.x, s->velocity.y,
			   s->velocity.z);
	RETURNIFERROR("source velocity");

	alSourcei(s->source, AL_LOOPING, s->loop);
	RETURNIFERROR("source looping");

	return s;
}

void source_delete(SourcePtr s)
{
	alDeleteSources(s->id, &s->source);
	free(s);
}

BufferPtr buffer_create(int id,  ALbyte *file)
{
	ALCenum error;

	/* TODO: Add NULL checks */
	BufferPtr b = calloc(1, sizeof(Buffer));

	b->id = id;
	b->file = file;

	alGenBuffers(b->id, &b->buffer);
	RETURNIFERROR("buffer generation");

	alutLoadWAVFile(file, &b->format, &b->data,
					&b->size, &b->freq, &b->loop);
	RETURNIFERROR("loading wav file");

	printf("buffer data:\n"
				"  format -> %d\n"
				"  size -> %d\n"
				"  freq -> %d\n",
				b->format, b->size, b->freq);

	alBufferData(b->buffer, b->format, b->data, b->size, b->freq);
	RETURNIFERROR("buffer copy");

	return b;
}

void buffer_delete(BufferPtr b)
{
	alDeleteBuffers(b->id, &b->buffer);
	free(b);
}

int main(int argc, char **argv)
{
	ALCenum error;

	alutInit(NULL, NULL);
	RETURNVALIFERROR(-1, "init Alut");

	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };

	ListenerPtr listener = listener_create(listenerOri, 0, 0, 0, 0, 0, 0);
	SourcePtr s = source_create(1, 1, 1, 0, 0, 0, 0, 0, 0);
	BufferPtr b = buffer_create(1, (ALbyte *)"../wav_files/bs1.wav");

	alSourcei(s->source, AL_BUFFER, b->buffer);
	RETURNVALIFERROR(-1, "buffer binding");

	alSourcePlay(s->source);
	RETURNVALIFERROR(-1, "source playing");

	alGetSourcei(s->source, AL_SOURCE_STATE, &s->state);
	RETURNVALIFERROR(-1, "source state get");

	while(s->state == AL_PLAYING)
		alGetSourcei(s->source, AL_SOURCE_STATE, &s->state);

	return 0;
}

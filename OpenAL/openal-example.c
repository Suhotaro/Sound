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
		fprintf(stderr, "ERROR: "_msg " %d\n", error);	\
		return;		\
	}

#define RETURNVALIFERROR(val, _msg)		\
	error = alGetError();		\
	if (error != AL_NO_ERROR) {	\
		fprintf(stderr, "ERROR: "_msg "\n");	\
		return val;		\
	}

#define IFERROR(_msg)		\
	error = alGetError();		\
	if (error != AL_NO_ERROR) {	\
		fprintf(stderr, "ERROR: "_msg "\n");	\
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


typedef enum
{
	SIMPLE = 0,
	MOVING_SRC,
	DOPPLER,
	CONE_SOUND,
	QUEUE,
} test_num;

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
				"  freq -> %d\n\n",
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

void test_simple(SourcePtr s, BufferPtr b)
{
	ALCenum error;

	alSourcei(s->source, AL_BUFFER, b->buffer);
	RETURNIFERROR("buffer binding");

	alSourcePlay(s->source);
	RETURNIFERROR("source playing");

	alGetSourcei(s->source, AL_SOURCE_STATE, &s->state);
	RETURNIFERROR("source state get");

	while(s->state == AL_PLAYING)
			alGetSourcei(s->source, AL_SOURCE_STATE, &s->state);
}

void test_moving_src(SourcePtr s, BufferPtr b)
{
	ALCenum error;
	int x = 10;
	int dx = -1;

	alSourcei(s->source, AL_BUFFER, b->buffer);
	RETURNIFERROR("buffer binding");

	alSourcePlay(s->source);
	RETURNIFERROR("source playing");

	alGetSourcei(s->source, AL_SOURCE_STATE, &s->state);
	RETURNIFERROR("source state get");

	alSource3f(s->source, AL_POSITION, x, 0, 0);

	while(s->state == AL_PLAYING)
	{
		if (x == 10)
			dx = -1;
		else if (x == -10)
			dx = 1;

		x += dx;
		printf("x: %d\n", x);

		alSource3f(s->source, AL_POSITION, x, 0, 0);
		alGetSourcei(s->source, AL_SOURCE_STATE, &s->state);

		usleep(500000);
	}
}

void test_doppler(SourcePtr s, BufferPtr b)
{
	ALCenum error;
	int x = 10;
	int dx = -1;

	alSourcei(s->source, AL_BUFFER, b->buffer);
	RETURNIFERROR("buffer binding");

	alSourcePlay(s->source);
	RETURNIFERROR("source playing");

	alGetSourcei(s->source, AL_SOURCE_STATE, &s->state);
	RETURNIFERROR("source state get");

	alSourcef(s->source, AL_GAIN, 0.5);
	alSource3f(s->source, AL_POSITION, x, 0, 0);
	alSource3f(s->source, AL_VELOCITY, -10, 0, 0);

	while(s->state == AL_PLAYING)
	{
		if (x == -10)
			x = 10;

		x += dx;

		printf("x: %d\n", x);

		alSource3f(s->source, AL_POSITION, x, 0, 0);
		alGetSourcei(s->source, AL_SOURCE_STATE, &s->state);

		usleep(500000);
	}
}

void test_cone_sound(SourcePtr s, BufferPtr b)
{
	ALCenum error;
	int x[16] = {2, 2, 2, 1, 0,-1,-2,-2,-2,-2,-2,-1, 0, 1, 2, 2 };
	int y[16] = {0,-1,-2,-2,-2,-2,-2,-1, 0, 1, 2, 2, 2, 2, 2, 1 };
	int clock_wize = 1;
	int i = 0;

	alSourcei(s->source, AL_BUFFER, b->buffer);
	alSourcePlay(s->source);
	alGetSourcei(s->source, AL_SOURCE_STATE, &s->state);


	/* TODO: check source axis and axis sound is round in*/

	alSourcef(s->source, AL_GAIN, 0.3);
	IFERROR("Gain");

	alSourcef(s->source, AL_CONE_OUTER_GAIN, 0.0);
	IFERROR("Outer Gain");

	alSource3f(s->source, AL_POSITION, 0, 5, 0);
	IFERROR("Position");

	/* Cone parameters */
	alSource3f(s->source, AL_DIRECTION, 0, -2, 0);
	IFERROR("Direction");

	alSourcef(s->source, AL_CONE_INNER_ANGLE, 30.0);
	IFERROR("Inner");

	alSourcef(s->source, AL_CONE_OUTER_ANGLE, 200.0);
	IFERROR("Outer");

	while(s->state == AL_PLAYING)
	{
		printf("i:%2d x:%2d y:%2d\n", i, x[i], y[i]);
		alSource3f(s->source, AL_DIRECTION, x[i], y[i], 0);
		IFERROR("Direction");

		i+=1;

		if (i == 16)
			i = 0;

		alGetSourcei(s->source, AL_SOURCE_STATE, &s->state);

		usleep(1000000);
	}
}

void test_queue(SourcePtr s)
{
	ALCenum error;
	ALuint buffs[2];
	ALuint queue_num;

	/* TODO: Why queue works only with same samples??? */
	BufferPtr b1 = buffer_create(1, (ALbyte *)"../wav_files/nightfall.wav");
	BufferPtr b2 = buffer_create(2, (ALbyte *)"../wav_files/nightfall.wav");
	BufferPtr b3 = buffer_create(3, (ALbyte *)"../wav_files/nightfall.wav");

	if (b1 == NULL || b2 == NULL || b3 == NULL)
	{
		printf("Fail create buffers\n");
		return;
	}

	buffs[0] = b1->buffer;
	buffs[1] = b2->buffer;
	buffs[2] = b3->buffer;

	alSourcei(s->source, AL_LOOPING, 0);
	RETURNIFERROR("source looping");

	alSourceQueueBuffers(s->source, 2, buffs);
	RETURNIFERROR("Queue buffers 1");

	alGetSourcei(s->source, AL_BUFFERS_QUEUED, &queue_num);
	printf("buffers: queued: %d\n", queue_num);

	alSourcePlay(s->source);
	RETURNIFERROR("source playing");

	alGetSourcei(s->source, AL_SOURCE_STATE, &s->state);

	/* Source is in Play state, queue another buffer */
	alSourceQueueBuffers(s->source, 1, &buffs[2]);
	RETURNIFERROR("Queue buffers 1");

	alGetSourcei(s->source, AL_BUFFERS_QUEUED, &queue_num);
	printf("buffers: queued: %d\n", queue_num);

	int played = 0;

	while(s->state == AL_PLAYING)
	{
		alGetSourcei(s->source, AL_SOURCE_STATE, &s->state);

		/* waite untill first sample is finished */
		usleep(9000000);


		alSourceUnqueueBuffers(s->source, 1, &buffs[played]);
		RETURNIFERROR("Unqueue 1");

		alGetSourcei(s->source, AL_BUFFERS_QUEUED, &queue_num);
		printf("buffers: queued: %d\n", queue_num);

		if (queue_num == 0)
			return;
	}
}


int main(int argc, char **argv)
{
	ALCenum error;
	int test = QUEUE;

	if (argc > 1)
		test = atoi( argv[1]);

	alutInit(NULL, NULL);
	RETURNVALIFERROR(-1, "init Alut");

	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };

	ListenerPtr listener = listener_create(listenerOri, 0, 0, 0, 0, 0, 0);
	SourcePtr s = source_create(1, 1, 1, 0, 0, 0, 0, 0, 0);

	/* Put here your files */
	BufferPtr b = buffer_create(1, (ALbyte *)"../wav_files/bs1.wav");

	/*
	 *  To hear sound with 3D effects samples MUST have only ONE chanell
	 */
	switch(test)
	{
		case SIMPLE:
			printf("Simple:\n");
			test_simple(s, b);

			break;

		case MOVING_SRC:
			printf("Moving source:\n");
			test_moving_src(s, b);

		case DOPPLER:
			printf("Doppler effect:\n");
			test_doppler(s, b);

			break;

		case CONE_SOUND:
			printf("Cone sound:\n");
			test_cone_sound(s, b);

			break;

		case QUEUE:
			printf("Queue:\n");
			test_queue(s);

			break;



		/* TODO: buffer queue */

		default:
			printf("bad input\n");
	}


	return 0;
}

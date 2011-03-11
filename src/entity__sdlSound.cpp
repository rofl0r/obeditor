#include <SDL.h>
#include <SDL_mixer.h>

#define MAX_SOUNDS 5

bool b_init_snd = false;
int channel;
Mix_Chunk *sounds[MAX_SOUNDS];

void initaudio()
{
	// If already initialised
	if( b_init_snd )
		return;

	if ( SDL_Init(SDL_INIT_AUDIO) != 0)
	{
		fprintf(stderr, "Unable to initialize SDL sound system: %s\n", SDL_GetError());
		b_init_snd = false;
		return;
	}

	int audio_rate = 22050;
	Uint16 audio_format = AUDIO_U8;
	int audio_channels = 2;
	int audio_buffers = 4096;

	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
		fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
		b_init_snd = false;
		return;
	}

	for( int i=0; i < MAX_SOUNDS; i++)
		sounds[i] = NULL;

	Mix_VolumeMusic( MIX_MAX_VOLUME / 2);

	b_init_snd = true;
}

void PlaySound(char *file)
{
	if( ! b_init_snd )
	{
		fprintf(stderr, "Sound system not initialised");
		return;
	}

	// Search for an avalaible channel
	int channel = -1;
	for( int i = 0; i < MAX_SOUNDS; i ++ )
	{
		if( Mix_Playing(i) == 0 )
		{
			if( sounds[i] != NULL )
			{
				Mix_FreeChunk(sounds[i]);
				sounds[i] = NULL;
			}
			channel = i;
			break;
		}
	}

	// No more channel avalaible
	if( channel == -1 )
	{
		fprintf(stderr, "No more channels for playing the sound");
		return;
	}

	sounds[channel] = Mix_LoadWAV(file);
	if( sounds[channel] == NULL) {
		fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError());
		return;
	}

	channel = Mix_PlayChannel(channel, sounds[channel], 0);
	if(channel == -1) {
		fprintf(stderr, "Unable to play WAV file: %s\n", Mix_GetError());
		return;
	}
}

void closeAudio()
{
	// If nothing to close
	if( ! b_init_snd )
		return;

	for( int i = 0; i < MAX_SOUNDS; i ++ )
	{
		if( sounds[i] != NULL )
		{
			Mix_FreeChunk(sounds[i]);
			sounds[i] = NULL;
		}
	}
	Mix_CloseAudio();
}

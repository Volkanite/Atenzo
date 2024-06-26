#include <alsa/asoundlib.h>
#include <stdio.h>

#include "sound.h"


char *SoundBuffer;
int buff_size;
unsigned int s_tmp;
snd_pcm_t *pcm_handle;
snd_pcm_uframes_t frames;

extern int Debug;
void LogToFile( char* Format, ... );


unsigned int GetNumSoundCards()
{
	unsigned int totalCards;
	int cardIndex;

	totalCards = 0;
	cardIndex = -1;

	for (;;)
	{
		if (snd_card_next(&cardIndex) < 0)
			break;

		if (cardIndex < 0)
			break;

		totalCards++;
	}

	snd_config_update_free_global();

	return totalCards;
}


void InitializeSoundDevice()
{
	snd_pcm_hw_params_t *params;
	int rate, channels;

	rate 	 = 44100;
	channels = 1;
	pcm_handle = 0;

	if(GetNumSoundCards() == 0)
		return;

	/* Open the PCM device in playback mode */
	if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
		return;

	if (Debug)
	{
		snd_pcm_info_t *pcmInfo;
		snd_ctl_t *audioHandle;
		snd_ctl_card_info_t *cardInfo;
		char str[64];
		int cardNum;
		int err;

		snd_pcm_info_alloca(&pcmInfo);
		memset(pcmInfo, 0, snd_pcm_info_sizeof());
		snd_pcm_info(pcm_handle, pcmInfo);

		audioHandle = 0;
		cardNum = snd_pcm_info_get_card(pcmInfo);

		sprintf(str, "hw:%i", cardNum);

		if ((err = snd_ctl_open(&audioHandle, str, 0)) == 0)
		{
			snd_ctl_card_info_alloca(&cardInfo);
			snd_ctl_card_info(audioHandle, cardInfo);

			LogToFile("Initialize sound device %s", snd_ctl_card_info_get_name(cardInfo));

			snd_ctl_close(audioHandle);
		}
		else
		{
			LogToFile("Can't open card %s: %s", snd_pcm_info_get_name(pcmInfo), snd_strerror(err));
		}
	}

	/* Allocate parameters object and fill it with default values*/
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(pcm_handle, params);

	/* Set parameters */
	snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(pcm_handle, params, channels);
	snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0);

	/* Write parameters */
	snd_pcm_hw_params(pcm_handle, params);

	/* Get info */
	snd_pcm_hw_params_get_channels(params, &s_tmp);
	snd_pcm_hw_params_get_rate(params, &s_tmp, 0);
	snd_pcm_hw_params_get_period_size(params, &frames, 0);
	snd_pcm_hw_params_get_period_time(params, &s_tmp, NULL);

	buff_size = frames * channels * 2 /* 2 -> sample size */;
}


void InitializeSound( char* FileName, SOUND_FILE* SoundFile )
{
	SoundFile->Handle = open(FileName, O_RDONLY);

	if (SoundFile->Handle == 0)
	{
		printf("Could not open source file\n");
	}

	//unsigned int pcm, dir;

	//InitializeSoundDevice();

	SoundFile->Size = lseek(SoundFile->Handle, 0, SEEK_END);
	SoundFile->Buffer = (char *) malloc(SoundFile->Size);

	lseek(SoundFile->Handle, 0, SEEK_SET);
	read(SoundFile->Handle, SoundFile->Buffer, SoundFile->Size);
	//close(SoundFile->Handle);
}


void TerminateSound()
{
	snd_pcm_drain(pcm_handle);
	snd_pcm_close(pcm_handle);
}


void CloseSoundFile( SOUND_FILE* SoundFile )
{
	free(SoundFile->Buffer);

	if (SoundFile->Handle)
	{
		close(SoundFile->Handle);
		SoundFile->Handle = 0;
	}
}


int PlaySoundFromFile( SOUND_FILE* SoundFile )
{
	int bytesRead = 0;
	int totalBytesRead = 0;
	unsigned int pcm;
	int bufferSize;

	lseek(SoundFile->Handle, 0, SEEK_SET);
	//snd_pcm_prepare(pcm_handle); //reset sound card buffer

	bufferSize = buff_size;

	while (totalBytesRead < SoundFile->Size)
	{
		if (totalBytesRead + bufferSize > SoundFile->Size)
			bufferSize = SoundFile->Size - totalBytesRead;

		bytesRead = read(SoundFile->Handle, SoundFile->Buffer, bufferSize);
		totalBytesRead += bytesRead;

		if (bytesRead == 0)
		{
			return 0;
		}

		if (snd_pcm_writei(pcm_handle, SoundFile->Buffer, frames) == -EPIPE)
		{
			snd_pcm_prepare(pcm_handle);
		}
	}

	return 0;
}


int PlaySoundFromBuffer( SOUND_FILE* SoundFile )
{
	int totalBytesRead = 0;
	int bytesToRead = 0;
	char* buff = 0;

	buff = SoundFile->Buffer;
	bytesToRead = buff_size;

	while (totalBytesRead < SoundFile->Size)
	{
		if (snd_pcm_writei(pcm_handle, buff, frames) == -EPIPE)
		{
			snd_pcm_prepare(pcm_handle);
		}

		if (totalBytesRead + bytesToRead > SoundFile->Size)
			bytesToRead = SoundFile->Size - totalBytesRead;

		buff += bytesToRead;
		totalBytesRead += bytesToRead;
	}

	return 0;
}


int PlaySound( SOUND_FILE* SoundFile )
{
	//Check if sound card was unplugged and re-initialize
	if (pcm_handle == 0 || snd_pcm_state(pcm_handle) == SND_PCM_STATE_DISCONNECTED)
	{
		InitializeSoundDevice();
	}

	snd_pcm_prepare(pcm_handle); //reset sound card buffer
	PlaySoundFromFile(SoundFile);

	return 0;
}

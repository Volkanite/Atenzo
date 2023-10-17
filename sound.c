#include <alsa/asoundlib.h>
#include <stdio.h>

#define PCM_DEVICE "default"

int SoundFile;
char *SoundBuffer;
int buff_size;
int Seconds;
unsigned int s_tmp;
snd_pcm_t *pcm_handle;
snd_pcm_uframes_t frames;


void InitializeSound()
{
	SoundFile = open("./beep.wav", O_RDONLY);

	if (SoundFile == 0)
	{
		printf("Could not open source file\n");
	}

	unsigned int pcm, dir;
	int rate, channels;
	snd_pcm_hw_params_t *params;

	rate 	 = 44100;
	channels = 1;
	Seconds  = 1;

	/* Open the PCM device in playback mode */
	if (pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE,
					SND_PCM_STREAM_PLAYBACK, 0) < 0)
		printf("ERROR: Can't open \"%s\" PCM device. %s\n",
					PCM_DEVICE, snd_strerror(pcm));

	/* Allocate parameters object and fill it with default values*/
	snd_pcm_hw_params_alloca(&params);

	snd_pcm_hw_params_any(pcm_handle, params);

	/* Set parameters */
	if (pcm = snd_pcm_hw_params_set_access(pcm_handle, params,
					SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
		printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));

	if (pcm = snd_pcm_hw_params_set_format(pcm_handle, params,
						SND_PCM_FORMAT_S16_LE) < 0)
		printf("ERROR: Can't set format. %s\n", snd_strerror(pcm));

	if (pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, channels) < 0)
		printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm));

	if (pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0) < 0)
		printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm));

	/* Write parameters */
	if (pcm = snd_pcm_hw_params(pcm_handle, params) < 0)
		printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));

	/* Resume information */
	//printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));

	//printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));

	snd_pcm_hw_params_get_channels(params, &s_tmp);
	//printf("channels: %i ", tmp);

	/*if (tmp == 1)
		printf("(mono)\n");
	else if (tmp == 2)
		printf("(stereo)\n");*/

	snd_pcm_hw_params_get_rate(params, &s_tmp, 0);
	//printf("rate: %d bps\n", tmp);

	//printf("seconds: %d\n", seconds);

	/* Allocate buffer to hold single period */
	snd_pcm_hw_params_get_period_size(params, &frames, 0);

	buff_size = frames * channels * 2 /* 2 -> sample size */;
	SoundBuffer = (char *) malloc(buff_size);

	snd_pcm_hw_params_get_period_time(params, &s_tmp, NULL);

	read(SoundFile, SoundBuffer, buff_size);
	close(SoundFile);
}


void TerminateSound()
{
	snd_pcm_drain(pcm_handle);
	snd_pcm_close(pcm_handle);

	free(SoundBuffer);
}


int PlaySound()
{
	snd_pcm_prepare(pcm_handle);
	snd_pcm_writei(pcm_handle, SoundBuffer, frames);

	return 0;
}

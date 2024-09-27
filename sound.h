typedef struct _SOUND_FILE
{
	int Handle;
	int Size;
	long long Interval;
	long long LastPlayed;
	char* Buffer;
}SOUND_FILE;

void InitializeSoundDevice();
unsigned int InitializeSoundFile(char* FileName, SOUND_FILE* SoundFile);
void TerminateSound();
int PlaySound(SOUND_FILE* SoundFile);

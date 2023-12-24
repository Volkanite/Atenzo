typedef struct _SOUND_FILE
{
	int Handle;
	int Size;
	char* Buffer;
}SOUND_FILE;

void InitializeSound(char* FileName, SOUND_FILE* SoundFile);
void TerminateSound();
int PlaySound(SOUND_FILE* SoundFile);

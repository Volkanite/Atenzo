#define TRUE    1
#define FALSE   0

extern int Debug;
void LogToFile( char* Format, ... );
long long current_timestamp();

long GetCommandResponse32(unsigned int Command);
long long GetCommandResponse64(long long Command);
void GetCommandResponse(char* Command, char* buff, int BufferLength);
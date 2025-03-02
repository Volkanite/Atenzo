extern int Debug;
void LogToFile( char* Format, ... );
long long current_timestamp();

long GetCommandResponse32(long Command);
long long GetCommandResponse64(long long Command);
void GetCommandResponse(char* Command, char* buff, int BufferLength);
typedef unsigned char byte;

long GetCommandResponse32(long Command);
long long GetCommandResponse64(long long Command);


int ReadDataByCommonIdentifier32( short RecordCommonIdentifier )
{
    int response, command;

    command = (0x22 << 16) + (RecordCommonIdentifier);
    response = GetCommandResponse32(command);

    if ((response & 0xff000000) >> 24 != 0x62)
        return 0;

    return (response & 0x000000ff);
}


int InputOutputControlByCommonIdentifier(
    short InputOutputCommonIdentifier,
    byte InputOutputControlParameter,
    byte ControlState
  )
{
    long long response, command;
    int id, param;

    id = InputOutputCommonIdentifier;
    param = InputOutputControlParameter;

    command = (0x2FLL << 32) + (id << 16) + (param << 8) + (ControlState);
    response = GetCommandResponse64(command);

    if ((response & 0xff00000000) >> 32 != 0x6F)
        return 0;

    return 1;
}

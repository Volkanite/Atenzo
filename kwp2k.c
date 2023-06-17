typedef unsigned char byte;
long long GetCommandResponse64(long long Command);


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

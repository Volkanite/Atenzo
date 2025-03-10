#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atenzo.h"


typedef unsigned char byte;


byte GetByte( unsigned long long Value, int BytePosition, int NumberOfBytes )
{
    unsigned long long mask;
    int nBits;

    nBits = (NumberOfBytes-BytePosition)*8;
    mask = 0xffULL << nBits;
    return (Value & mask) >> nBits;
}


ushort GetWord( unsigned long long Value, int BytePosition, int NumberOfBytes )
{
    unsigned long long mask;
    int nBits;

    nBits = (NumberOfBytes-(BytePosition+1))*8;
    mask = 0xffffULL << nBits;
    return (Value & mask) >> nBits;
}


int ECUReset( byte ResetMode )
{
    int request, response;

    request = (0x11 << 8) + ResetMode;
    response = GetCommandResponse32(request);

    if ((response >> 8) != 0x51)
        return 0;

    return 1;
}


int ClearDiagnosticInformation( ushort GroupOfDiagnosticInformation )
{
    int command, response;

    command = (0x14 << 16) + (GroupOfDiagnosticInformation);
    response = GetCommandResponse32(command);

    if ((response >> 16) != 0x54)
        return 0;

    return 1;
}


int ReadDiagnosticTroubleCodesByStatus(
    byte StatusOfDTCRequest,
    ushort GroupOfDTC,
    ushort* ArrayOfDTCs
  )
{
    int iCommand, statusOfDTCRequest, nBytes, nDTCs;
    unsigned long long ullBuffer;
    char szCommand[10], buffer[100];
    char *start;
    int i;

    statusOfDTCRequest = StatusOfDTCRequest;
    iCommand = (0x18 << 24) + (statusOfDTCRequest << 16) + (GroupOfDTC);

    snprintf(szCommand, 10, "%X\r", iCommand);
    GetCommandResponse(szCommand, buffer, 100);

    nBytes = (strlen(buffer)-1)/2;
    i = 0;
    start = buffer + 4;

    ullBuffer = strtoll(buffer, NULL, 16);

    if (GetByte(ullBuffer, 1, nBytes) != 0x58)
        return 0;

    nDTCs = GetByte(ullBuffer, 2, nBytes);

    while (i < nDTCs)
    {
        ullBuffer = strtoll(start, NULL, 16);
        nBytes = (strlen(start)-1)/2;

        if (ArrayOfDTCs)
            ArrayOfDTCs[i] = GetWord(ullBuffer, 1, nBytes);

        start+=6;
        i++;
    }

    return nDTCs; // Number of DTCs
}


int ReadDataByCommonIdentifier32( short RecordCommonIdentifier )
{
    int response, command;

    command = (0x22 << 16) + (RecordCommonIdentifier);
    response = GetCommandResponse32(command);

    if ((response & 0xff000000) >> 24 != 0x62)
        return 0;

    return (response & 0x000000ff);
}


int ReadDataByCommonIdentifier64( short RecordCommonIdentifier )
{
    int command;
    long long response;

    command = (0x22 << 16) + (RecordCommonIdentifier);
    response = GetCommandResponse64(command);

    if ((response & 0xff00000000) >> 32 != 0x62)
        return 0;

    return (response & 0xffff);
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "abs.h"
#include "kwp2k.h"
#include "atenzo.h"

typedef unsigned char byte;

byte GetByte( unsigned long long Value, int BytePosition, int NumberOfBytes );
ushort GetWord( unsigned long long Value, int BytePosition, int NumberOfBytes );


/* ABS Functions */
int ABS_GetWheelSpeed( unsigned char Wheel )
{
    switch (Wheel)
    {
        case WSPD_FRONT_LEFT:   return 0;
        case WSPD_FRONT_RIGHT:  return 0;
        case WSPD_REAR_LEFT:    return ReadDataByCommonIdentifier32(0x398A);
        case WSPD_REAR_RIGHT:   return ReadDataByCommonIdentifier32(0x398B);

        default: return 0;
    }
}


int ABS_GetDiagnosticTroubleCodes( unsigned short* ArrayOfDTCs )
{
    //return ReadDiagnosticTroubleCodesByStatus(0x00, 0xFF00, ArrayOfDTCs);

    int iCommand, statusOfDTCRequest, nBytes, nDTCs;
    unsigned long long ullBuffer;
    char szCommand[10], buffer[100];
    char *start;
    int i;

    statusOfDTCRequest = 0x00;
    iCommand = (0x18 << 24) + (statusOfDTCRequest << 16) + (0xFF00);

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

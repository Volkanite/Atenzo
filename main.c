#include <Windows.h>
#include "ftd2xx_mini.h"

TYPE_FT_Open                    FT_Open;
TYPE_FT_Read                    FT_Read;
TYPE_FT_Write                   FT_Write;
TYPE_FT_SetTimeouts             FT_SetTimeouts;
TYPE_FT_GetQueueStatus          FT_GetQueueStatus;
TYPE_FT_GetStatus               FT_GetStatus;
TYPE_FT_SetBaudRate             FT_SetBaudRate;
TYPE_FT_ClrDtr                  FT_ClrDtr;
TYPE_FT_SetDataCharacteristics  FT_SetDataCharacteristics;
TYPE_FT_SetLatencyTimer         FT_SetLatencyTimer;

FT_HANDLE AzoDevice;


int ReadDevice( FT_HANDLE Device, LPVOID Buffer, unsigned int *Length )
{
    FT_STATUS status;
    DWORD dwRxBytes;
    int bytesRead;
    int numBytes;
    DWORD time;
    unsigned int len;

    dwRxBytes = 0;

    time = timeGetTime();
    status = FT_GetQueueStatus(Device, &dwRxBytes);

    if (status == FT_OK)
    {
        while (1)
        {
            numBytes = dwRxBytes;

            if (dwRxBytes)
                break;

            if (timeGetTime() - time >= 2000)
            {
                numBytes = dwRxBytes;
                break;
            }

            Sleep(2);

            status = FT_GetQueueStatus(Device, &dwRxBytes);

            if (status != FT_OK)
                return -1;
        }

        if (status == FT_OK)
        {
            if (!numBytes)
                return numBytes;

            if (numBytes < *Length)
                *Length = numBytes;

            len = *Length;
            bytesRead = 0;

            status = FT_Read(Device, Buffer, len, &bytesRead);

            if (status == FT_OK && bytesRead == *Length)
                return bytesRead;
        }
    }

    return -1;
}


int WriteDevice( FT_HANDLE Device, LPVOID Buffer, int Length )
{
    DWORD dwBytesWritten;

    dwBytesWritten = 0;

    FT_Write(Device, Buffer, Length, &dwBytesWritten);

    return dwBytesWritten;
}


void Echo( char* Command )
{
    char buffer[32];
    DWORD dwRxSize = 32;
    char *start, *end;

    WriteDevice(AzoDevice, Command, strlen(Command));
    ReadDevice(AzoDevice, &buffer, &dwRxSize);

    start = strchr(&buffer, '\r');

    if (start)
    {
        start++;
        end = strchr(start, '\r');
    }

    // dirty check for echo off
    if (end - start < 1)
    {
        start = &buffer;
    }

    buffer[dwRxSize] = '\0';

    //replace line feeds at end with null terminator
    for (int i = 1; i < 4; i++)
    {
        if (buffer[dwRxSize - i] == '\r')
            buffer[dwRxSize - i] = '\0';
    }

    printf("%s \n", start);
}


void Send( char* Command )
{
    char buffer[32];
    DWORD dwRxSize = 32;

    WriteDevice(AzoDevice, Command, strlen(Command));
    ReadDevice(AzoDevice, &buffer, &dwRxSize);
}


int main()
{
    HMODULE ftdi;
    FT_HANDLE device;
    char buffer[32];
    FT_STATUS status;

    device = NULL;
    ftdi = LoadLibraryW(L"ftd2xx.dll");

    FT_Open = (TYPE_FT_Open)GetProcAddress(ftdi, "FT_Open");
    FT_Read = (TYPE_FT_Read)GetProcAddress(ftdi, "FT_Read");
    FT_Write = (TYPE_FT_Write)GetProcAddress(ftdi, "FT_Write");
    FT_SetTimeouts = (TYPE_FT_SetTimeouts)GetProcAddress(ftdi, "FT_SetTimeouts");
    FT_GetQueueStatus = (TYPE_FT_GetQueueStatus)GetProcAddress(ftdi, "FT_GetQueueStatus");
    FT_GetStatus = (TYPE_FT_GetStatus)GetProcAddress(ftdi, "FT_GetStatus");
    FT_SetBaudRate = (TYPE_FT_SetBaudRate)GetProcAddress(ftdi, "FT_SetBaudRate");
    FT_ClrDtr = (TYPE_FT_ClrDtr)GetProcAddress(ftdi, "FT_ClrDtr");
    FT_SetDataCharacteristics = (TYPE_FT_SetDataCharacteristics)GetProcAddress(ftdi, "FT_SetDataCharacteristics");
    FT_SetLatencyTimer = (TYPE_FT_SetLatencyTimer)GetProcAddress(ftdi, "FT_SetLatencyTimer");

    status = FT_Open(0, &device);

    if (status != FT_OK)
    {
        printf("failed to open device!");
        return -1;
    }

    AzoDevice = device;

    FT_SetBaudRate(device, 115200);
    FT_ClrDtr(device);
    FT_SetDataCharacteristics(device, 0x8, 0x0, 0x0);
    FT_SetTimeouts(device, 2000, 1000);
    FT_SetLatencyTimer(device, 0x2);

    // Reboot device to flush any bad settings
    //Send("ATWS\r");
    FT_SetBaudRate(device, 115200);

    Send("ATE0\r");//Echo off 
    Echo("ATI\r"); //identify
    Echo("STI\r"); //Print firmware ID string
    Echo("STDI\r"); //Print device hardware ID string
    Echo("ATDP\r"); //describe current protocol
    Echo("ATRV\r"); //read voltage

    /*Set current protocol preset to ISO 15765, 11-bit Tx, 125kbps, DLC=8 .
    Medium Speed CAN (MS-CAN) is a dual-wire transceiver typically connected 
    to pins 3 and 11 of the OBD port(Ford MSC network).*/
    Send("STP53\r");

	Send("ATCFC1\r"); //can flow control on
	Send("STCSEGR1\r"); //turn CAN Rx segmentation on
	Send("STCSEGT1\r"); // turn CAN Tx segmentation on
	Send("ATSH7E0\r"); // set the header of transmitted OBD messages
	Send("ATCRA7E8\r"); // set CAN hardware filter
	Send("ATCEAAE\r"); //use CAN extended address hh
	Send("ATTAAE\r"); //set tester address to hh
	Send("STCFCPC\r");     //# clear all flow control address pairs
	Send("STCFCPA 7E0 AE, 7E8 AE\r");   //# add flow control address pair

	//# message to check status of Brakes(last byte in response : 00 - brake is off, 01/02 - brake is on)
	//BOO = Brake On/Off Switch
	Echo("221101\r");

	/*while (0)
	{
		Echo("221101\r");
		Sleep(1000);
	}*/

    return 0;
}
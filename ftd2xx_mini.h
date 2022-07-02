typedef PVOID   FT_HANDLE;
typedef ULONG   FT_STATUS;

typedef FT_STATUS(__stdcall* TYPE_FT_Open)(int iDevice, FT_HANDLE *ftHandle);
typedef FT_STATUS(__stdcall* TYPE_FT_Read)(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD nBufferSize, LPDWORD lpBytesReturned);
typedef FT_STATUS(__stdcall* TYPE_FT_Write)(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpdwBytesWritten);
typedef FT_STATUS(__stdcall* TYPE_FT_SetTimeouts)(FT_HANDLE ftHandle, ULONG ReadTimeout, ULONG WriteTimeout);
typedef FT_STATUS(__stdcall* TYPE_FT_GetQueueStatus)(FT_HANDLE ftHandle, DWORD *dwRxBytes);
typedef FT_STATUS(__stdcall* TYPE_FT_GetStatus)(FT_HANDLE ftHandle, DWORD *dwRxBytes, DWORD *dwTxBytes, DWORD *dwEventDWord);
typedef FT_STATUS(__stdcall* TYPE_FT_SetBaudRate)(FT_HANDLE ftHandle, ULONG BaudRate);
typedef FT_STATUS(__stdcall* TYPE_FT_ClrDtr)(FT_HANDLE ftHandle);
typedef FT_STATUS(__stdcall* TYPE_FT_SetDataCharacteristics)(FT_HANDLE ftHandle, UCHAR WordLength, UCHAR StopBits, UCHAR Parity);
typedef FT_STATUS(__stdcall* TYPE_FT_SetLatencyTimer)(FT_HANDLE ftHandle, UCHAR ucLatency);

enum {
FT_OK,
FT_INVALID_HANDLE
};

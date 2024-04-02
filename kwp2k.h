typedef unsigned char byte;


typedef enum
{
    ReturnControlToECU,
    ReportCurrentState,
    ReportIOConditions,
    ReportIOScaling,
    ResetToDefault,
    FreezeCurrentState,
    ExecuteControlState,
    ShortTermAdjustment,
    LongTermAdjustment,
    ReportIOCalibrationParameters

}IOCP;

typedef enum
{
    /*Only the powerOn value of the resetMode (RM_) parameter is supported for the Atenza*/

    reservedByDocument00,
    PowerOn,
    reservedByDocument02,
    KeyOn

}RM_;


int ECUReset(byte ResetMode);
int ClearDiagnosticInformation(unsigned short GroupOfDiagnosticInformation);
int ReadDiagnosticTroubleCodesByStatus(byte StatusOfDTCRequest, unsigned short GroupOfDTC, unsigned short* ArrayOfDTCs);
int ReadDataByCommonIdentifier32(short RecordCommonIdentifier);
int ReadDataByCommonIdentifier64(short RecordCommonIdentifier);
int InputOutputControlByCommonIdentifier(short InputOutputCommonIdentifier, byte InputOutputControlParameter, byte ControlState);

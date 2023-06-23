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


int ReadDiagnosticTroubleCodesByStatus(byte StatusOfDTCRequest, unsigned short GroupOfDTC, unsigned short* ArrayOfDTCs);
int ReadDataByCommonIdentifier32(short RecordCommonIdentifier);
int InputOutputControlByCommonIdentifier(short InputOutputCommonIdentifier, byte InputOutputControlParameter, byte ControlState);

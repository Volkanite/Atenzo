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


int ReadDataByCommonIdentifier32(short RecordCommonIdentifier);
int InputOutputControlByCommonIdentifier(short InputOutputCommonIdentifier, byte InputOutputControlParameter, byte ControlState);

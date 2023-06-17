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


int InputOutputControlByCommonIdentifier(short InputOutputCommonIdentifier, byte InputOutputControlParameter, byte ControlState);

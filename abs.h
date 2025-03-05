/* ABS Functions */
int ABS_ClearDiagnosticTroubleCodes();
int ABS_GetDiagnosticTroubleCodes( unsigned short* ArrayOfDTCs );
int ABS_GetWheelSpeed( unsigned char Wheel );

typedef enum _WHEEL_SPEED
{
    WSPD_FRONT_LEFT,
    WSPD_FRONT_RIGHT,
    WSPD_REAR_LEFT,
    WSPD_REAR_RIGHT

} WHEEL_SPEED;

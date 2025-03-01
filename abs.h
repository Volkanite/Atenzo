/* ABS Functions */
int ABS_GetWheelSpeed( unsigned char Wheel );
int ABS_GetDiagnosticTroubleCodes( unsigned short* ArrayOfDTCs );

typedef enum _WHEEL_SPEED
{
    WSPD_FRONT_LEFT,
    WSPD_FRONT_RIGHT,
    WSPD_REAR_LEFT,
    WSPD_REAR_RIGHT

} WHEEL_SPEED;

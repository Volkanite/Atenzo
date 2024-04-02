/* Diagnostic and Communications Management */
int AuthenticateSession();
int StartDiagnosticSession(int SessionId);
int ResetEngineControlUnit();
int ClearDiagnosticTroubleCodes();
int GetDiagnosticTroubleCodes(unsigned short* ArrayOfDTCs);

/* Data Transmission */
float GetAlternatorDutyCycle();
float GetAlternatorOutputVoltage();
int GetBrakeSwitchState();
float GetControlModuleVoltage();
int GetDiagnosticTroubleCodeCount();
int GetEngineCoolantTemperature();
int GetEngineSpeed();
int GetFanState(int* Fan1, int* Fan2);
int GetFuelSystemStatus();
float GetIntakeAirMassFlowRate();
int GetIntakeAirTemperature();
float GetLongTermFuelTrim();
float GetShortTermFuelTrim();
float GetThrottlePosition();
int GetTransmissionFluidTemperature();
int GetTransmissionForwardGearCommanded();
float GetTransmissionLinePressureSolenoidAmperage();
int GetTransmissionOilPressureSwitchState();
char GetTransmissionRangeSensorPosition();
int GetTransmissionTurbineShaftSpeed();

/* Input / Output Control */
int SetFanState(int Index, int State);
int SetTransmissionLinePressureSolenoidAmperage(float Amperage);


/* ABS Functions */
int ABS_GetWheelSpeed( unsigned char Wheel );

typedef enum _WHEEL_SPEED
{
    WSPD_FRONT_LEFT,
    WSPD_FRONT_RIGHT,
    WSPD_REAR_LEFT,
    WSPD_REAR_RIGHT

} WHEEL_SPEED;


/* Session IDs */
#define SESSION_DEFAULT     0x81
#define SESSION_PROGRAMMING 0x85
#define SESSION_ADJUSTMENT  0x87
#define SESSION_EOL         0xFB

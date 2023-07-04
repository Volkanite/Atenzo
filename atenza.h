/* Diagnostic and Communications Management */
int AuthenticateSession();
int StartDiagnosticSession(int SessionId);
int ClearDiagnosticTroubleCodes();
int GetDiagnosticTroubleCodes(unsigned short* ArrayOfDTCs);

/* Data Transmission */
int GetBrakeSwitchState();
int GetDiagnosticTroubleCodeCount();
int GetEngineCoolantTemperature();
int GetEngineSpeed();
int GetFanState();
int GetFuelSystemStatus();
float GetIntakeAirMassFlowRate();
int GetIntakeAirTemperature();
float GetLongTermFuelTrim();
float GetShortTermFuelTrim();
float GetThrottlePosition();
int GetTransmissionFluidTemperature();
float GetTransmissionLinePressureSolenoidAmperage();
int GetTransmissionOilPressureSwitchState();
char GetTransmissionRangeSensorPosition();
int GetTransmissionTurbineShaftSpeed();

/* Input / Output Control */
int SetFanState(int State);
int SetTransmissionLinePressureSolenoidAmperage(float Amperage);

/* Session IDs */
#define SESSION_DEFAULT     0x81
#define SESSION_PROGRAMMING 0x85
#define SESSION_ADJUSTMENT  0x87
#define SESSION_EOL         0xFB

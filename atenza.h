/* Diagnostic and Communications Management */
int AuthenticateSession();
int StartExtendedDiagnoticSession();
int StopExtendedDiagnosticSession();

/* Data Transmission */
int GetBrakeSwitchState();
int GetEngineCoolantTemperature();
int GetEngineRevolutionsPerMinute();
int GetFanState();
float GetThrottlePosition();
int GetTransmissionFluidTemperature();
float GetTransmissionLinePressureSolenoidAmperage();
int GetTransmissionOilPressureSwitchState();
char GetTransmissionRangeSensorPosition();
int GetTransmissionTurbineShaftSpeed();

/* Input / Output Control */
int SetFanState(int State);
int SetTransmissionLinePressureSolenoidAmperage(float Amperage);

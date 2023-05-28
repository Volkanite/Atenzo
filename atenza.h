/* Diagnostic and Communications Management */
int AuthenticateSession();
int StartExtendedDiagnoticSession();
int StopExtendedDiagnosticSession();

/* Data Transmission */
int GetBrakeSwitchState();
int GetEngineCoolantTemperature();
int GetEngineRevolutionsPerMinute();
int GetFanState();
int GetFuelSystemStatus();
float GetIntakeAirMassFlowRate();
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

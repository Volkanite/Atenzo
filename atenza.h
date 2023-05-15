/*ECU Operations*/
int AuthenticateSession();

/*Diagnostics*/
int GetBrakeSwitchState();
int GetEngineCoolantTemperature();
int GetEngineRevolutionsPerMinute();
int GetFanState();
int GetTransmissionFluidTemperature();
float GetTransmissionLinePressureSolenoidAmperage();
int GetTransmissionOilPressureSwitchState();
int GetTransmissionTurbineShaftSpeed();
int SetFanState(int State);

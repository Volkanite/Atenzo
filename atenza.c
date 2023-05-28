long GetCommandResponseAsLong(char* Command);
long long GetCommandResponseAsLongLong(char* Command);
long long GetCommandResponse64(long long Command);
int GetKeyFromSeed(int Seed);


int StartExtendedDiagnoticSession()
{
    int response;
    
    response = GetCommandResponseAsLong("1087\r");
    
    if ((response & 0xff00) >> 8 != 0x50)
        return 0;
        
    return 1;
}


int StopExtendedDiagnosticSession()
{
    int response;
    
    response = GetCommandResponseAsLong("1081\r");
    
    if ((response & 0xff00) >> 8 != 0x50)
        return 0;
        
    return 1;
}


int AuthenticateSession()
{
    long long response, key;
    int seed;
    
    if (!StartExtendedDiagnoticSession())
        return 0;
            
    //Authenticate
    response = GetCommandResponseAsLongLong("2701\r");
    
    if ((response & 0xff00000000) >> 32 != 0x67)
        return 0;
    
    seed = (response & 0x0000ffffff);
    
    key = GetKeyFromSeed(seed);
    key += 0x2702000000;
    
    //send key
    response = GetCommandResponse64(key); 
    
    if ((response & 0xff00) >> 8 != 0x67)
        return 0;
    
    return 1;
}


int GetBrakeSwitchState()
{
	int state;
	
	state = GetCommandResponseAsLong("221101\r");
	state = (state & 0x000000ff);
	
	return (state >> 1) & 1;
}


int GetEngineCoolantTemperature()
{
    int temp;
    
	temp = GetCommandResponseAsLong("220005\r");
    temp = (temp & 0x000000ff);
    temp -= 40; //Celsius
    
    return temp;
}


int GetEngineRevolutionsPerMinute()
{
    long long response;
    short rpm;
    
    response = GetCommandResponseAsLongLong("22000C\r");
    
    if ((response & 0xff00000000) >> 32 != 0x62)
        return 0;
    
    rpm = (response & 0xffff);
    
    return rpm * 0.25;
}


int GetFanState()
{
	int fans;

	fans = GetCommandResponseAsLong("221103\r");
	fans = (fans & 0x000000ff);
    
    return (fans >> 3) & 1;
}


int GetFuelSystemStatus()
{
    long long response;
    
    response = GetCommandResponseAsLongLong("220003\r");
    
    if ((response & 0xff00000000) >> 32 != 0x62)
        return 0;
        
    response = (response & 0x000000ff00) >> 8;
    
    return response;
}


float GetIntakeAirMassFlowRate()
{
    long long response;
    float flowRate;
    
    response = GetCommandResponseAsLongLong("220010\r");
    
    if ((response & 0xff00000000) >> 32 != 0x62)
        return 0.0f;
        
    flowRate = (float)(response & 0x000000ffff);
    flowRate = (flowRate * 0.01f);
    
    return flowRate;
}


float GetLongTermFuelTrim()
{
    int response;
    float percentage;
    
    response = GetCommandResponseAsLong("220007\r");
    
    if ((response & 0xff000000) >> 24 != 0x62)
        return 0.0f;
        
    percentage = (float)(response & 0x000000ff);
    percentage = (percentage - 128.0f) * 0.78125f;
    
    return percentage;
}


float GetShortTermFuelTrim()
{
    int response;
    float percentage;
    
    response = GetCommandResponseAsLong("220006\r");
    
    if ((response & 0xff000000) >> 24 != 0x62)
        return 0.0f;
        
    percentage = (float)(response & 0x000000ff);
    percentage = (percentage - 128.0f) * 0.78125f;
    
    return percentage;
}


float GetThrottlePosition()
{
    int response;
    float percentage;
    
    response = GetCommandResponseAsLong("2217B6\r");
    
    if ((response & 0xff000000) >> 24 != 0x62)
        return 0.0f;
        
    percentage = (float)(response & 0x000000ff);
    percentage = (percentage * 100.0f) / 256.0f;
    
    return percentage;
}


int GetTransmissionFluidTemperature()
{
    int temp, temp2;
    //short scale;
    //short adder;
    
    temp = GetCommandResponseAsLong("2217B3\r");
    temp = (temp & 0x000000ff);
    
    /*
    This calculation is based on Scanguage's MTH 
    field and seems to be accurate
    //TXD        RXF          RXD  MTH
    //07E02217B3 0462051706B3 3008 002A0019FFC7
    */
    
    //0xFFC7 = -57
    temp = ((temp * 0x2A) / 0x19) + -57; //Fahrenheit
    temp2 = (temp - 32) * (5.0/9.0); //Celsius
    
    return temp2;
}


float GetTransmissionLinePressureSolenoidAmperage()
{
    int response;
    float amperage;
    
    response = GetCommandResponseAsLong("2217B8\r");
    
    if ((response & 0xff000000) >> 24 != 0x62)
        return 0.0f;
    
    response = (response & 0xff); 
           
    amperage = (float) response;
    amperage /= 256.0f;
    
    return amperage;
}


int GetTransmissionOilPressureSwitchState()
{
    int state;
    
    state = GetCommandResponseAsLong("221709\r");
    state = (state & 0x000000ff);
    
    return state;
}


char GetTransmissionRangeSensorPosition()
{
    int position;
    
    position = GetCommandResponseAsLong("2217B1\r");
    
    if ((position & 0xff000000) >> 24 != 0x62)
        return 'E'; //error;
        
    position = (position & 0x000000ff);
    
    switch (position)
    {
        case 1: return 'R';
        case 2: return 'N';
        case 4: return 'L';
        case 8: return 'S';
        case 16: return 'D';
        case 32: return 'P';
        
        //error
        default: return 'E';
    }
    
    return 'E';
}


int GetTransmissionTurbineShaftSpeed()
{
	int speed;
		
	speed = GetCommandResponseAsLong("2217B0\r");
	
	if ((speed & 0xff000000) >> 24 != 0x62)
        return 0;
        
	speed = (speed & 0x000000ff);
	speed = (speed * 375) / 10;
	
	return speed;
}


int SetFanState( int State )
{
    long long response, command;
    
    //response = GetCommandResponseAsLongLong("2F17C40701\r");
    
    if (State > 1)
        return 0;
    
    command = 0x2F17C40700 + State;
    response = GetCommandResponse64(command);
    
    if ((response & 0xff00000000) >> 32 != 0x6F)
        return 0;
    
    //fan read actually turns on fan [VERIFY]
    GetCommandResponseAsLong("221103\r");
    
    return 1;
}


/*
Conditions: These conditions must be met in order to adjust this
parameter else you'll get a negative response.
TR = 'N' //Transmission Shift Lever in Neutral position
THOP = 0.0 //Throttle fully closed
*/

int SetTransmissionLinePressureSolenoidAmperage( float Amperage )
{
    long long response;
    
    response = GetCommandResponseAsLongLong("2F17C20700\r");
    
    if ((response & 0xff00000000) >> 32 != 0x6F)
        return 0;
            
    return 1;
}

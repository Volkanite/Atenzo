long GetCommandResponseAsLong(char* Command);
long GetCommandResponseAsLongLong(char* Command);


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

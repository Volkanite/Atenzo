long GetCommandResponseAsLong(char* Command);


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


int GetFanState()
{
	int fans;

	fans = GetCommandResponseAsLong("221103\r");
	fans = (fans & 0x000000ff);
    
    return (fans >> 3) & 1;
}

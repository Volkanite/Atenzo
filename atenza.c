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

#include "atenza.h"
#include "kwp2k.h"
#include <math.h>


long GetCommandResponseAsLong(char* Command);
long GetCommandResponse32(long Command);
long long GetCommandResponseAsLongLong(char* Command);
long long GetCommandResponse64(long long Command);
int GetKeyFromSeed(int Seed);


int StartDiagnosticSession( int SessionId )
{
    int response, command;

    command = 0x1000 + SessionId;
    response = GetCommandResponse32(command);

    if ((response & 0xff00) >> 8 != 0x50)
        return 0;

    return 1;
}


int ClearDiagnosticTroubleCodes()
{
    return ClearDiagnosticInformation(0xFF00);
}


int GetDiagnosticTroubleCodes( unsigned short* ArrayOfDTCs )
{
    return ReadDiagnosticTroubleCodesByStatus(0x00, 0xFF00, ArrayOfDTCs);
}


int AuthenticateSession()
{
    long long response, key;
    int seed;

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

	  state = ReadDataByCommonIdentifier32(0x1101);

    if (!state) return 0;

	  return (state >> 1) & 1;
}


int GetDiagnosticTroubleCodeCount()
{
    return ReadDataByCommonIdentifier32(0x0200);
}


int GetEngineCoolantTemperature()
{
    int temp;

	  temp = ReadDataByCommonIdentifier32(0x0005);

    if (!temp) return 0;

    return temp - 40; //Celsius
}


int GetEngineSpeed()
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

	  fans = ReadDataByCommonIdentifier32(0x1103);

    if (!fans) return 0;

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


int GetIntakeAirTemperature()
{
    int response;

    response = ReadDataByCommonIdentifier32(0x000F);

    if (!response) return 0;

    return response - 40; //Celsius
}


float GetLongTermFuelTrim()
{
    int response;
    float percentage;

    response = ReadDataByCommonIdentifier32(0x0007);

    if (!response) return 0.0f;

    percentage = (float) response;
    percentage = (percentage - 128.0f) * 0.78125f;

    return percentage;
}


float GetShortTermFuelTrim()
{
    int response;
    float percentage;

    response = ReadDataByCommonIdentifier32(0x0006);

    if (!response) return 0.0f;

    percentage = (float) response;
    percentage = (percentage - 128.0f) * 0.78125f;

    return percentage;
}


float GetThrottlePosition()
{
    int response;
    float percentage;

    response = ReadDataByCommonIdentifier32(0x17B6);

    if (!response) return 0.0f;

    percentage = (float) response;
    percentage = (percentage * 100.0f) / 256.0f;

    return percentage;
}


int GetTransmissionFluidTemperature()
{
    int response;
    float temperature;

    response = ReadDataByCommonIdentifier32(0x17B3);

    if (!response) return 0;

    temperature = (float)response;
    temperature = ((temperature * 240.0) / 256.0) - 53.0;

    return round(temperature);
}


int GetTransmissionForwardGearCommanded()
{
    int response;

    response = ReadDataByCommonIdentifier32(0x17B2);

    switch (response)
    {
        case 1: return 1;
        case 2: return 2;
        case 4: return 3;
        case 8: return 4;
        case 16: return 5;
        case 32: return 6;

        //error
        default: return 0;
    }

    return 0;
}


float GetTransmissionLinePressureSolenoidAmperage()
{
    int response;
    float amperage;

    response = ReadDataByCommonIdentifier32(0x17B8);

    if (!response) return 0.0f;

    amperage = (float) response;
    amperage /= 256.0f;

    return amperage;
}


int GetTransmissionOilPressureSwitchState()
{
    return ReadDataByCommonIdentifier32(0x1709);
}


char GetTransmissionRangeSensorPosition()
{
    int position;

    position = ReadDataByCommonIdentifier32(0x17B1);

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

    speed = ReadDataByCommonIdentifier32(0x17B0);

	  if (!speed) return 0;

	  return (speed * 375) / 10;
}


int SetFanState( int State )
{
    if(!InputOutputControlByCommonIdentifier(0x17C4, ShortTermAdjustment, State))
        return 0;

    //fan read actually turns on fan [VERIFY]
    GetFanState();

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
    if(!InputOutputControlByCommonIdentifier(0x17C2, ShortTermAdjustment, 0))
        return 0;

    return 1;
}

#include "pcm.h"
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


int ResetEngineControlUnit()
{
    return ECUReset(PowerOn);
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

    if (seed == 0)
        return 1; //already unlocked

    key = GetKeyFromSeed(seed);
    key += 0x2702000000;

    //send key
    response = GetCommandResponse64(key);

    if ((response & 0xff00) >> 8 != 0x67)
        return 0;

    return 1;
}


float GetAlternatorDutyCycle()
{
    return (float)ReadDataByCommonIdentifier64(0x16E8) * 0.003051757812f;
}


float GetAlternatorOutputVoltage()
{
    return (float)ReadDataByCommonIdentifier64(0x16E9) * 0.125f;
}


int GetBrakeSwitchState()
{
    int state;

	  state = ReadDataByCommonIdentifier32(0x1101);

    if (!state) return 0;

	  return (state >> 1) & 1;
}


float GetControlModuleVoltage()
{
    return (float)ReadDataByCommonIdentifier64(0x0042) * 0.001f;
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
    int rpm;

    rpm = ReadDataByCommonIdentifier64(0x000C);

    if (!rpm) return 0;

    return rpm * 0.25;
}


int GetFanState( int* Fan1, int* Fan2 )
{
    int response;

    response = ReadDataByCommonIdentifier32(0x1103);

    if (!response) return 0;

    //low speed (both fans)
    //Right fan (inside car, looking out of windshield)
    //4-pin
    if (Fan1)
        *Fan1 = (response >> 2) & 1;

    //high speed (one fan)
    //Left fan (inside car, looking out of windshield)
    //2-pin
    if (Fan2)
        *Fan2 = (response >> 3) & 1;

    return 1;
}


int GetFuelSystemStatus()
{
    int response;

    response = ReadDataByCommonIdentifier64(0x0003);

    if (!response) return 0;

    return response >> 8;
}


float GetIntakeAirMassFlowRate()
{
    int response;
    float flowRate;

    response = ReadDataByCommonIdentifier64(0x0010);

    if (!response) return 0.0f;

    flowRate = (float)(response);
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
    return (float)ReadDataByCommonIdentifier32(0x17B6) * 0.390625f;
}


int GetTransmissionFluidTemperature()
{
    int response;
    float temperature;

    response = ReadDataByCommonIdentifier32(0x17B3);

    if (!response) return 0;

    temperature = (float)response;
    temperature = (temperature * 0.9375f) - 53.0f;

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
    return (float)ReadDataByCommonIdentifier32(0x17B8) * 0.00390625f;
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
    return (int)(float)ReadDataByCommonIdentifier32(0x17B0) * 37.5f;
}


/*
These conditions must be met in order to adjust these parameters else you'll get
a negative response from the ECU.
TR = 'N' or 'P' //Transmission Shift Lever in Neutral position
THOP = 0.0 //Throttle fully closed
*/

/*
0 based fan index.
0 = FAN1; 1 = FAN2
*/

int SetFanState( int Index, int State )
{
    short id;

    if (Index == 0)
        id = 0x17C3;
    else if (Index == 1)
        id = 0x17C4;
    else
        return 0;

    if(!InputOutputControlByCommonIdentifier(id, ShortTermAdjustment, State))
        return 0;

    return 1;
}


int SetTransmissionLinePressureSolenoidAmperage( float Amperage )
{
    if(!InputOutputControlByCommonIdentifier(0x17C2, ShortTermAdjustment, 0))
        return 0;

    return 1;
}

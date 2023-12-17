#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include <curses.h>
#include <sys/time.h>
#include <time.h>

#include "atenza.h"
#include "sound.h"


typedef struct _PID PID;
typedef int (*PID_CONDITIONAL_THRESHOLD)(PID* ParameterIdsBase);

typedef struct _PID
{
    char* Name;
    char* Unit;
    int ValueType; //0=int,1=float
    int HasThreshold;
    float Threshold;
    PID_CONDITIONAL_THRESHOLD ConditionalThreshold;
    int Value;
    float Value2;
}PID;


typedef enum _PidValueType
{
    Type_Int,
    Type_Float,
    Type_Char,
    Type_String

}PidValueType;


typedef enum _PID_INDEX
{
    ECT,
    TFT,
    IAT,
    FAN1,
    FAN2,
    BOO,
    OP_SW_B,
    RPM,
    TSS,
    LPS,
    TR,
    THOP,
    DR,
    LONGFT1,
    SHRTFT1,
    MAF,
    FUELSYS1,
    DTC_CNT,
    GEAR,
    VPWR

} PID_INDEX;

#define TRUE 1
#define FALSE 0

int Device = 0;
int LogFile = 0;
int Debug = 0;
int ScreenX = 0;
int ScreenY = 0;
int64_t EngineStartTime;
PID* ParameterIdsBase;

#define FAN_CTRL_HI     95
#define FAN_CTRL_LO     90
#define FAN_CTRL_CRIT   100


char * removeCharFromStr(char *string, char character);
void GetClockTime( char* Buffer, struct tm* Time );


void Send( char* Command, int Len )
{
    write(Device, Command, Len ? Len : strlen(Command));
}


void LogToFile( char* Format, ... )
{
    char output[121], buffer[100], time[20];
    va_list args;
    int result;

    va_start(args, Format);
    vsnprintf(buffer, 100, Format, args);
    va_end(args);

    GetClockTime(time, NULL);

    strcpy(output, time);
    strcat(output, " ");
    strcat(output, buffer);
    strcat(output, "\n");

    write(LogFile, output, strlen(output));
}


void GetCommandResponse( char* Command, char* Buffer, int BufferLength)
{
    int cmdLen;

    cmdLen = strlen(Command);

    if (Debug)
        LogToFile("=> %s", Command);

    Send(Command, cmdLen);

    int n=0, ntot=0;
    const int maxLineLength = 1024, max_ntot = 20480;
    char str[maxLineLength + 1], *readpt=str;
     do {
        n = read(Device, readpt, maxLineLength);
        if (n > 0)
        {
            ntot += n;
            readpt[n] = '\0';
           // printf("%s", readpt);
            readpt += n;
        }
    } while ((n > 0) && (*(readpt-1)!='>') && (ntot<max_ntot) );

   if (Buffer)
   {
      //ncurses doesn't like the CRs
      removeCharFromStr(str, '\r');
      strncpy(Buffer, str, BufferLength);

      if (Debug)
        LogToFile("<= %s", Buffer);
   }
}


long GetCommandResponseAsLong( char* Command )
{
    char buffer[100];

    GetCommandResponse(Command, buffer, 100);
    //removeSpacesFromStr(buffer);

    return strtol(buffer, NULL, 16);
}


long long GetCommandResponseAsLongLong( char* Command )
{
    char buffer[100];

    GetCommandResponse(Command, buffer, 100);
    //removeSpacesFromStr(buffer);

    return strtoll(buffer, NULL, 16);
}


long GetCommandResponse32( unsigned int Command )
{
    char buffer[50];

    snprintf(buffer, 50, "%X\r", Command);
    return GetCommandResponseAsLong(buffer);
}


long long GetCommandResponse64( long long Command )
{
    char buffer[50];

    snprintf(buffer, 50, "%llX\r", Command);
    return GetCommandResponseAsLongLong(buffer);
}


void Echo(char* Command)
{
    char buffer[100];

    GetCommandResponse(Command, buffer, 100);
    printw("%s", buffer);
    refresh();
}


// Funtion removing spaces from string
char * removeCharFromStr( char *string, char character )
{
    // non_space_count to keep the frequency of non space characters
    int non_space_count = 0;

    //Traverse a string and if it is non space character then, place it at index non_space_count
    for (int i = 0; string[i] != '\0'; i++)
    {
        if (string[i] != character)
        {
            string[non_space_count] = string[i];
            non_space_count++;//non_space_count incremented
        }
    }

    //Finally placing final character at the string end
    string[non_space_count] = '\0';
    return string;
}


void GetClockTime( char* Buffer, struct tm* Time )
{
    struct timeval tv;
    struct timezone tz;
    struct tm *today;
    int zone;

    gettimeofday(&tv,&tz);

    /* get time details */
    today = localtime(&tv.tv_sec);

    if (Buffer)
        snprintf(Buffer, 20, "%d:%02d:%02d.%06ld", today->tm_hour, today->tm_min, today->tm_sec, tv.tv_usec);

    if (Time)
        *Time = *today;
}


/*
Return current timestamp in milliseconds (64-bit int)
*/
long long current_timestamp()
{
    struct timeval te;
    long long milliseconds;

    gettimeofday(&te, NULL);
    milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;

    return milliseconds;
}


void PrintToScreen( int PosY, char* Message )
{
    move(PosY, 0);
    clrtoeol();
    printw("%s", Message);
}


void StatusPrint( char* Message )
{
    PrintToScreen(ScreenY-1, Message);
}


char* GetFuelSysStatus( int Status )
{
    switch (Status)
    {
        case 0: return "Off";
        case 1: return "OL-T";
        case 2: return "CL-1";
        case 4: return "OL-D";
        case 8: return "OL-F";
        case 16: return "CL-F";

        default: return "Err";
    }
}


int UnlockActuation()
{
    if (!StartDiagnosticSession(SESSION_ADJUSTMENT))
        return 0;

    if (!AuthenticateSession())
        return 0;

    return 1;
}


int GetPidLen( PID* ParameterId )
{
    char buffer[20];
    int len;

    len = snprintf(buffer, 20, "%s: ", ParameterId->Name);

    if (ParameterId->ValueType == Type_Int) //int
        len += snprintf(buffer, 20, "%i", ParameterId->Value);
    else if (ParameterId->ValueType == Type_Float) //float
        len += snprintf(buffer, 20, "%.2f", ParameterId->Value2);
    else if (ParameterId->ValueType == Type_Char) //char
        len += snprintf(buffer, 20, "%c", ParameterId->Value);
    else if (ParameterId->ValueType == Type_String) //string, fuelsys1 is the only string for now
        len += snprintf(buffer, 20, "%s", GetFuelSysStatus(ParameterId->Value));

    if (ParameterId->Unit)
        len += snprintf(buffer, 20, " %s", ParameterId->Unit);

    return len;
}


int IsEngineRunning()
{
    if (ParameterIdsBase[RPM].Value > 500)
        return TRUE;
    else
        return FALSE;
}


int IsVoltageGood( PID* ParameterIdsBasePtr )
{
    if (current_timestamp() - EngineStartTime < 5000)
        return 1;

    if (IsEngineRunning()
        && ParameterIdsBasePtr[VPWR].Value2 > 0.0
        && ParameterIdsBasePtr[VPWR].Value2 < 12.7)
        {
            return 0;
        }

    return 1;
}


int main()
{
    Device = open("/dev/ttyUSB0", O_RDWR);

    if (Device == -1)
    {
        printf("Could not open ttyUSB0: %s\n", strerror(errno));

        switch (errno)
        {
            case 2: printf("Is the adapter plugged in?\n");
                break;
            case 13: printf("Have you added yourself to the dialout group?\n"
            "If in a rush, you can also try running the program with sudo.\n");
                break;
        }

        return 0;
    }

    // Create new termios struct, we call it 'tty' for convention
    struct termios tty;

    // Read in existing settings, and handle any error
    if(tcgetattr(Device, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl

    // Disable any special handling of received bytes
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 115200
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty settings, also checking for error
    if (tcsetattr(Device, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    if (Debug)
    {
        LogFile = open("./debug.log", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    }

    initscr();
    InitializeSound();

    // Reboot device to flush any bad settings
    Send("ATWS\r",0); //soft reset
    //Send("ATZ\r"); //hard reset

    sleep(1);

    Echo("ATL1\r"); // turn on line feed
    Echo("ATE1\r");//Echo on

    Echo("ATI\r"); //identify
    Echo("STI\r"); //Print firmware ID string
    Echo("STDI\r"); //Print device hardware ID string
    Echo("ATDP\r"); //describe current protocol
    Echo("ATRV\r"); //read voltage

    //Set current protocol preset to ISO 15765, 11-bit Tx, 500kbps, DLC=8; High Speed CAN (HS-CAN)
    GetCommandResponse("STP33\r", 0,0);

    GetCommandResponse("ATSH7E0\r", 0,0); // set the header of transmitted OBD messages
    GetCommandResponse("ATL0\r", 0,0); // turn off line feed
    GetCommandResponse("ATE0\r", 0,0); //Echo off
    GetCommandResponse("ATS0\r", 0,0); //Turn off spaces on OBD responses
    GetCommandResponse("STCSEGR1\r", 0,0); //Disable PCI bytes

    int max_x, max_y, voltage_y;
    int currentEngineState, previousEngineState;
    int virtualColumns;
    int fullPressure;
    int releasePressure;
    int awaitingFullPressure;
    char buffer[100];
    long long start, delta;
    int timeout, timeoutValue;
    int manualFanControl, tempHi, tempLo, fan1, fan2;
    int prev_dtc_count;

    PID ParameterIds[] = {
        {"ECT","°C",Type_Int,1,100.0f},
        {"TFT","°C",Type_Int,1,100.0f},
        {"IAT","°C"},
        {"FAN1"},
        {"FAN2"},
        {"BOO"},
        {"TOPS"},
        {"RPM"},
        {"TSS"},
        {"LPS","A",Type_Float},
        {"TR",0,2},
        {"TP","%",Type_Float},
        {"DR",0,Type_Float},
        {"LTFT",0,Type_Float,1,-14.00f},
        {"STFT",0,Type_Float},
        {"MAF","g/s",Type_Float},
        {"FSS", 0,3},
        {"DTCs",0,Type_Int,1,1.0f},
        {"GR"},
        {"VPWR","V",Type_Float,FALSE,0.0f,IsVoltageGood}
    };

    ParameterIdsBase = ParameterIds;

    ScreenX = ScreenY = 0;
    currentEngineState = previousEngineState = 0;
    EngineStartTime = 0;
    fullPressure = releasePressure = awaitingFullPressure = 0;
    start = delta = timeout = timeoutValue = 0;
    manualFanControl = 0;
    prev_dtc_count = 0;

    tempHi = FAN_CTRL_HI;
    tempLo = FAN_CTRL_LO;

    initscr(); //init ncurses
    getyx(stdscr, ScreenY, ScreenX); //backup cursor position

    voltage_y = ScreenY; //backup voltage pos
    ScreenY += 2; //two lines from last echo
    ScreenX = 0; //first column

    getmaxyx(stdscr, max_y, max_x); //get screen size;
    virtualColumns = max_x / 10; //avg PID width is 10 chars
    //printw("%i", virtualColumns);

    while (1)
    {
        ParameterIds[BOO].Value = GetBrakeSwitchState();
        //ParameterIds[FAN].Value = GetFanState();
        ParameterIds[ECT].Value = GetEngineCoolantTemperature();
        ParameterIds[TFT].Value = GetTransmissionFluidTemperature();
        ParameterIds[OP_SW_B].Value = GetTransmissionOilPressureSwitchState();
        ParameterIds[RPM].Value = GetEngineSpeed();
        ParameterIds[TSS].Value = GetTransmissionTurbineShaftSpeed();
        ParameterIds[LPS].Value2 = GetTransmissionLinePressureSolenoidAmperage();
        ParameterIds[TR].Value = GetTransmissionRangeSensorPosition();
        ParameterIds[GEAR].Value = GetTransmissionForwardGearCommanded();
        ParameterIds[THOP].Value2 = GetThrottlePosition();
        ParameterIds[LONGFT1].Value2 = GetLongTermFuelTrim();
        ParameterIds[SHRTFT1].Value2 = GetShortTermFuelTrim();
        ParameterIds[MAF].Value2 = GetIntakeAirMassFlowRate();
        ParameterIds[IAT].Value = GetIntakeAirTemperature();
        ParameterIds[FUELSYS1].Value = GetFuelSystemStatus();
        ParameterIds[DTC_CNT].Value = GetDiagnosticTroubleCodeCount();
        ParameterIds[VPWR].Value2 = GetControlModuleVoltage();

        //Fans
        fan1 = fan2 = 0;

        GetFanState(&fan1, &fan2);

        ParameterIds[FAN1].Value = fan1;
        ParameterIds[FAN2].Value = fan2;

        //Calculated values
        if (ParameterIds[RPM].Value && ParameterIds[TSS].Value)
            ParameterIds[DR].Value2 = (float) ParameterIds[TSS].Value / (float) ParameterIds[RPM].Value;
        else
            ParameterIds[DR].Value2 = 0.0;

        currentEngineState = IsEngineRunning();

        if (currentEngineState == 1 && previousEngineState == 0)
        {
            //Code in this block runs once every engine restart
            struct tm today;
            char buffer[100];
            int newline;

            manualFanControl = 0;
            start = delta = timeout = timeoutValue = 0;
            EngineStartTime = current_timestamp();

            if (Debug)
            {
                GetClockTime(NULL, &today);
                sprintf(buffer, "%s", asctime(&today));

                //remove newline char
                newline = strlen(buffer);
                buffer[newline-1] = '\0';

                LogToFile("[Engine Start] @ %s", buffer);
            }
        }

        previousEngineState = currentEngineState;

        //scan DTCs
        if (ParameterIds[DTC_CNT].Value && prev_dtc_count != ParameterIds[DTC_CNT].Value)
        {
            int dtcFound = 0;
            char buffer[100], buffer2[150];
            ushort DTCs[8];

            memset(DTCs, 0, sizeof(DTCs));

            if(GetDiagnosticTroubleCodes(DTCs))
            {
                strcpy(buffer, "DTCs:");

                for (
                  int i = 0;
                  i < ParameterIds[DTC_CNT].Value && i < sizeof(DTCs)/sizeof(DTCs[0]);
                  i++)
                {
                    snprintf(buffer2, 150, i?", %X":" %X", DTCs[i]);
                    strcat(buffer, buffer2);

                    if (DTCs[i] == 0x500)
                        dtcFound = 1;
                }

                StatusPrint(buffer);

                if (Debug && dtcFound)
                {
                    StatusPrint("Clearing DTCs..");
                    ClearDiagnosticTroubleCodes();
                    PlaySound();
                }
            }
        }

        prev_dtc_count = ParameterIds[DTC_CNT].Value;

        if ((ParameterIds[ECT].Value > tempHi
            || ParameterIds[TFT].Value > tempHi)
            && !ParameterIds[FAN1].Value
            && IsEngineRunning())
        {
            manualFanControl = 1;

            if (UnlockActuation())
            {
                SetFanState(0,1);

                if (ParameterIds[ECT].Value > FAN_CTRL_CRIT
                    || ParameterIds[TFT].Value > FAN_CTRL_CRIT)
                    {
                        SetFanState(1,1); //turn on fan2
                    }
            }
            else
            {
                StatusPrint("UnlockActuation() failed! Fan setting failed!");
            }
        }
        else if (manualFanControl)
        {
            if (ParameterIds[TR].Value == 'P')
            {
                tempHi = FAN_CTRL_HI - 5;
                tempLo = FAN_CTRL_LO - 5;
            }
            else
            {
                tempHi = FAN_CTRL_HI;
                tempLo = FAN_CTRL_LO;
            }

            if (ParameterIds[ECT].Value < tempLo
                && ParameterIds[TFT].Value < tempLo
                && ParameterIds[FAN1].Value)
            {
                StatusPrint("turning off FAN..");
                SetFanState(0,0);

                if (ParameterIds[FAN2].Value)
                    SetFanState(1,0);
            }
        }

        if (Debug
            && !fullPressure
            && !awaitingFullPressure
            && ParameterIds[TR].Value == 'N'
            && ParameterIds[BOO].Value == 1
            && ParameterIds[THOP].Value2 > 0.5)
        {
            int temp;
            float slope, intercept;
            float time;

            StatusPrint("Release throttle to commit LPS..");
            PlaySound();

            awaitingFullPressure = 1;
            temp = ParameterIds[TFT].Value;

            //Point1: x=30°C, y=150000 ms (2.5 min)
            //Point2: x=75°C, y=30000 ms (0.5 min)
            slope = -2666.67f;
            intercept = 230000.0f;

            time = ((float)temp * slope) + intercept;

            if (!timeoutValue)
                timeoutValue = (int) time;

            //set to 30s minimum
            if (timeoutValue < 30000)
                timeoutValue = 30000;
        }

        if (awaitingFullPressure && ParameterIds[THOP].Value2 < 0.78)
        {
            char buffer[50];
            float timeoutInMinutes;

            timeoutInMinutes = (float) timeoutValue / 60000.0f;

            //continue if previously started
            if (start)
            {
                delta = current_timestamp() - start;

                if (timeoutValue - delta < 30000)
                    timeoutValue += 30000;

                timeoutInMinutes = (float)(timeoutValue - delta) / 60000.0f;

                snprintf(buffer, 50, "Resuming full pressure LPS for %.1f mins", timeoutInMinutes);
            }
            else
            {
                snprintf(buffer, 50, "Setting LPS to full pressure for %.1f mins", timeoutInMinutes);
            }

            StatusPrint(buffer);
            PlaySound();

            fullPressure = 1;
            awaitingFullPressure = 0;

            UnlockActuation();
            SetTransmissionLinePressureSolenoidAmperage(0.0);

            /*long long response = GetCommandResponseAsLongLong("2F17C20700\r");
            char responseByte = (response & 0xff00000000) >> 32;

            move(y-1, 0);
            clrtoeol();
            printw("LPS: 0x%X", responseByte);*/
        }

        if (fullPressure && ParameterIds[TR].Value == 'D')
        {
            releasePressure = 1;

            if (!start)
                start = current_timestamp();

            delta = current_timestamp() - start;

            if (delta > timeoutValue)
            {
                if (ParameterIds[THOP].Value2 > 0.30
                    && ParameterIds[THOP].Value2 < 7.0
                    && ParameterIds[RPM].Value < 1300
                    && ParameterIds[TFT].Value < 76
                    && ParameterIds[DR].Value2 > 0.20)
                {
                    StatusPrint("Extending timer for another 30s");
                    timeoutValue += 30000;
                }
                else
                {
                    timeout = 1;
                }
            }
        }

        if ((releasePressure && ParameterIds[TR].Value != 'D') || timeout)
        {
            //Return LPS control to ECU
            StartDiagnosticSession(SESSION_DEFAULT);

            releasePressure = fullPressure = 0;

            if (timeout)
                start = delta = timeout = timeoutValue = 0;

            StatusPrint("Returning LPS control to ECU..");
            PlaySound();
        }

        //Print voltage
        GetCommandResponse("ATRV\r", buffer, 100);
        removeCharFromStr(buffer, '>');
        PrintToScreen(voltage_y-2, buffer);

        //Print PIDs
        move(ScreenY, ScreenX); //restore cursor pos
        clrtobot(); //clear line
        refresh();

        //int j = 1;
        int lineLen = 0;
        int pidLen = 0;

        for (int i = 0; i < sizeof(ParameterIds)/sizeof(ParameterIds[0]); i++)
        {
            int colorChanged;

            colorChanged = FALSE;
            pidLen = GetPidLen(&ParameterIds[i]);

            if (lineLen + pidLen >= max_x)
            {
                printw("\n");
                lineLen = 0;
            }
            else if (i)
            {
                printw("  ");
            }

            printw("%s: ", ParameterIds[i].Name);

            if (ParameterIds[i].HasThreshold || ParameterIds[i].ConditionalThreshold)
            {
                float value;

                //convert everything to floatational numbers because the Threshold values are all floats
                value = ParameterIds[i].ValueType ? ParameterIds[i].Value2 : (float) ParameterIds[i].Value;

                if ((ParameterIds[i].Threshold > 0.0f && value > ParameterIds[i].Threshold)
                 || (ParameterIds[i].Threshold < 0.0f && value < ParameterIds[i].Threshold)
                 || (ParameterIds[i].ConditionalThreshold && !ParameterIds[i].ConditionalThreshold(ParameterIds)))
                {
                    colorChanged = TRUE;

                    start_color();
                    init_pair(1, COLOR_RED, COLOR_BLACK);
                    attron(COLOR_PAIR(1));

                    PlaySound();
                }
            }

            if (ParameterIds[i].ValueType == Type_Int) //int
                printw("%i", ParameterIds[i].Value);
            else if (ParameterIds[i].ValueType == Type_Float) //float
                printw("%.2f", ParameterIds[i].Value2);
            else if (ParameterIds[i].ValueType == Type_Char) //char
                printw("%c", ParameterIds[i].Value);
            else if (ParameterIds[i].ValueType == Type_String) //string, fuelsys1 is the only string for now
                printw("%s", GetFuelSysStatus(ParameterIds[FUELSYS1].Value));

            if (ParameterIds[i].Unit)
                printw(" %s", ParameterIds[i].Unit);

            if (colorChanged)
                attroff(COLOR_PAIR(1));

            lineLen += pidLen + 2;
        }

        refresh();
        usleep(500000);
    }

    TerminateSound();

    if (Debug)
        close(LogFile);

    return 0;
}

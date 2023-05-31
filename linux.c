#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include <curses.h>
#include <sys/time.h>

#include "atenza.h"


typedef struct _PID
{
    char* Name;
    char* Unit;
    int Type;
    int Value;
    float Value2;
}PID;

typedef enum _PID_INDEX
{
    ECT,
    TFT,
    FAN,
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
    DTC_CNT

} PID_INDEX;


int fd = 0;
int Debug = 0;
int ScreenX = 0;
int ScreenY = 0;

char * removeCharFromStr(char *string, char character);
char * removeSpacesFromStr(char *string);

void Send( char* Command )
{
    write(fd, Command, strlen(Command));
}


void GetCommandResponse( char* Command, char* buff )
{
    Send(Command);
    //read(fd, buffer, 100);
    
    int n=0, ntot=0;
    const int maxLineLength = 1024, max_ntot = 20480;
    char str[maxLineLength + 1], *readpt=str;
     do {
        n = read(fd, readpt, maxLineLength);
        if (n > 0) 
        {
            ntot += n;
            readpt[n] = '\0';
           // printf("%s", readpt);
            readpt += n;
        }
    } while ((n > 0) && (*(readpt-1)!='>') && (ntot<max_ntot) );
    
   // printf("%s\n", readpt);
   if (buff)
   {
      //ncurses doesn't like the CRs
      removeCharFromStr(str, '\r');
      
      strcpy(buff, str);
   }
}


long GetCommandResponseAsLong( char* Command )
{
    char buffer[100];
    
    GetCommandResponse(Command, buffer);
    removeSpacesFromStr(buffer);
    
    return strtol(buffer, NULL, 16);
}


long long GetCommandResponseAsLongLong( char* Command )
{
    char buffer[100];
    
    GetCommandResponse(Command, buffer);
    removeSpacesFromStr(buffer);
    
    return strtoll(buffer, NULL, 16);
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

    GetCommandResponse(Command, buffer);
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


char * removeSpacesFromStr( char *string )
{
	removeCharFromStr(string, ' ');
}


long long current_timestamp() 
{
    struct timeval te;
    long long milliseconds;
     
    gettimeofday(&te, NULL);
    milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    
    return milliseconds;
}


void StatusPrint( char* Message )
{
    move(ScreenY-1, 0);
    clrtoeol();
    printw("%s", Message);
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


int main()
{
    fd = open("/dev/ttyUSB0", O_RDWR);
        
    if (!fd)
    {
        printf("what are you doing bruv?\n");
        return 0;
    }
    
    // Create new termios struct, we call it 'tty' for convention
    struct termios tty;

    // Read in existing settings, and handle any error
    if(tcgetattr(fd, &tty) != 0) {
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
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 115200
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty settings, also checking for error
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    initscr();
    
    // Reboot device to flush any bad settings
    Send("ATWS\r"); //soft reset
    //Send("ATZ\r"); //hard reset
    sleep(1);
    
    Echo("ATL1\r"); // turn on line feed
    Echo("ATE1\r");//Echo on
    
    Echo("ATI\r"); //identify
    Echo("STI\r"); //Print firmware ID string
    Echo("STDI\r"); //Print device hardware ID string
    Echo("ATDP\r"); //describe current protocol
    Echo("ATRV\r"); //read voltage
    
    GetCommandResponse("ATSH7E0\r", 0); // set the header of transmitted OBD messages
    GetCommandResponse("ATL0\r", 0); // turn off line feed
    GetCommandResponse("ATE0\r", 0); //Echo off
    
    int max_x, max_y;
    int currentEngineState, previousEngineState;
    int virtualColumns;
    int fullPressure;
    int releasePressure;
    int awaitingFullPressure;
    char buffer[100];
    long long start, delta;
    int timeout, timeoutValue;
    int manualFanControl, tempHi, tempLo;
    
    PID ParameterIds[] = {
        {"ECT","°C",0,0},
        {"TFT","°C",0,0},
        {"FAN",0,0,0},
        {"BOO",0,0,0},
        {"TOPS",0,0,0},
        {"RPM",0,0,0},
        {"TSS",0,0,0},
        {"LPS","A",1,0},
        {"TR",0,2,0},
        {"TP","%",1,0},
        {"DR",0,1,0},
        {"LTFT",0,1,0},
        {"STFT",0,1,0},
        {"MAF","g/s",1,0},
        {"FSS", 0,3,0},
        {"DTCs",0,0,0}
    };
    
    ScreenX = ScreenY = 0;
    currentEngineState = previousEngineState = 0;
    fullPressure = releasePressure = awaitingFullPressure = 0;
    start = delta = timeout = timeoutValue = 0;
    manualFanControl = 0;
    tempHi = 95;
    tempLo = 90;
    
    initscr(); //init ncurses
    getyx(stdscr, ScreenY, ScreenX); //backup cursor position
    ScreenY += 2; //two lines from last echo
    ScreenX = 0; //first column
    
    getmaxyx(stdscr, max_y, max_x); //get screen size;
    virtualColumns = max_x / 10; //avg PID width is 10 chars
    //printw("%i", virtualColumns);
    
    while (1)
    {
    	ParameterIds[BOO].Value = GetBrakeSwitchState();
        ParameterIds[FAN].Value = GetFanState();    
        ParameterIds[ECT].Value = GetEngineCoolantTemperature();
        ParameterIds[TFT].Value = GetTransmissionFluidTemperature();
        ParameterIds[OP_SW_B].Value = GetTransmissionOilPressureSwitchState();
        ParameterIds[RPM].Value = GetEngineSpeed();
        ParameterIds[TSS].Value = GetTransmissionTurbineShaftSpeed();
        ParameterIds[LPS].Value2 = GetTransmissionLinePressureSolenoidAmperage();
        ParameterIds[TR].Value = GetTransmissionRangeSensorPosition();
        ParameterIds[THOP].Value2 = GetThrottlePosition();
        ParameterIds[LONGFT1].Value2 = GetLongTermFuelTrim();
        ParameterIds[SHRTFT1].Value2 = GetShortTermFuelTrim();
        ParameterIds[MAF].Value2 = GetIntakeAirMassFlowRate();
        ParameterIds[FUELSYS1].Value = GetFuelSystemStatus();
        ParameterIds[DTC_CNT].Value = GetDiagnosticTroubleCodeCount();
        
        //Calculated values
        if (ParameterIds[RPM].Value && ParameterIds[TSS].Value)
            ParameterIds[DR].Value2 = (float) ParameterIds[TSS].Value / (float) ParameterIds[RPM].Value;
        else
            ParameterIds[DR].Value2 = 0.0;
                   
        currentEngineState = (ParameterIds[RPM].Value > 0) ? 1 : 0;
        
        if (currentEngineState == 1 && previousEngineState == 0)
        {
            //Code in this block runs once every engine restart
            
            manualFanControl = 0;
        }
        
        previousEngineState = currentEngineState;
        
        //clear DTCs
        if (Debug && ParameterIds[DTC_CNT].Value == 1)
        {
            StatusPrint("Clearing DTCs..");
    	    GetCommandResponse("14FF00\r", 0); //Clear DTCs
        }
        
        if ((ParameterIds[ECT].Value > tempHi || ParameterIds[TFT].Value > tempHi) && !ParameterIds[FAN].Value)
        {
        	manualFanControl = 1;
        	
        	StatusPrint("turning on FAN..");
        	
        	AuthenticateSession();    
            SetFanState(1);
        }
        else if (manualFanControl)
        {   
            if (ParameterIds[TR].Value == 'P')
            {
                tempHi = 90;
                tempLo = 85;
            }
            else
            {
                tempHi = 95;
                tempLo = 90;
            }
            
            if (ParameterIds[ECT].Value < tempLo && ParameterIds[TFT].Value < tempLo && ParameterIds[FAN].Value)
            {
                StatusPrint("turning off FAN..");
                SetFanState(0);
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
            
            awaitingFullPressure = 1;
            temp = ParameterIds[TFT].Value;
            
            //For every 10°C increase in temp subtract 40000 ms
            //Point1: 180000 ms (3.0 min) at 30°C
            //Point2: 30000 ms (0.5 min) at 75°C
            slope = -3333.33f;
            intercept = 280000.0f;
            
            time = ((float)temp * slope) + intercept;
            timeoutValue = (int) time;
            
            //set to 30s minimum
            if (timeoutValue < 30000)
                timeoutValue = 30000;
        }
        
        if (awaitingFullPressure && ParameterIds[THOP].Value2 == 0.0)
        {
            char buffer[50];
            float timeoutInMinutes;
            
            timeoutInMinutes = (float) timeoutValue / (float) 60000;
            
            snprintf(buffer, 50, "Setting LPS to full pressure for %.1f mins", timeoutInMinutes);
            StatusPrint(buffer);
            
            fullPressure = 1;
            awaitingFullPressure = 0;
            
            AuthenticateSession();
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
                timeout = 1;
            }
        }
        
        if ((releasePressure && ParameterIds[TR].Value != 'D') || timeout)
        {
            //Return LPS control to ECU
            StopExtendedDiagnosticSession();
            
            releasePressure = fullPressure = 0;
            start = delta = timeout = 0;
            
            //move(y-1, 0);
            //clrtoeol();
            //printw("Returning LPS control to ECU..");
            StatusPrint("Returning LPS control to ECU..");
        }
        
        move(ScreenY, ScreenX); //restore cursor pos
        clrtobot(); //clear line
        refresh();
            
        int j = 1;
        
        for (int i = 0; i < sizeof(ParameterIds)/sizeof(ParameterIds[0]); i++)
        {
            printw("%s: ", ParameterIds[i].Name);
            
            if (ParameterIds[i].Type == 0) //int
                printw("%i", ParameterIds[i].Value);
            else if (ParameterIds[i].Type == 1) //float
                printw("%.2f", ParameterIds[i].Value2);
            else if (ParameterIds[i].Type == 2) //char
                printw("%c", ParameterIds[i].Value);
            else if (ParameterIds[i].Type == 3) //string
                printw("%s", GetFuelSysStatus(ParameterIds[FUELSYS1].Value)); //fuelsys1 is the only string for now
            
            if (ParameterIds[i].Unit)
                printw(" %s", ParameterIds[i].Unit);
            
            if (j == virtualColumns)
            {
                printw("\n");
                j = 1;
            }
            else
            {
                printw("  ");
                j++;
            }
        }
        
        refresh();
        usleep(500000);
    }    
    
    return 0;
}

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
    THOP

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
    int timeout;
    
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
        {"TP","%",1,0}
    };
    
    ScreenX = ScreenY = 0;
    currentEngineState = previousEngineState = 0;
    fullPressure = releasePressure = awaitingFullPressure = 0;
    start = delta = timeout = 0;
    
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
        ParameterIds[RPM].Value = GetEngineRevolutionsPerMinute();
        ParameterIds[TSS].Value = GetTransmissionTurbineShaftSpeed();
        ParameterIds[LPS].Value2 = GetTransmissionLinePressureSolenoidAmperage();
        ParameterIds[TR].Value = GetTransmissionRangeSensorPosition();
        ParameterIds[THOP].Value2 = GetThrottlePosition();
        
        currentEngineState = (ParameterIds[RPM].Value > 0) ? 1 : 0;
        
        if (currentEngineState == 1 && previousEngineState == 0)
        {
        	//clear DTCs
            if (Debug)
            {
                //move(y-1, 0);
        	    //printw("Clearing DTCs..");
        	    StatusPrint("Clearing DTCs..");
    	        GetCommandResponse("14FF00\r", 0); //Clear DTCs
            }
        }
        
        previousEngineState = currentEngineState;
        
        if ((ParameterIds[ECT].Value > 90 || ParameterIds[TFT].Value > 90) && !ParameterIds[FAN].Value)
        {
        	//move(y-1, 0);
        	//printw("turning on FAN..");
        	StatusPrint("turning on FAN..");
        	
        	AuthenticateSession();    
            SetFanState(1);
        }
        
        if (Debug
            && !fullPressure
            && !awaitingFullPressure
            && ParameterIds[TR].Value == 'N'
            && ParameterIds[BOO].Value == 1 
            && ParameterIds[THOP].Value2 > 0.5)
        {
            awaitingFullPressure = 1;
            
            //move(y-1, 0);
            //printw("Release throttle to commit LPS..");
            StatusPrint("Release throttle to commit LPS..");
        }
        
        if (awaitingFullPressure && ParameterIds[THOP].Value2 == 0.0)
        {
            //move(y-1, 0);
            //clrtoeol();
            //printw("Setting LPS to full pressure..");
            StatusPrint("Setting LPS to full pressure..");
            
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
            
            if (delta > 240000)
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
            if (ParameterIds[i].Type == 0) //int
                printw("%s: %i", ParameterIds[i].Name, ParameterIds[i].Value);
            else if (ParameterIds[i].Type == 1) //float
                printw("%s: %.2f", ParameterIds[i].Name, ParameterIds[i].Value2);
            else if (ParameterIds[i].Type == 2)
                printw("%s: %c", ParameterIds[i].Name, ParameterIds[i].Value);
            
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

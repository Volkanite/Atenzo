#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include <curses.h>

#include "atenza.h"


typedef struct _PID
{
    char* Name;
    char* Unit;
    int Value;
}PID;

typedef enum _PID_INDEX
{
    ECT,
    TFT,
    FAN,
    BOO,
    OP_SW_B

} PID_INDEX;


int fd = 0;

int GetKeyFromSeed(char* Seed);
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
    
    Echo("ATSH7E0\r"); // set the header of transmitted OBD messages
    
    GetCommandResponse("ATL0\r", 0); // turn off line feed
    GetCommandResponse("ATE0\r", 0); //Echo off
    
    //int brake, ect, tft, fanOn, tops;
    int x, y;
    int max_x, max_y;
    int virtualColumns;
    char buffer[100];
    
    PID ParameterIds[] = {
        {"ECT","°C",0},
        {"TFT","°C",0},
        {"FAN",0,0},
        {"BOO",0,0},
        {"TOPS",0,0}
    };
    
    x = y = 0;
    
    initscr(); //init ncurses
    getyx(stdscr, y, x); //backup cursor position
    y += 2; //two lines from last echo
    x = 0; //first column
    
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
        
        if ((ParameterIds[ECT].Value > 90 || ParameterIds[TFT].Value > 90) && !ParameterIds[FAN].Value)
        {
        	move(y-1, 0);
        	printw("turning on FAN..");
        	
            GetCommandResponse("1087\r", 0); //tester present
            
            //Authenticate
            GetCommandResponse("2701\r", buffer);
            removeSpacesFromStr(buffer);
            //printw("%s\n", buffer);
    
            int key = GetKeyFromSeed(buffer+4);
            
            char newbuff[100];
            snprintf(newbuff, 100, "2702%X\r", key);
            //printw("%s\n",newbuff);
            GetCommandResponse(newbuff, 0); //send key
    
            GetCommandResponse("2F17C40701\r", 0); //set fan 1 ON
            GetCommandResponse("221103\r", 0); //second read actually turns on fan
        }
        
        move(y, x); //restore cursor pos
        clrtobot(); //clear line
        refresh();
            
        int j = 1;
        
        for (int i = 0; i < sizeof(ParameterIds)/sizeof(ParameterIds[0]); i++)
        {
            printw("%s: %i", ParameterIds[i].Name, ParameterIds[i].Value);
            
            if (ParameterIds[i].Unit)
                printw("%s", ParameterIds[i].Unit);
            
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
        sleep(1);
    }    
    
    return 0;
}

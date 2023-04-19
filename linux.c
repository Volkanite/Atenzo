#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>


int fd = 0;

int GetKeyFromSeed(char* Seed);
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
            printf("%s", readpt);
            readpt += n;
        }
    } while ((n > 0) && (*(readpt-1)!='>') && (ntot<max_ntot) );
    
   // printf("%s\n", readpt);
   if (buff)
   {
      strcpy(buff, str+12);
      removeSpacesFromStr(buff);
   }
}


void Echo(char* Command)
{
    GetCommandResponse(Command, 0);
}


// Funtion removing spaces from string
char * removeSpacesFromStr(char *string)
{
    // non_space_count to keep the frequency of non space characters
    int non_space_count = 0;
 
    //Traverse a string and if it is non space character then, place it at index non_space_count
    for (int i = 0; string[i] != '\0'; i++)
    {
        if (string[i] != ' ')
        {
            string[non_space_count] = string[i];
            non_space_count++;//non_space_count incremented
        }    
    }
    
    //Finally placing final character at the string end
    string[non_space_count] = '\0';
    return string;
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

    
    // Reboot device to flush any bad settings
    Send("ATWS\r"); //soft reset
    //Send("ATZ\r"); //hard reset
    sleep(1);
    
    Echo("ATL1\r"); // turn on line feed
    //Echo("ATE0\r");//Echo off
    Echo("ATE1\r");//Echo on
    
    Echo("ATI\r"); //identify
    Echo("STI\r"); //Print firmware ID string
    Echo("STDI\r"); //Print device hardware ID string
    Echo("ATDP\r"); //describe current protocol
    Echo("ATRV\r"); //read voltage
    
    Echo("ATSH7E0\r"); // set the header of transmitted OBD messages
    
    //# message to check status of Brakes(last byte in response : 00 - brake is off, 01/02 - brake is on)
    //BOO = Brake On/Off Switch
    //Echo("221101\r");
    
    int temp;
    int fanOn = 0;
    char smallbuff[100];
    
    while (1)
    {
        Echo("221103\r"); //fan
        GetCommandResponse("220005\r", smallbuff); //ECT
        
        temp = strtol(smallbuff, NULL, 16);
        temp = (temp & 0x000000ff); //last byte
        temp -= 40; //Celsius
        
        if (temp > 90 && !fanOn)
        {
            fanOn = 1;
            
            Echo("1087\r");
            GetCommandResponse("2701\r", smallbuff);
            printf("%s\n", smallbuff);
    
            int key = GetKeyFromSeed(smallbuff);
            
            char newbuff[100];
            snprintf(newbuff, 100, "2702%X\r", key);
            printf("%s\n",newbuff);
            Echo(newbuff);
    
            //Echo("221103\r"); //fan pid
            Echo("2F17C40701\r"); //set fan 1 ON
            Echo("221103\r"); //second read actually turns on fan
        }
        
        printf("\nECT: %i °C\n", temp);
        sleep(1);
    }    
    
    return 0;
}

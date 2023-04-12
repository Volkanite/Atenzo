#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>


int fd = 0;


void Send( char* Command )
{
    write(fd, Command, strlen(Command));
}


void Echo( char* Command )
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
    
    printf("%s\n", readpt);
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
    Send("ATWS\r");
    //Send("ATZ\r");
    sleep(1);
    
    Echo("ATL1\r"); // turn on line feed
    //Echo("ATE0\r");//Echo off
    Echo("ATE1\r");//Echo on
    
    Echo("ATI\r"); //identify
    Echo("STI\r"); //Print firmware ID string
    Echo("STDI\r"); //Print device hardware ID string
    Echo("ATDP\r"); //describe current protocol
    Echo("ATRV\r"); //read voltage
    
    /*Set current protocol preset to ISO 15765, 11-bit Tx, 125kbps, DLC=8 .
    Medium Speed CAN (MS-CAN) is a dual-wire transceiver typically connected 
    to pins 3 and 11 of the OBD port(Ford MSC network).*/
    Send("STP53\r");

    Send("ATCFC1\r"); //can flow control on
    Send("STCSEGR1\r"); //turn CAN Rx segmentation on
    Send("STCSEGT1\r"); // turn CAN Tx segmentation on
    Send("ATSH7E0\r"); // set the header of transmitted OBD messages
    Send("ATCRA7E8\r"); // set CAN hardware filter
    Send("ATCEAAE\r"); //use CAN extended address hh
    Send("ATTAAE\r"); //set tester address to hh
    Send("STCFCPC\r");     //# clear all flow control address pairs
    Send("STCFCPA 7E0 AE, 7E8 AE\r");   //# add flow control address pair
    
    //# message to check status of Brakes(last byte in response : 00 - brake is off, 01/02 - brake is on)
    //BOO = Brake On/Off Switch
    Echo("221101\r");

    /*while (0)
    {
        Echo("221101\r");
        Sleep(1000);
    }*/
    
    
    /*Send("27011\r"); //SecurityAccess ;)
    int seed = GetSeed();
    
    //Reply with key
    int key = GetKeyFromSeed(seed);
    //BuildKeyMessage(buffer, key);
    //Echo(buffer); //send key*/
    
    
    return 0;
}

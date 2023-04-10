#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


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
	
	// Reboot device to flush any bad settings
    //Send("ATWS\r");
    //Send("ATZ\r");
    
    Echo("ATL1\r"); // turn on line feed
    
	Echo("ATE0\r");//Echo off
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
	
	return 0;
}

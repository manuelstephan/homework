// Manuel Stephan and Ben Paras
// ECE 497 Homework 4
// Program that uses the stepper motor and photo sensors to track the light. If the button 
// on P8_13 is pressed, the stepper motor rotates 360 degrees and remembers the point
// at which the light is the brightest . Usage: ./test 
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include <time.h>
#include <unistd.h> // get usleep ;)  
#include "gpio-utils.h"

 /****************************************************************
 * Constants
 ****************************************************************/
 
#define POLL_TIMEOUT (500) /* .5 seconds */
#define MAX_BUF 64

/****************************************************************
 * Global variables
 ****************************************************************/
int keepgoing = 1;	// Set to 0 when ctrl-c is pressed
int ledport[4];
int avgANval[20];
int brightPos = 0;
int AINread1, AINread2;
/****************************************************************
 * signal_handler
 ****************************************************************/
void signal_handler(int sig);
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepgoing = 0;
}




/****************************************************************
 * Analog input handling
 ****************************************************************/
int AINread(char *AIN)
{
	FILE *fp;
	char AINpath[MAX_BUF];
	char AINval[MAX_BUF];
	
    	snprintf(AINpath, sizeof AINpath, "/sys/devices/ocp.2/helper.14/%s", AIN);

	if((fp = fopen(AINpath, "r")) == NULL)
	{
		printf("Error, can't read pin, %s\n", AIN);
		return 1;
	}

	fgets(AINval, MAX_BUF, fp);
	fclose(fp);
	return atoi(AINval);
}

// functions for the stepper motor 

/****************************************************************
 * Sequence Calculator
 ****************************************************************/
// calculate the next sequence of the stepper motor ... 
// dir = direction, 1 is clockwise, 0 is counterclockwise 
unsigned int calcNextSeq(int dir, unsigned int seq)// dir = direction, 1 is clockwise, 0 is counterclockwise 
{
	if(dir == 1)
	{
		if((seq) == 12)
		{
			(seq) = 9;
		}
		else if( (seq) == 9 )
		{
			(seq) = 3;
		}
		else
		{
			(seq) = (seq) * 2;	
		}
	}

	else
	{
		if( (seq) == 3 )
		{
			(seq) = 9;
		}
		else if ( (seq) == 9 )
		{
			(seq) = 12;
		}
		else
		{
			(seq) = (seq) / 2;
		}
	}

return seq;
}


/****************************************************************
 * drive inputs with sequence function
 ****************************************************************/
void driveTheOutputs(unsigned int seq)
{
	unsigned int i;

	for(i = 0; i < 4; i++)
	{
		unsigned int mask = 0x1;
		mask = (mask << i);
		if((seq & mask))
		{
			gpio_set_value(ledport[i],1);
		}		
		else
		{
			gpio_set_value(ledport[i],0);
		}
	} 
	return;
} 

// turns the stepper motor (times) times
// direction means: 1 = Clockwise, 0 = Counterclockwise
void turnTimes(int times, int dir, unsigned int *seq, int val[])
{
	int i;
	int minVal = val[0];
	for(i = 0; i < (times*20); i++)
	 	{
		(*seq) = calcNextSeq(dir,(*seq));
		driveTheOutputs(*seq);	
		// read twice to get current value
		AINread1 = AINread("AIN0");
		usleep(100);	
		AINread1 = AINread("AIN0");
		AINread2 = AINread("AIN1");
		usleep(100);	
		AINread2 = AINread("AIN1");
		val[(i%20)] = (AINread1 + AINread2) / 2 ;

		if( val[(i%20)] < minVal)
			{
				minVal = val[(i%20)];
				brightPos = (i%20);
			}
	
		usleep(50000);//yes its the time in seconds ...  
	
		}

}

/****************************************************************
 * Light tracking functions

 ****************************************************************/
// tracking the led light source
void trackLight( unsigned int *seq)
{
	int dir = 0;

	// read twice to get current value
	AINread1 = AINread("AIN0");
	usleep(100);	
	AINread1 = AINread("AIN0");

	AINread2 = AINread("AIN1");
	usleep(100);	
	AINread2 = AINread("AIN1");
  
	if ( AINread1 <= AINread2 )
	{	
		dir = 0;
	}
	else 
	{	
		dir = 1;
	}

	printf("AIN1= %d AIN2 = %d \n", AINread1, AINread2);
	(*seq) = calcNextSeq(dir, *seq);
	driveTheOutputs( *seq);
	usleep(50000);//yes its the time in seconds ...  	
}


//go back to brightest point
void turnToLight(unsigned int *seq)
{
	int i;
	for(i = 0; i < (20 - brightPos ); i++)
	 {
		(*seq) = calcNextSeq(0,(*seq));
		driveTheOutputs(*seq);	
		usleep(50000);//microseconds 
	
	}

}


/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{
	struct pollfd fdset[5]; // modified to 3, 2 previously ... 
	int nfds = 5; // how many different gpiointerrupts do you have, thats nfds ... 
	int gpio_fd[4], timeout, rc;
	char buf[MAX_BUF];
	unsigned int gpio;
	int len;

	int i; // counter variable ... 

	ledport[0] = 60; // gpios of leds 
	ledport[1] = 48; // gpios of leds 
	ledport[2] = 49; // gpios of leds 
	ledport[3] = 30; // gpios of leds 
	int buttonport[4] = {23,7,26,51}; //gpios of buttons  
	int toggle[4] = {0}; // array used to toggle leds ... 
	unsigned int seq = 0x3;

	signal(SIGINT, signal_handler);

	system("echo 'welcome to the led interrupt program, press hw button to toggle led'");


	// access led
	for(i=0;i<4;i++)
	{
		gpio_export(ledport[i]);
		gpio_set_dir(ledport[i],"out");
		gpio_set_value(ledport[i],0);
	}
	
	

	for(i=0;i<4;i++)
	{
		gpio_export(buttonport[i]);
		gpio_set_dir(buttonport[i], "in");
		gpio_set_edge(buttonport[i], "both");  // Can be rising, falling or both
		gpio_fd[i] = gpio_fd_open(buttonport[i], O_RDONLY);
	}

	timeout = POLL_TIMEOUT;
  
	while (keepgoing) 
		{
			memset((void*)fdset, 0, sizeof(fdset));
			
			fdset[0].fd = STDIN_FILENO;
			fdset[0].events = POLLIN;

			for(i=1;i<5;i++)
			{
				fdset[i].fd = gpio_fd[(i-1)];
				fdset[i].events = POLLPRI;
			}

			rc = poll(fdset, nfds, timeout);      

			if (rc < 0) 
			{
				printf("\npoll() failed!\n");
				return -1;
			}
	      
			if (rc == 0) 
			{
				//printf(".");
			}

            		for(i=1;i<5;i++)
	    		{
				if (fdset[i].revents & POLLPRI) 
				{
					lseek(fdset[i].fd, 0, SEEK_SET);  // Read from the start of the file
					len = read(fdset[i].fd, buf, MAX_BUF);
					//printf("\npoll() GPIO %d interrupt occurred, value=%c, len=%d\n",
					//	 buttonport[i-1], buf[0], len);

					if(buf[0] == '1')
					{
						turnTimes(1, 1,&seq, avgANval);
		
						printf("Brightest Light at: %d (%d) \n", 
							avgANval[brightPos], brightPos);

						turnToLight( &seq);
						sleep(1);
					}		
	      
				}
		
	    		}

		trackLight( &seq);
	
		if (fdset[0].revents & POLLIN) 
			{
				(void)read(fdset[0].fd, buf, 1);
				printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
			}

		fflush(stdout);
       
	}
	
	return 0;
        
}


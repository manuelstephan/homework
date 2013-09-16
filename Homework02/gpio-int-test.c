/* Copyright (c) 2011, RidgeRun
 * All rights reserved.
 *
From https://www.ridgerun.com/developer/wiki/index.php/Gpio-int-test.c

 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the RidgeRun.
 * 4. Neither the name of the RidgeRun nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY RIDGERUN ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL RIDGERUN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// modified by Manuel Stephan 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include "gpio-utils.h"

 /****************************************************************
 * Constants
 ****************************************************************/
 
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

/****************************************************************
 * Global variables
 ****************************************************************/
int keepgoing = 1;	// Set to 0 when ctrl-c is pressed

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
int ledport[4] = {60,48,49,30}; // gpios of leds 
int buttonport[4] = {23,7,26,51}; //gpios of buttons  
int toggle[4] = {0}; // array used to toggle leds ... 

signal(SIGINT, signal_handler);

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
  
	while (keepgoing) {
		memset((void*)fdset, 0, sizeof(fdset));

		
		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;
      
		fdset[1].fd = gpio_fd[0];
		fdset[1].events = POLLPRI;
 
		fdset[2].fd = gpio_fd[1];
		fdset[2].events = POLLPRI;

		fdset[3].fd = gpio_fd[2];
		fdset[3].events = POLLPRI;

		fdset[4].fd = gpio_fd[3];
		fdset[4].events = POLLPRI;

		rc = poll(fdset, nfds, timeout);      

		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}
      
		if (rc == 0) {
			printf(".");
		}
            for(i=1;i<5;i++)
	    {
		if (fdset[i].revents & POLLPRI) {
			lseek(fdset[i].fd, 0, SEEK_SET);  // Read from the start of the file
			len = read(fdset[i].fd, buf, MAX_BUF);
			printf("\npoll() GPIO %d interrupt occurred, value=%c, len=%d\n",
				 buttonport[i-1], buf[0], len);
		if(buf[0] == '1')
		toggle[i-1] = !toggle[i-1];
		}
	    }
	for(i=0;i<4;i++) {      
		if(toggle[i])
		gpio_set_value(ledport[i],1);
		else
		gpio_set_value(ledport[i],0);
	}
		
		if (fdset[0].revents & POLLIN) {
			(void)read(fdset[0].fd, buf, 1);
			printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
		}

		fflush(stdout);
       
	}
	for(i=0;i<4;i++){
	gpio_fd_close(ledport[i]);
	gpio_fd_close(buttonport[i]);
	}
	
	return 0;
        
}


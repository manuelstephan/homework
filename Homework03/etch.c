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
// modified by Manuel Stephan to perform etch a sketch
// now on the led matrix ... 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include "gpio-utils.h"
#include "i2c-dev.h"
#include "i2cbusses.h"

 /****************************************************************
 * Constants
 ****************************************************************/
 
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64
#define BICOLOR		// undef if using a single color display

const int MATDIM1 = 8; // y dimension
const int MATDIM2 = 8; // x dimension

/****************************************************************
 * Global variables
 ****************************************************************/
int keepgoing = 1;	// Set to 0 when ctrl-c is pressed
static __u16 tm[8]={0};  // array of 8 __u16, representing the matrixdata
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

static void help(void) __attribute__ ((noreturn));

static void help(void) {
	fprintf(stderr, "Usage: matrixLEDi2c (hardwired to bus 3, address 0x70)\n");
	exit(1);
}

static int check_funcs(int file) {
	unsigned long funcs;

	/* check adapter functionality */
	if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
		fprintf(stderr, "Error: Could not get the adapter "
			"functionality matrix: %s\n", strerror(errno));
		return -1;
	}

	if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE)) {
		fprintf(stderr, MISSING_FUNC_FMT, "SMBus send byte");
		return -1;
	}
	return 0;
}

// Writes block of data to the display
static int write_block(int file, __u16 *data) {
	int res;
#ifdef BICOLOR
	res = i2c_smbus_write_i2c_block_data(file, 0x00, 16, 
		(__u8 *)data);
	return res;
#else
/*
 * For some reason the single color display is rotated one column, 
 * so pre-unrotate the data.
 */
	int i;
	__u16 block[I2C_SMBUS_BLOCK_MAX];
//	printf("rotating\n");
	for(i=0; i<8; i++) {
		block[i] = (data[i]&0xfe) >> 1 | 
			   (data[i]&0x01) << 7;
	}
	res = i2c_smbus_write_i2c_block_data(file, 0x00, 16, 
		(__u8 *)block);
	return res;
#endif
}


/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{

 int i,j,xpos=0,ypos=0,running=1;
 char c,dummy;
 int matrix[MATDIM1][MATDIM2];

 int res, i2cbus, address, file;
 char filename[20];
 int force = 0;

// init the matrix
for( i=0; i< MATDIM1; i++)
 for( j=0; j< MATDIM2; j++)
 { 
  matrix[i][j]=0;
 } 
	struct pollfd fdset[5]; // modified to 3, 2 previously ... 
	int nfds = 5; // how many different gpiointerrupts do you have, thats nfds ... 
	int gpio_fd[4], timeout, rc;
	char buf[MAX_BUF];
	int len;


int buttonport[4] = {23,7,26,51}; //gpios of buttons  


signal(SIGINT, signal_handler);

system("echo 'Welcome to a basic etch-a-sketch program!'");
system("echo '========================================='");
	
// export buttonIOs, set direction to in and set edges
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

		for(i=1;i<5;i++){
		fdset[i].fd = gpio_fd[(i-1)];
		fdset[i].events = POLLPRI;
		}

		rc = poll(fdset, nfds, timeout);      

		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}
      
		
            for(i=1;i<5;i++)
	    {
		if (fdset[i].revents & POLLPRI) {
			lseek(fdset[i].fd, 0, SEEK_SET);  // Read from the start of the file
			len = read(fdset[i].fd, buf, MAX_BUF);
			//printf("\npoll() GPIO %d interrupt occurred, value=%c, len=%d\n",buttonport[i-1], buf[0], len);
		if((buf[0] == '1')&&(xpos < (MATDIM2-1))&&(i==1))
			xpos++;
		if((buf[0] == '1')&&(xpos > 0)&&(i==2))
			xpos--;
		if((buf[0] == '1')&&(ypos > 0)&&(i==3))
			ypos--;
		if((buf[0] == '1')&&(ypos < (MATDIM1-1))&&(i==4))
			ypos++;
		
		}
	    }
	matrix[ypos][xpos]=1;
        system("echo '\n'");
	for( i=0; i< MATDIM1; i++)
	 for( j=0; j< MATDIM2; j++)
	 { 
	  printf("%i  ",matrix[i][j]);
	  if(j==(MATDIM2-1))
	  printf("\n");
	 } 
 // Matrixdriver for the display ... 
	for(i=0;i<8;i++)
 		for(j=0;j<8;j++){
	printf("%i", matrix[i][j]);
	if(j==7)
	printf("\n");
	if(matrix[i][j]==1)
	tm[j]=(tm[j]|(1<<(7-i)));
	}
i2cbus = lookup_i2c_bus("1");
	printf("i2cbus = %d\n", i2cbus);
	if (i2cbus < 0)
		help();

	address = parse_i2c_address("0x70");
	printf("address = 0x%2x\n", address);
	if (address < 0)
		help();

	file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
//	printf("file = %d\n", file);
	if (file < 0
	 || check_funcs(file)
	 || set_slave_addr(file, address, force))
		exit(1);

	// Check the return value on these if there is trouble
	i2c_smbus_write_byte(file, 0x21); // Start oscillator (p10)
	i2c_smbus_write_byte(file, 0x81); // Disp on, blink off (p11)
	i2c_smbus_write_byte(file, 0xe7); // Full brightness (page 15)

	write_block(file, tm);

		
		if (fdset[0].revents & POLLIN) {
			(void)read(fdset[0].fd, buf, 1);
			printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
		}

		fflush(stdout);
       
	}
	
	
	return 0;
        
}


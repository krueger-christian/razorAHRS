#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <fcntl.h>
#include <termios.h>

#include <string.h> // needed for memset

#include <sys/time.h>

#include "config.h"
#include "razorTools.h"


/*********************************************
 ***          USER SELECTION AREA          ***
*********************************************/

	/*  => select out of {single, continous}
	 *
	 *  tracker is readable in two different 
	 *  streaming modes:
	 *  1. single mode: 
	 *     the tracker sends only a frame 
	 *     after request
	 *  2. continous mode: 
	 *     the tracker sends permanently data
	 *     (every 20ms one frame, 12 byte) */
	strMode mode = continous;

	/* time limit (milliseconds) to wait 
	 * during synchronization phase */
	int connect_timeout_ms = 1000;

	/* disable/enable the output of program 
	 * status reports on the console */
	bool messageOn = true;

/*********************************************
 ***       END OF USER SELECTION AREA      ***
*********************************************/


	/* union structure to store blocks of 
	 *  4 Byte received by the tracker (equal 
	 *  to the size of a single float value.
	 *  for further information look at 
	 *  razorTools.h */
	razorBuffer floatBuffer;


	/* array that stores the sensor data
	 *
	 * values[0] = Yaw
	 * values[1] = Pitch
	 * values[2] = Roll    */
	float values[3];

	bool synchronized = false;

/*-----------------------------------------------------------------*/

/* INITIALIZING THE TRACKER
 *
 * The function...
 * ... checks, if tracker is available
 * ... activates continous binary streaming mode
 * ... ensures that receiving and sending is synchronous
 * ... returns true, if tracker is synchronized
 * ... returns false, if connection or synchronization failed */
bool initRazor(int tty_fd){
	
	// Buffer to store one byte
	char input = 'D';

	/* We know the length of the expected token from the arduino code (1)
	 * Token is: "#SYNCH00\r\n*/	
	int token_length = 10;

	// Get some space to store the token we receive after requesting
	char* token;
	token = calloc(token_length, sizeof(char));
	
	// We compare the received token with our following reference
	char token_reference[10];
	strcpy(token_reference, "#SYNCH00\r\n"); 

	// Trying to connect and switch to binary mode during loop
	while(1){
		if(messageOn) printf("\n  – LOOKING FOR TRACKER: ");
		if (read(tty_fd,&input,1)>0){
			if(messageOn){
				printf("__okay.\n\r");
				printf("\n  – SWITCH TO BINARY OUTPUT MODE.\n\r");
			}
	
			write(tty_fd,"#o1",3); // output continous stream
			write(tty_fd,"#ob",3); // output binary
			break;
		}
	}

	/* kind of flushing the input.
	 * the tracker started in textmode and sends strings/frames of the format:
	 * "YPR=<yaw-value>,<pitch-value2>,<roll-value3>\r\n",
	 * after receiving '\n' we are sure that with the next byte we will 
	 * receive the start of a new frame */	
	while(input != '\n'){
		read(tty_fd,&input,1);
	}

	// send request for synchronization token
	if(messageOn) printf("\n  – SYNCHRONIZING: ");
	write(tty_fd,"#s00",4);

	/* wait a little bit to get sure that the
	 * tracker read the send signals */
	razorSleep(200);

	/* variables to store time values
	 * used to measure how long 
	 * synchronization takes */
	struct timeval t0, t1, t2;
	gettimeofday(&t0, NULL);
	t1 = t0;

	// make sure, control variable is set to 0
	size_t token_pos = 0;

	/* SYNCHRONIZATION
	 * Looking for correct token. */
	while(1){

		// check if input available...
		if (read(tty_fd,&input,1)>0){

			/* ...is the first byte equal to the first
			 * character of the reference token */
			if(input == '#'){
				token_pos++;
				
				token[0] = input;
				
				// ... first byte matchs, so get the next bytes
				while(token_pos < token_length){
					if (read(tty_fd,&input,1)>0){
						token[token_pos] = input;
						token_pos++;
					}					
				}
				
				/* if received token is equal to the reference token
				 * the variable synchronized is set to true */	
				synchronized = stringCompare(token, token_reference, token_length);
				
				if(synchronized == true){
					if(messageOn) printf("__okay.\n\r");
						free(token);					
						return true;
				}
			}
		}

		gettimeofday(&t2, NULL);
		if (elapsed_ms(t1, t2) > 200) {
			// 200ms elapsed since last request and no answer -> request synch again
			// (this happens when DTR is connected and Razor resets on connect)
			write(tty_fd,"#s00",4);
			t1 = t2;
		}
		// check if time out is reached
		if (elapsed_ms(t0, t2) > connect_timeout_ms) {
			if(messageOn) printf("___failed.\n\r"); // TIME OUT!		
				free(token);			
				return false;
		}

		token_pos = 0;	

	}

}

/*-----------------------------------------------------------------*/

int readContinously(int tty_fd){

	// Buffer to store user input from console
	unsigned char console = 'D';
	char singleByte = 'D';

	size_t values_pos = 0;
	write(tty_fd,"#ob",3); // just to ensure binary output
	
	bool printData;
	if(messageOn) printData = false;
	else printData = true;
	bool stopRead = false;

	if(messageOn){
		printf("_________________________________________________\r\n");
		printf("\n  !\n\r    PRESS SPACEBAR TO START/STOP SHOWING DATA\n\n\n\r");
	}

	int result;
	int bufferInput = 0;
	while (!stopRead) {

		result = read(tty_fd,&singleByte,1);

       	if (result == 1) {

			floatBuffer.ch[bufferInput] = singleByte;
			bufferInput++;

				if(bufferInput == 4){
					values[values_pos] = floatBuffer.f;
					values_pos++;
					bufferInput = 0;
				}		
		}

		// if new data is available on the serial port, print it out
		if(values_pos == 3){
			if(printData)printf("YAW = %.1f \t PITCH = %.1f \t ROLL = %.1f \r\n", values[0], values[1], values[2]);
			values_pos = 0;
			razorSleep(20);
		}

		// if new data is available on the console, send it to the serial port
        if (read(STDIN_FILENO,&console,1)>0){  
			if ((printData == false) && (console == ' ')) printData = true;
			else stopRead = true;
		}
	}

	write(tty_fd,"#ot",3);

	return 0;	
}


/*-----------------------------------------------------------------*/

bool readingRazor(int tty_fd){

	if(initRazor(tty_fd) == false) return false;
	
	readContinously(tty_fd);

/*	if(mode == continous){
		readContinously(tty_fd);
	}
	else if(mode == single) {
		readSingle(tty_fd);
	}
	else{
		printf("no streaming mode selected!");
		return 1;
	}*/

	return true;		
}

/*-----------------------------------------------------------------*/

int razorAHRS( speed_t baudRate, char* port){

        struct termios old_tio;
		struct termios old_stdio;
        int tty_fd;

		//saving current termios configurations of STDOUT_FILENO
		if (tcgetattr(STDOUT_FILENO, &old_stdio) != 0) {
        	printf("tcgetattr(fd, &old_stdtio) failed\n");
        	return 1;
    	}

		stdio_Config();

        tty_fd=open(port, O_RDWR | O_NONBLOCK);      

		//saving current termios configurations of tty_fd
		if (tcgetattr(tty_fd, &old_tio) != 0) {
        	printf("tcgetattr(fd, &old_tio) failed\n");
        	return 1;
    	}		

		tio_Config(tty_fd, baudRate);

		readingRazor(tty_fd);


		// reactivating previous configurations of tty_fd and the STDOUT_FIELNO
		tcsetattr(tty_fd, TCSANOW, &old_tio);
		tcsetattr(STDOUT_FILENO, TCSANOW, &old_stdio);
        close(tty_fd);
		
		return 0;	
}

/*-----------------------------------------------------------------*/

int main(int argc,char** argv)
{
	printf("_________________________________________________\r\n");
	printf("\n    RAZOR AHRS – Headtracker Reader\n");
	printf("_________________________________________________\r\n\n");
	
	razorAHRS(B57600, "/dev/pts/3"); //"/dev/ttyUSB0");

	printf("\n_________________________________________________\r\n");

	return 0;
}

// https://tty1.net/blog/2009/linux-serial-programming-example_en.html

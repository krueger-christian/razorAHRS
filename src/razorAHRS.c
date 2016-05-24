/*****************************************************************
 *                                                               *
 * C-coded functions to read the the measuring data of the       *
 * 9 Degree of Measurement Attitude and Heading Reference System *
 * of Sparkfun's "9DOF Razor IMU" and "9DOF Sensor Stick"        *
 *                                                               *
 * Developed at Quality & Usability Lab,                         *
 * Deutsche Telekom Laboratories & TU Berlin,                    *
 * written by Christian Krüger,                                  *
 * (C) 2016                                                      *
 *                                                               *
 *                                                               *
 * a former version, used as reference and coded in C++, was     *
 * written by Peter Bartz:                                       *
 *     https://github.com/ptrbrtz/razor-9dof-ahrs                *
 *                                                               *
 *                                                               *
 * --> more informations and changeable user settings            *
 *     below after the preprocessor instructions or              *  
 *     on github:                                                *
 *     https://github.com/krueger-christian/razorAHRS            *
 *                                                               *
 ****************************************************************/

#ifndef RAZORAHRS_C
#define RAZORAHRS_C

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h> // needed for memset
#include <sys/time.h>

/* 
 * includes the termios settings 
 * of the standard IO and the tracker IO
 * and a reset function resetConfig() 
 */
#include "config.h"

/*
 * includes several types of storing 
 * data and settings as well as the
 * functions
 *   - razorsleep( int milliseconds)
 *   - long elapsed_ms(struct timeval start, struct timeval end)
 */
#include "razorTools.h"


/****************************************************************
 ***                  USER SELECTION AREA                     ***
 ****************************************************************/

	/*  => select out of {single, continuous}
	 *
	 *  tracker is readable in two different 
	 *  streaming modes:
	 *  1. single mode: 
	 *     the tracker sends only a frame 
	 *     after request
	 *  2. continuous mode: 
	 *     the tracker sends permanently data
	 *     (every 20ms one frame, 12 byte) */
	//strMode mode = continuous;

	/* time limit (milliseconds) to wait 
	 * during synchronization phase */
	const int connect_timeout_ms = 1000;

	/* time limit (milliseconds) to wait 
	 * during flushing the input line to razor */
	const int flush_timeout_ms = 1000;

	/* disable/enable the output of program 
	 * status reports on the console */
	#define message true

/****************************************************************
 ***                END OF USER SELECTION AREA                ***
 ****************************************************************/




/* kind of flushing the input.
 * the tracker started in textmode and sends strings/frames of the format:
 * "YPR=<yaw-value>,<pitch-value2>,<roll-value3>\r\n",
 * after receiving '\n' we are sure that with the next byte we will 
 * receive the start of a new frame */
bool razorFlush(struct adjustment* settings){
	char singleByte = 'D';

	/* variables to store time values
	 * used to measure how long 
	 * synchronization takes */
	struct timeval t0, t1;
	gettimeofday(&t0, NULL);

	while(singleByte != '\n'){
	
		read(settings->tty_fd,&singleByte,1);

		// check if time out is reached
		gettimeofday(&t1, NULL);
		if (elapsed_ms(t0, t1) > flush_timeout_ms) {
			printf("INFO: Flushing failed. (time out)\n\r"); // TIME OUT!		
			return false;
		}
	}
	return true;
}

/*-----------------------------------------------------------------*/

/* INITIALIZING THE TRACKER
 *
 * The function...
 * ... checks, if tracker is available
 * ... activates continuous binary streaming mode
 * ... ensures that receiving and sending is synchronous
 * ... returns true, if tracker is synchronized
 * ... returns false, if connection or synchronization failed */
bool initRazor(struct adjustment *settings){

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

	/* variables to store time values
	 * used to measure how long 
	 * connecting takes */
	struct timeval t0, t1, t2;
	gettimeofday(&t0, NULL);
	t1 = t0;

	// Trying to connect and switch to binary mode during loop
	if(settings->messageOn) printf("\n  – LOOKING FOR TRACKER: ");	
	while(1){
		if (read(settings->tty_fd,&input,1)>0){
			if(settings->messageOn){
				printf("__okay.\n\r");
				printf("\n  – SWITCH TO BINARY OUTPUT MODE.\n\r");
			}
	
			write(settings->tty_fd,"#o1",3); // output continuous stream
			write(settings->tty_fd,"#ob",3); // output binary
			break;
		}

		// check if time out is reached
		gettimeofday(&t2, NULL);
		if (elapsed_ms(t0, t2) > connect_timeout_ms) {
			if(settings->messageOn) printf("___failed. (time out)\n\r"); // TIME OUT!		
				free(token);			
				return false;
		}
	}

	/* kind of flushing the input.
	 * the tracker started in textmode and sends strings/frames of the format:
	 * "YPR=<yaw-value>,<pitch-value2>,<roll-value3>\r\n",
	 * after receiving '\n' we are sure that with the next byte we will 
	 * receive the start of a new frame */	
	while(input != '\n'){
		read(settings->tty_fd,&input,1);
	}

	// send request for synchronization token
	if(settings->messageOn) printf("\n  – SYNCHRONIZING: ");
	write(settings->tty_fd,"#s00",4);

	/* wait a little bit to get sure that the
	 * tracker read the send signals */
	razorSleep(200);

	// make sure, control variable is set to 0
	size_t token_pos = 0;

	// updating time reference
	gettimeofday(&t0, NULL);
	t1 = t0;

	/* SYNCHRONIZATION
	 * Looking for correct token. */
	while(1){

		// check if input available...
		if (read(settings->tty_fd,&input,1)>0){

			/* ...is the first byte equal to the first
			 * character of the reference token */
			if(input == '#'){
				token_pos++;
				
				token[0] = input;
				
				// ... first byte matchs, so get the next bytes
				while(token_pos < token_length){
					if (read(settings->tty_fd,&input,1)>0){
						token[token_pos] = input;
						token_pos++;
					}					
				}
				
				/* if received token is equal to the reference token
				 * the variable synchronized is set to true */	
				settings->synchronized = ( strcmp(token, token_reference) == 0 ) ? true : false ;

				if(settings->synchronized == true){
					if(settings->messageOn) printf("__okay.\n\r");
						free(token);					
						return true;
				}
			}
		}

		gettimeofday(&t2, NULL);
		if (elapsed_ms(t1, t2) > 200) {
			// 200ms elapsed since last request and no answer -> request synch again
			// (this happens when DTR is connected and Razor resets on connect)
			write(settings->tty_fd,"#s00",4);
			t1 = t2;
		}
		// check if time out is reached
		if (elapsed_ms(t0, t2) > connect_timeout_ms) {
			settings->synchronized = false;
			if(settings->messageOn) printf("___failed. (time out)\n\r"); // TIME OUT!		
				free(token);			
				return false;
		}

		token_pos = 0;	

	}

}

/*-----------------------------------------------------------------*/

bool valueCheck(struct adjustment *settings, struct razorData *data){
			if((data->values[0] > 180) || \
			   (data->values[1] > 180) || \
			   (data->values[0] > 180)){
				settings->messageOn = false;
				settings->synchronized = false;
				initRazor(settings);
				settings->messageOn = true;
				data->data_fail = true;
				
				if(settings->synchronized == false) {
					printf("\n  !\n\r    INFO: Reading failed. (Synchronization Problems)\n\n\n\r");
					return false;
				}
			}
			else data->data_fail = false;
			return true;
}

/*-----------------------------------------------------------------*/

bool readContinuously(struct adjustment *settings, struct razorData *data){

	// Buffer to store user input from console
	unsigned char console = 'D';
	char singleByte = 'D';

	size_t values_pos = 0;
	write(settings->tty_fd,"#ob",3); // just to ensure binary output
	
	bool printData;
	if(settings->messageOn) printData = false;
	else printData = true;
	bool stopRead = false;

	if(settings->messageOn){
		printf("_________________________________________________\r\n");
		printf("\n  !\n\r    PRESS SPACEBAR TO START/STOP SHOWING DATA\n\n\n\r");
	}

	int result;
	int bufferInput = 0;
	while (!stopRead) {

		result = read(settings->tty_fd,&singleByte,1);
		//printf("buffer= %d \t result = %d\n\r", bufferInput, result); // debugging option

       	if (result == 1) {
			data->floatBuffer.ch[bufferInput] = singleByte;
			bufferInput++;

			if(bufferInput == 4){
				data->values[values_pos] = data->floatBuffer.f;
				values_pos++;
				bufferInput = 0;
			}		
		}

		// if new data is available on the serial port, print it out
		if(values_pos == 3){
			if(valueCheck(settings, data) == false) return false;
			if( (printData) && (data->data_fail == false) ){
				printf("YAW = %6.1f \t PITCH = %6.1f \t ROLL = %6.1f \r\n", \
				data->values[0], data->values[1], data->values[2]);
			}
			values_pos = 0;
			razorSleep(20);
		}

		// if new data is available on the console, send it to the serial port
        if (read(STDIN_FILENO,&console,1)>0){  
			if ((printData == false) && (console == ' ')) printData = true;
			else if (console == ' ') stopRead = true;
		}
	}

	// reactivate text mode
	write(settings->tty_fd,"#ot",3);

	return true;	
}

/*-----------------------------------------------------------------*/

bool readSingle(struct adjustment *settings, struct razorData *data){

	// Buffer to store user input from console
	unsigned char console = 'D';
	char singleByte = 'D';

	size_t values_pos = 0;
	write(settings->tty_fd,"#o0",3); // disable output continuous stream
	razorSleep(20);
	write(settings->tty_fd,"#ob",3); // just to ensure binary output
	razorSleep(20);
	razorFlush(settings);

	/* variables to store time values
	 * used to measure how long 
	 * requesting takes */
	struct timeval t0, t1, t2;
	gettimeofday(&t0, NULL);
	t1 = t0;

	bool stopRead = false;

	if(settings->messageOn){
		printf("_________________________________________________\r\n");
		printf("\n  !\n\r    PRESS SPACEBAR TO REQUEST DATA FRAME\n\r");
		printf("    PRESS     Q    TO QUIT\n\n\n\r");
	}

	int result;
	int bufferInput = 0;
	while (!stopRead) {

		// if new data is available on the console...
        if (read(STDIN_FILENO,&console,1)>0){

			// pressed q or Q --> quit
			if ((console == 'q') || (console == 'Q')) stopRead = true;
			// pressed spacebar --> read one frame
			else if(console == ' ') {

				write(settings->tty_fd,"#f",2); // send request

				gettimeofday(&t0, NULL);
				t1 = t0;	
				while(1){
					result = read(settings->tty_fd,&singleByte,1);
					//printf("buffer= %d \t result = %d\n\r", bufferInput, result); // debugging option
					
	       			if (result == 1) {
						data->floatBuffer.ch[bufferInput] = singleByte;
						bufferInput++;

						if(bufferInput == 4){
							data->values[values_pos] = data->floatBuffer.f;
							values_pos++;
							bufferInput = 0;
						}					
					}

					// if new data is available on the serial port, print it out
					if(values_pos == 3){
						if(valueCheck(settings, data) == false) return false;
						if(data->data_fail == false){ 
							printf("YAW = %6.1f \t PITCH = %6.1f \t ROLL = %6.1f \r\n",\
							data->values[0], data->values[1], data->values[2]);
						}
						values_pos = 0;
						razorSleep(20);
						break;
					}

					gettimeofday(&t2, NULL);
					
					if (elapsed_ms(t1, t2) > 200) {
						// 200ms elapsed since last request and no answer -> request synch again
						// (this happens when DTR is connected and Razor resets on connect)
						write(settings->tty_fd,"#f",2);
						values_pos = 0;
						bufferInput = 0;
						t1 = t2;
					}
					
					// check if time out is reached
					if (elapsed_ms(t0, t2) > connect_timeout_ms) {
						if(settings->messageOn) printf("INFO: request failed. (time out)\n\r"); // TIME OUT!		
						break;
					}
				}		
			}
		}
	}

	// reactivate text mode
	write(settings->tty_fd,"#ot",3);

	return true;	
}

/*-----------------------------------------------------------------*/

bool readingRazor(struct adjustment *settings, struct razorData *data){

	if(initRazor(settings) == false) return false;
		
	if(settings->streaming_Mode == continuous){
		readContinuously(settings, data);
	}
	else if(settings->streaming_Mode == single) {
		readSingle(settings,data);
	}
	else{
		printf("INFO: No streaming mode selected!");
		return 1;
	}

	return true;		
}

/*-----------------------------------------------------------------*/

int razorAHRS( speed_t baudRate, char* port, int mode){


		// setting description that is used during the whole process
		struct adjustment *settings;
		settings = (struct adjustment*) calloc(1, sizeof(struct adjustment) );

		// construction to store the data
		struct razorData *data;
		data = (struct razorData*) calloc(1, sizeof(struct razorData) );

		//saving current termios configurations of STDOUT_FILENO
		if (tcgetattr(STDOUT_FILENO, &settings->old_stdio) != 0) {
			settings->stdio_config_changed = false;
        	printf("INFO: Saving configuration STDOUT_FILENO failed.\n\r--> tcgetattr(fd, &old_stdtio)\r\n");
        	return -1;
    	}

		stdio_Config();
		settings->stdio_config_changed = true;

		// saving port id and name in the settings
        settings->tty_fd = open(port, O_RDWR | O_NONBLOCK);      
        settings->port = port;
		settings->streaming_Mode = (mode == 1) ? single : continuous;
		settings->messageOn = message;
		//saving current termios configurations of tty_fd
		if (tcgetattr(settings->tty_fd, &settings->old_tio) != 0) {
			settings->tio_config_changed = false;
        	printf("INFO: Saving configuration of %s failed.\n\r--> tcgetattr(fd, &old_tio)\r\n", port);

			/*reactivating the previous configurations of STDOUT_FILENO 
			because of breaking the process */
			resetConfig(settings);

        	return -1;
    	}		

		tio_Config(settings->tty_fd, baudRate);
		settings->tio_config_changed = true;
	
		readingRazor(settings, data);


		// reactivating previous configurations of tty_fd and the STDOUT_FIELNO
		resetConfig(settings);

		free(settings);
		free(data);
		
		return 0;	
}

/*-------------------------------------------------------------------

--	references

	https://tty1.net/blog/2009/linux-serial-programming-example_en.html

-------------------------------------------------------------------*/

#endif // RAZORAHRS_C

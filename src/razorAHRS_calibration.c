#ifndef RAZORAHRS_CALIB_C
#define RAZORAHRS_CALIB_C

#include "razorAHRS.c"
#include "razorCalib.h"


bool checkCalibToken(struct adjustment *settings){

	write(settings->tty_fd,"#oc",3);
	razorSleep(20);	

	char singleByte = 'D';

	int detectedCharacters = 0;

	/* variables to store time values
	 * used to measure how long 
	 * synchronization takes */
	struct timeval t0, t1;
	gettimeofday(&t0, NULL);

	/* wait for the first character of the expected token */
	while(true){

		read(settings->tty_fd,&singleByte,1);

		if(singleByte == '#'){

			detectedCharacters++;
		//	printf("In: %c (%d)\r\n", singleByte, singleByte);                      //debug

			singleByte = 'D';
			if(detectedCharacters == 25) return true;

			read(settings->tty_fd,&singleByte,1);


			if(singleByte == '#') {
				detectedCharacters++;
			//	printf("In: %c (%d)\r\n", singleByte, singleByte);                      //debug
				singleByte = 'D';
				if(detectedCharacters == 25) return true;

			}
			else{
			//	printf("In: %c (%d)\r\n", singleByte, singleByte);                      //debug
				detectedCharacters = 0;
				singleByte = 'D';
				razorFlush(settings, flush_timeout_ms, 0);
			}

		}

		// check if time out is reached
		gettimeofday(&t1, NULL);
		if (elapsed_ms(t0, t1) > connect_timeout_ms) {
			printf("INFO: Checking calibration token failed. (time out)\n\r"); // TIME OUT!		
			return false;
		}
	}
}

bool calibratingRazor(struct adjustment *settings, struct calibData *data){

	if(initRazor(settings) == false) return false;
		
	// Buffer to store user input from console
	unsigned char console = 'D';
	char singleByte = 'D';
	data->calibTypeSet = false;

	size_t values_pos = 0;

	write(settings->tty_fd,"#ob",3); // just to ensure binary output
	razorSleep(20);

	if(checkCalibToken(settings) == false){
		printf("INFO: Calibration failed.\r\n");
		return false;
	}

	//bool printData;
	//if(settings->messageOn) printData = false;
	//printData = true;
	bool stopRead = false;

	if(settings->messageOn){
		printf("_________________________________________________\r\n");
		printf("\n  !\n\r    PRESS Q TO QUIT CALIBRATION\n\n\n\r");
	}

	int result;
	char *stringBuffer = (char*) calloc(2, sizeof(char));
	bool keepValue = false;
	bool headerSet[3] = {false, false, false};
	int step = 0;
	int bufferInput = 0;
	while (!stopRead) {

		result = read(settings->tty_fd,&singleByte,1);
		//printf("buffer= %d \t result = %d\n\r", bufferInput, result); // debugging option
	

       	if (result == 1) {
			if(data->calibTypeSet == false){
				
				/* first and second sent byte declare the type of sensor
				 * that is currently checked for calibration 
				 * ( "ac" -> accelerometer, "mg" -> magnetometer, "gy" -> gyroscope ) */
				
				if( (singleByte == 'a') || \
					(singleByte == 'm') || \
					(singleByte == 'g') ) 
				{
					stringBuffer[bufferInput] = singleByte;
			//	printf("In: %c (%d)\r\n", singleByte, singleByte);                      //debug
					bufferInput++;
					read(settings->tty_fd,&singleByte,1);

					stringBuffer[bufferInput] = singleByte;
		//		printf("In: %c (%d)\r\n", singleByte, singleByte);                      //debug
					bufferInput++;
			//	printf("Buff: %s \r\n", stringBuffer);                      //debug
				}
				
				if(bufferInput == 2){
					data->calibTypeSet = true;
					if     ( strcmp(stringBuffer, "ac") == 0 ) data->calibType = 0;
					else if( strcmp(stringBuffer, "mg") == 0 ) data->calibType = 1;
					else if( strcmp(stringBuffer, "gy") == 0 ) data->calibType = 2;
					else data->calibTypeSet = false;
					bufferInput = 0;
				}
				
			}
			else{
				data->floatBuffer.ch[bufferInput] = singleByte;
				//printf("In: %c (%d)\r\n", singleByte, singleByte);                      //debug
				bufferInput++;

				if(bufferInput == 4){
					data->values[values_pos] = data->floatBuffer.f;
		//			printf("--> %f\r\n", data->floatBuffer.f);                   //debug
					values_pos++;
					bufferInput = 0;
				}
			}		
		}

		// if new data is available on the serial port, print it out
		if(values_pos == 6){

			//if(valueCheck(settings, data) == false) return false;
			//if( (printData) && (data->data_fail == false) ){
//			if(data->calibType == 0){

				if(headerSet[0] == false){
					print_hint(step);
					headerSet[0] = true;
				}
				else{
					if(step <= 7){
						//       x min | x max || y min | y max || z min | z max
						printf(" %6.1f | %6.1f || %6.1f | %6.1f || %6.1f | %6.1f \r", \
						data->values[0], data->values[1], \
						data->values[2], data->values[3], \
						data->values[4], data->values[5]);
					}

					if(keepValue == true){
						headerSet[0] = false; 
						keepValue = false;
						if     (step == 0) data->acc_xmin = data->values[0];
						else if(step == 1) data->acc_xmax = data->values[1];
						else if(step == 2) data->acc_ymin = data->values[2];
						else if(step == 3) data->acc_ymax = data->values[3];
						else if(step == 4) data->acc_zmin = data->values[4];
						else if(step == 5) data->acc_zmin = data->values[5];
						//else if(step == 6) {}
						else if(step == 7){

							printf("\r\n\n");
							for(int i = 10; i > 0; i--) {
								printf("\r            Wait for %d seconds                         ", i);
								razorSleep(1000);
							}
							printf("\r\n");
							data->gyr_x = data->values[1];
							data->gyr_y = data->values[3];
							data->gyr_z = data->values[5];

							print_hint(8);
							print_result(data);
							step++;
							stopRead = true;					
						}
					}
				}
//			}
//			else if(data->calibType == 2){			
//				printf("GYROSCOPE: x %6.1f \t y %6.1f \t z %6.1f \r", \
//				data->values[1], \
//				data->values[3], \
//				data->values[5]);
//			}
			values_pos = 0;
			data->calibTypeSet = false;
			razorSleep(20);
		}

		// if new data is available on the console, send it to the serial port
        if (read(STDIN_FILENO,&console,1)>0){  
			if((console == 'q') ||  (console == 'Q')) stopRead = true;
			//else if((console == 'n') ||  (console == 'N')) write(settings->tty_fd,"#on",3);
			else if((console == 'k') ||  (console == 'K')) {
				keepValue = true;
				if(step == 5) {
					write(settings->tty_fd,"#on",3); // switch to magnetometer callibration (jump over)
					razorSleep(20);
					write(settings->tty_fd,"#on",3); // switch to gyroscope callibration
					razorSleep(20);
				}				
				else write(settings->tty_fd,"#oc",3); // restart callibration (inner mode)
				razorSleep(20);
				step++;
			}
		}
	}

	for(int i = 0; i < 3; i++){
		if(headerSet[i] == false) {
			write(settings->tty_fd,"#on",3);
			razorSleep(20);
		}
	}
	// reactivate text mode
	write(settings->tty_fd,"#ot",3);
	razorSleep(20);

	printf("\r\n");
	return true;	

}


int razorAHRS_calibration( speed_t baudRate, char* port){


		// setting description that is used during the whole process
		struct adjustment *settings;
		settings = (struct adjustment*) calloc(1, sizeof(struct adjustment) );

		// construction to store the data
		struct calibData *data;
		data = (struct calibData*) calloc(1, sizeof(struct calibData) );

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
		settings->streaming_Mode = continuous;
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
	
		calibratingRazor(settings, data);


		// reactivating previous configurations of tty_fd and the STDOUT_FILENO
		resetConfig(settings);

		free(settings);
		free(data);
		
		return 0;	
}

#endif

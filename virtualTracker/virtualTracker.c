#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h>
#include <time.h>

#include <fcntl.h>
#include <termios.h>

#include <string.h> // needed for memset

#include "../config.h"
#include "../razorTools.h"


/*---------------------------------------------------------*/

struct adjustment{
	struct termios old_tio;
	struct termios old_stdio;
	outputFormat output_Format;
	char *vt_port;
    int tty_fd;
	int vt_frequency;
	int waitingTime;
	speed_t vt_baudRate;
};

/*---------------------------------------------------------*/

int hertzToMilliseconds(int frequency){
	return (1000 / frequency);
}

/*---------------------------------------------------------*/

bool prepareIOConfiguration(struct adjustment *adj){

	//saving current termios configurations of STDOUT_FILENO
	if (tcgetattr(STDOUT_FILENO, &adj->old_stdio) != 0) {
       	printf("tcgetattr(fd, &old_stdtio) failed\n");
       	return false;
   	}

	stdio_Config();

    adj->tty_fd = open(adj->vt_port, O_RDWR | O_NONBLOCK);      
	
	//saving current termios configurations of tty_fd
	if (tcgetattr(adj->tty_fd, &adj->old_tio) != 0) {
		printf("working on %s (%d) \r\n", adj->vt_port, adj->tty_fd );
    	printf("tcgetattr(fd, &old_tio) failed\n");
       	return false;
    }		

	tio_Config(adj->tty_fd, adj->vt_baudRate);

}

/*---------------------------------------------------------*/

void resetIOConfiguration(struct adjustment *adj){
	// reactivating previous configurations of tty_fd and the STDOUT_FIELNO
	tcsetattr(adj->tty_fd, TCSANOW, &adj->old_tio);
	tcsetattr(STDOUT_FILENO, TCSANOW, &adj->old_stdio);
    close(adj->tty_fd);
}

/*---------------------------------------------------------*/

bool virtualTracker(int frequency, speed_t baudRate, char* port){

	struct adjustment *setting;
	setting = (struct adjustment*) calloc(1, sizeof(struct adjustment) );
	setting->vt_port = port;
	setting->vt_baudRate = baudRate;
	setting->vt_frequency = frequency;
	setting->output_Format = text;
	setting->waitingTime = hertzToMilliseconds(frequency);
	if(prepareIOConfiguration(setting) == false) return false;

	razorBuffer *floatBuffer;
	floatBuffer = (razorBuffer*) calloc(1, sizeof(razorBuffer) );
	int bufferInput = 0;

	float arr[3];
	arr[0] = 0;		
	arr[1] = 0;
	arr[2] = 0;

	unsigned char singleByte = 'D';
	unsigned char id[2];
	unsigned char newline = '\n';
	int i = 0;
	int result = 0;
	while (i < 1000) {

		result = read(setting->tty_fd,&singleByte,1);

       	if( (result == 1) && (singleByte = '#') ){

			result = read(setting->tty_fd,&singleByte,1);

	       	if( (result == 1) && (singleByte = 's') ){
				// Synch Request
				read(setting->tty_fd,&singleByte,1);
				id[0] = singleByte;
				read(setting->tty_fd,&singleByte,1);
 				id[1] = singleByte;
				char synch[6] = {'#','S','Y','N','C','H'};
				write(setting->tty_fd, synch, 6);
				write(setting->tty_fd, id, 2);
				write(setting->tty_fd, &newline, 1);
			}
			else if( (result == 1) && (singleByte = 'b') ) setting->output_Format = binary;
			else if( (result == 1) && (singleByte = 't') ) setting->output_Format = text;
			
		}

		if(arr[0] < 180) arr[0] += 10;
		else if(arr[1] < 180) arr[1] += 10;
		else if(arr[2] < 180) arr[2] += 10;
		else {arr[0] = 0; arr[1] = 0; arr[2] = 0;}

		if(setting->output_Format == binary){
			printf("\rYAW: %.1f\t PITCH: %.1f\t ROLL: %.1f\n", arr[0], arr[1], arr[2]);				
			write(setting->tty_fd, arr, 12);
		}
		else{
			// snprintf() returns the number of characters it would have written
			ssize_t len = snprintf(NULL, 0, "#YPR=%.2f,%.2f,%.2f\r\n", arr[0], arr[1], arr[2]);

			// allocate storage for a string
			char *output = malloc( len+1 );

			if( output != NULL ) {
				// generate String
				sprintf(output, "#YPR=%.2f,%.2f,%.2f\r\n", arr[0], arr[1], arr[2]);

				printf("%s\n", output);

				write(setting->tty_fd, output, len);
				free(output);
			}
		}
	

		razorSleep(setting->waitingTime);
		i++;

	}

	resetIOConfiguration(setting);
	free(floatBuffer);
	free(setting);

	return true;

}

/*---------------------------------------------------------*/



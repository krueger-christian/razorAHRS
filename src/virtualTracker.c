#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h>
#include <time.h>

#include <fcntl.h>
#include <termios.h>

#include <string.h> // needed for memset

#include <errno.h>

#include "config.h"
#include "razorTools.h"


/*---------------------------------------------------------*/

struct vt_adjustment{
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

void resetIOConfiguration(struct vt_adjustment *setting){
	// reactivating previous configurations of tty_fd and the STDOUT_FIELNO
	tcsetattr(setting->tty_fd, TCSANOW, &setting->old_tio);
	tcsetattr(STDOUT_FILENO, TCSANOW, &setting->old_stdio);
    close(setting->tty_fd);
}

/*---------------------------------------------------------*/

bool prepareIOConfiguration(struct vt_adjustment *setting){

	//saving current termios configurations of STDOUT_FILENO
	if (tcgetattr(STDOUT_FILENO, &setting->old_stdio) != 0) {
       	printf("tcgetattr(fd, &old_stdtio) failed\n");
       	return false;
   	}

	stdio_Config();

    setting->tty_fd = open(setting->vt_port, O_RDWR | O_NONBLOCK);      

	printf("Connecting to: %s (%d) \r\n", setting->vt_port, setting->tty_fd );		

	//saving current termios configurations of tty_fd
	if (tcgetattr(setting->tty_fd, &setting->old_tio) != 0) {
		printf("tcgetattr(fd, &old_tio) failed\n");
		resetIOConfiguration(setting);
       	return false;
    }		

	tio_Config(setting->tty_fd, setting->vt_baudRate);

	return true;

}

/*---------------------------------------------------------*/

void sendToken(struct vt_adjustment *setting, unsigned char *id){

	unsigned char newline = '\n';

	char synch[6] = {'#','S','Y','N','C','H'};
	write(setting->tty_fd, synch, 6);
	write(setting->tty_fd, id, 2);
	write(setting->tty_fd, &newline, 1);

}

/*---------------------------------------------------------*/

bool loopOrBreak(char stopCharacter){
	unsigned char console = 'D';

	if (read(STDIN_FILENO,&console,1)>0){  
		if (console == stopCharacter) return false;
	}
	
	return true;
}

/*---------------------------------------------------------*/

int virtualTracker(int frequency, speed_t baudRate, char* port){

	struct vt_adjustment *setting;
	setting = (struct vt_adjustment*) calloc(1, sizeof(struct vt_adjustment) );
	setting->vt_port = port;
	setting->vt_baudRate = baudRate;
	setting->vt_frequency = frequency;
	setting->output_Format = text;
	setting->waitingTime = hertzToMilliseconds(frequency);
	if(prepareIOConfiguration(setting) == false) return -1;

	razorBuffer *floatBuffer;
	floatBuffer = (razorBuffer*) calloc(1, sizeof(razorBuffer) );

	float arr[3] = {0,0,0};

	unsigned char singleByte = 'D';
	unsigned char *id;
	id = calloc(2, sizeof(int) );

	int readBytes = 0;

	printf("\nPress spacebar to stop virtual Razor.\n\n");
	while ( loopOrBreak(' ') ) {

		readBytes = read(setting->tty_fd,&singleByte,1);

		// INPUT CHECK
       	if( (readBytes == 1) && (singleByte = '#') ){

			readBytes = read(setting->tty_fd,&singleByte,1);

	       	if( (readBytes == 1) && (singleByte = 's') ){
				
				// Synch Request
				read(setting->tty_fd,&singleByte,1);
				id[0] = singleByte;
				read(setting->tty_fd,&singleByte,1);
 				id[1] = singleByte;

 				sendToken(setting, id);

 				free(id);
				
			}
			// set output mode to binary (3 * 4 bytes = three floating point values)
			else if( (readBytes == 1) && (singleByte = 'b') ) setting->output_Format = binary;
			// set output mode to string (text)
			else if( (readBytes == 1) && (singleByte = 't') ) setting->output_Format = text;
			
		}

		// incrementing output angles
		if(arr[0] < 180) arr[0] += 10;
		else if(arr[1] < 180) arr[1] += 10;
		else if(arr[2] < 180) arr[2] += 10;
		else {arr[0] = 0; arr[1] = 0; arr[2] = 0;}

		// BINARY OUTPUT MODE
		if(setting->output_Format == binary){
			printf("\rYAW: %.1f\t PITCH: %.1f\t ROLL: %.1f\n", arr[0], arr[1], arr[2]);				
			write(setting->tty_fd, arr, 12);
		}
		// TEXT OUTPUT MODE
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

	}

	resetIOConfiguration(setting);
	free(floatBuffer);
	free(setting);

	return 0;

}

/*---------------------------------------------------------*/



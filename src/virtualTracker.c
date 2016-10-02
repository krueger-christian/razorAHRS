#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h>
#include <time.h>

#include <fcntl.h>
#include <termios.h>

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <errno.h>

/*----------------------------------------------------------------------------------------------------*/

	// streaming mode could be single or continous
	enum streamingMode {single, continuous};
	typedef enum streamingMode strMode;

/*----------------------------------------------------------------------------------------------------*/

	enum oFormat {text, binary};
	typedef enum oFormat outputFormat;

/*----------------------------------------------------------------------------------------------------*/

	struct vt_adjustment {
		struct termios old_tio;
		struct termios old_stdio;
		outputFormat output_Format;
		strMode output_Mode;
		char *vt_port;
		int tty_fd;
		int vt_frequency;
		int waitingTime;
		speed_t vt_baudRate;
	};

/*----------------------------------------------------------------------------------------------------*/

    /* measuring elapsed time:
	 * During synchronization we need to check the time. Ones to know,
	 * when we have to send a new request to the tracker/board, second 
	 * to stop the attempt after a certain amount of time, when we could 
	 * be shure not having success anymore.   
	*/
	long elapsed_ms(struct timeval start, struct timeval end) {
      return (long) ((end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000);
    }

/*----------------------------------------------------------------------------------------------------*/

	/* 
	 * sleep function
	 */
	void razorSleep(int milliseconds) {
		struct timespec waitTime;
		waitTime.tv_sec = milliseconds / 1000;
		waitTime.tv_nsec = (milliseconds % 1000) * 1000000;
		nanosleep(&waitTime, NULL);
	}

/*----------------------------------------------------------------------------------------------------*/

	void stdio_Config() {

		struct termios stdio;

		memset(&stdio, 0, sizeof (stdio));
		stdio.c_iflag = 0;
		stdio.c_oflag = 0;
		stdio.c_cflag = 0;
		stdio.c_lflag = 0;
		stdio.c_cc[VMIN] = 1;
		stdio.c_cc[VTIME] = 10;
		tcsetattr(STDOUT_FILENO, TCSANOW, &stdio);
		tcsetattr(STDOUT_FILENO, TCSAFLUSH, &stdio);
		fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // make the reads non-blocking

	}

/*----------------------------------------------------------------------------------------------------*/

	void tio_Config(int tty_fd, speed_t baudRate) {

		struct termios tio;

		memset(&tio, 0, sizeof (tio));
		tio.c_iflag = 0;
		tio.c_oflag = 0;
		tio.c_cflag = CS8 | CREAD | CLOCAL; // 8n1, see termios.h for more information
		tio.c_lflag = 0;
		tio.c_cc[VMIN] = 1;
		tio.c_cc[VTIME] = 10;
		cfsetospeed(&tio, baudRate); // 57600 baud
		cfsetispeed(&tio, baudRate); // 57600 baud
		tcsetattr(tty_fd, TCSANOW, &tio);
	}

/*----------------------------------------------------------------------------------------------------*/

	int hertzToMilliseconds(int frequency) {
		return (1000 / frequency);
	}

/*----------------------------------------------------------------------------------------------------*/

	void resetIOConfiguration(struct vt_adjustment *setting) {
		// reactivating previous configurations of tty_fd and the STDOUT_FIELNO
		tcsetattr(setting->tty_fd, TCSANOW, &setting->old_tio);
		tcsetattr(STDOUT_FILENO, TCSANOW, &setting->old_stdio);
		close(setting->tty_fd);
	}

/*----------------------------------------------------------------------------------------------------*/

	bool prepareIOConfiguration(struct vt_adjustment *setting) {
		//saving current termios configurations of STDOUT_FILENO
		if (tcgetattr(STDOUT_FILENO, &setting->old_stdio) != 0) {
		    printf("tcgetattr(fd, &old_stdtio) failed\n");
		    return false;
		}

		stdio_Config();

		setting->tty_fd = open(setting->vt_port, O_RDWR | O_NONBLOCK);

		printf("Connecting to: %s (%d) \r\n", setting->vt_port, setting->tty_fd);

		//saving current termios configurations of tty_fd
		if (tcgetattr(setting->tty_fd, &setting->old_tio) != 0) {
		    printf("tcgetattr(fd, &old_tio) failed\n");
		    resetIOConfiguration(setting);
		    return false;
		}

		tio_Config(setting->tty_fd, setting->vt_baudRate);

		return true;

	}

/*----------------------------------------------------------------------------------------------------*/

	void sendToken(struct vt_adjustment *setting, unsigned char *id) {
		unsigned char *synch;
		synch = calloc(10, sizeof (unsigned char));

		synch[0] = '#';
		synch[1] = 'S';
		synch[2] = 'Y';
		synch[3] = 'N';
		synch[4] = 'C';
		synch[5] = 'H';
		synch[6] = id[0];
		synch[7] = id[1];
		synch[8] = '\r';
		synch[9] = '\n';
		printf("sending: %s", synch);

		write(setting->tty_fd, synch, 10);

		free(synch);
	}

/*----------------------------------------------------------------------------------------------------*/

	bool loopOrBreak(char stopCharacter) {
		unsigned char console = 'D';

		if (read(STDIN_FILENO, &console, 1) > 0 && console == stopCharacter) {
		    return false;
		}

		return true;
	}

/*----------------------------------------------------------------------------------------------------*/

	int virtualTracker(int frequency, speed_t baudRate, char* port) {
		struct vt_adjustment *setting;
		setting = (struct vt_adjustment*) calloc(1, sizeof (struct vt_adjustment));
		setting->vt_port = port;
		setting->vt_baudRate = baudRate;
		setting->vt_frequency = frequency;
		setting->output_Format = text;
		setting->output_Mode = continuous;
		setting->waitingTime = hertzToMilliseconds(frequency);
		if (prepareIOConfiguration(setting) == false) return -1;

		float arr[3] = {0, 0, 0};

		unsigned char singleByte = 'D';
		unsigned char *id;
		id = calloc(2, sizeof (int));

		int readBytes = 0;
		bool request = false;

		printf("\nPress spacebar to stop virtual Razor.\n\n");
		while (loopOrBreak(' ')) {
		    readBytes = read(setting->tty_fd, &singleByte, 1);

		    // INPUT CHECK
		    if ((readBytes == 1) && (singleByte = '#')) {
		        printf("\r\nreading: %c", singleByte);
		        readBytes = read(setting->tty_fd, &singleByte, 1);

		        if ((readBytes == 1) && (singleByte == 's')) {
		            printf("%c\n\r", singleByte);

		            // Synch Request
		            read(setting->tty_fd, &singleByte, 1);
		            id[0] = singleByte;
		            read(setting->tty_fd, &singleByte, 1);
		            id[1] = singleByte;

		            sendToken(setting, id);

		        } else if ((readBytes == 1) && (singleByte == 'o')) {
		            printf("%c", singleByte);
		            readBytes = read(setting->tty_fd, &singleByte, 1);

		            printf("%c\r\n", singleByte);

		            // set output mode to binary (3 * 4 bytes = three floating point values)
		            if ((readBytes == 1) && (singleByte == 'b')) setting->output_Format = binary;
		                // set output mode to string (text)
		            else if ((readBytes == 1) && (singleByte == 't')) setting->output_Format = text;
					else if ((readBytes == 1) && (singleByte == '1')) setting->output_Mode = continuous;
					else if ((readBytes == 1) && (singleByte == '0')) setting->output_Mode = single;
		        } else if ((readBytes == 1) && (singleByte == 'f')) {
		            printf("%c", singleByte);					
					request = true;
				}
		    }

		    // incrementing output angles
		    if (arr[0] < 180) arr[0] += 10;
		    else if (arr[1] < 180) arr[1] += 10;
		    else if (arr[2] < 180) arr[2] += 10;
		    else {
		        arr[0] = 0;
		        arr[1] = 0;
		        arr[2] = 0;
		    }

			if((setting->output_Mode == continuous) || (request == true)){
				// BINARY OUTPUT MODE
				if (setting->output_Format == binary) {
					//printf("\rYAW: %.1f\t PITCH: %.1f\t ROLL: %.1f\n",\
					arr[0], arr[1], arr[2]);				
				    write(setting->tty_fd, arr, 12);
				}// TEXT OUTPUT MODE
				else {
				    // snprintf() returns the number of characters it would have written
				    ssize_t len = snprintf(NULL, 0, "#YPR=%.2f,%.2f,%.2f\r\n", arr[0], arr[1], arr[2]);

				    // allocate storage for a string
				    char *output = malloc(len + 1);

				    if (output != NULL) {
				        // generate String
				        sprintf(output, "#YPR=%.2f,%.2f,%.2f\r\n", arr[0], arr[1], arr[2]);

				        //printf("%s\n", output);

				        write(setting->tty_fd, output, len);
				        free(output);
				    }
				}
				request = false;
			}
		    razorSleep(setting->waitingTime);
		}

		resetIOConfiguration(setting);
		free(setting);
		free(id);

		return 0;
	}

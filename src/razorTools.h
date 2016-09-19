/*****************************************************************
 *                                                               *
 * (c) 2016 / QU Lab / T-Labs / TU Berlin                        *
 *                                                               *
 * --> more informations and changeable user settings            *
 *     in the razorAHRS.c file or on github                      *
 *     https://github.com/krueger-christian/razorAHRS            *
 *                                                               *
 ****************************************************************/

#ifndef RAZORTOOLS_H
#define RAZORTOOLS_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

/*----------------------------------------------------------------------------------------------------*/

	// streaming mode could be single or continous
	enum streamingMode {single, continuous};
	typedef enum streamingMode strMode;

/*----------------------------------------------------------------------------------------------------*/

	/*  text: char array with undefined length, starts with "YPR" and terminates with "\n"
	 *  binary: 12 Byte format, equals 3 float values
	 *  32bit: 4 Byte format, equal to the long format but isn't readable as a long, containing 
	 *         all three sensor angles as kind of a 10 Bit "integer"
     */
	enum oFormat {text, binary, fourbyte};
	typedef enum oFormat outputFormat;

/*----------------------------------------------------------------------------------------------------*/

	/* The struggle is, to transform the binary data of the serial stream
	 * into useful values. It's just a matter of interpretting ones
	 * and zeros. So we can use the same memory location and don't care if
	 * we put four characters inside, one float value or the razor specific 32 bit 
	 * data format. All cases require four bytes. We do this using a union structure... */ 
	union rzrBffr{
		float f;
		char ch[4];
		long l;
	};
	typedef union rzrBffr razorBuffer;

/*----------------------------------------------------------------------------------------------------*/

	struct adjustment{
		struct termios old_tio;
		outputFormat output_Format;
		strMode streaming_Mode;
		bool messageOn;
		bool tio_config_changed;
		char *port;
    	int tty_fd;
		int vt_frequency;
		int waitingTime;
		bool synchronized;
		bool *tracker_should_exit;
		speed_t baudRate;
	};

/*----------------------------------------------------------------------------------------------------*/

	typedef struct r_thread_manager{
		struct adjustment* settings;
		struct razorData*  data;
		pthread_mutex_t settings_protect;
		pthread_mutex_t data_protect;
		pthread_cond_t data_updated;
		pthread_cond_t update;
		int thread_id;
		pthread_t thread;
		bool dataUpdated;
		bool razor_is_running;
		bool printer_is_running;
	} razor_thread_manager;

/*----------------------------------------------------------------------------------------------------*/

	/* data type to store the yaw, pitch and roll
	   value of the RazorAHRS tracker */
	struct razorData{

		/*
		 * Flag that is managed by valueCheck() function
		 * true: the current values don't match the valid range
		 * false: the current values are inside the valid range */
		bool data_fail;

		/* 
		 * Flag to signal if someone requests an update of the data */
		bool dataRequest;

		/* array that stores the sensor data
		 *
		 * values[0] = Yaw
		 * values[1] = Pitch
	 	 * values[2] = Roll  */
		float values[3];

		/* union structure to store blocks of 
		 *  4 Byte received by the tracker (equal 
		 *  to the size of a single float or long value.
		 *  for further information look at 
		 *  razorTools.h */
		razorBuffer buffer;
	};

/*----------------------------------------------------------------------------------------------------*/

	/*  data type to store the calibration values
	 *	                |        |        |          |          |          |
	 *	TYPE OF CALIBR. |   0    |   1    |     2    |     3    |     4    |    5
	 *	––––––––––––––––+––––––––+––––––––+––––––––––+––––––––––+––––––––––+–––––––––
	 *	––––––––––––––––+––––––––+––––––––+––––––––––+––––––––––+––––––––––+–––––––––
	 *   accelerometer  | x_min  | x_max  | y_min    | y_max    | z_min    | z_max
	 *	––––––––––––––––+––––––––+––––––––+––––––––––+––––––––––+––––––––––+–––––––––
	 *	 magnetometer   | x_min  | x_max  | y_min    | y_max    | z_min    | z_max
	 *	––––––––––––––––+––––––––+––––––––+––––––––––+––––––––––+––––––––––+–––––––––
	 *	 gyrometer      | x      | x_aver | y        | y_aver   | z        | z_aver
	 *	––––––––––––––––+––––––––+––––––––+––––––––––+––––––––––+––––––––––+–––––––––
	 */
	struct calibData{

		/*
		 * Flag that is managed by valueCheck() function
		 * true: the current values don't match the valid range
		 * false: the current values are inside the valid range*/
		bool data_fail;

		/*
		 * 0: accelerometer
		 * 1: magnetometer
		 * 2: gyroscope
		 */
		int calibType;

		bool calibTypeSet;

		float acc_xmin;
		float acc_xmax;
		float acc_ymin;
		float acc_ymax;
		float acc_zmin;
		float acc_zmax;

		float gyr_x;
		float gyr_y;
		float gyr_z;

		/* array that stores the sensor data
		 *
		 * values[0] = Yaw
		 * values[1] = Pitch
	 	 * values[2] = Roll  */
		float values[6];

		/* union structure to store blocks of 
		 *  4 Byte received by the tracker (equal 
		 *  to the size of a single float value.
		 *  for further information look at 
		 *  razorTools.h */
		razorBuffer buffer;
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

/* kind of flushing the input.
 *
 * On one hand because the tracker started in textmode and sends 
 * strings/frames of the format:
 * "YPR=<yaw-value>,<pitch-value2>,<roll-value3>\r\n",
 * after receiving '\n' we are sure that with the next byte we 
 * will receive the start of a new frame 
 *
 * On the other hand it could be a problem after switching from
 * continuous streaming to single frame streaming. During continuous
 * streaming are so many reads made that there are still a lot of 
 * frames in the pipe after even after the switch. We need the second
 * flush type:
 *
 * 1. flush type: flushing until end-of-line character is detected
 *                --> use flag flushType = 0
 *
 * 2. flush type: flushing until no input is readable anymore
 *                --> use flag flushType = 1
 */
bool razorFlush(struct adjustment* settings, int flush_timeout_ms, int flushType){
	char singleByte = 'D';

	/* variables to store time values
	 * used to measure how long 
	 * synchronization takes */
	struct timeval t0, t1;
	gettimeofday(&t0, NULL);

	//flush to end of line
	if(flushType == 0){
		while(singleByte != '\n'){
	
			read(settings->tty_fd,&singleByte,1);

			// check if time out is reached
			gettimeofday(&t1, NULL);
			if (elapsed_ms(t0, t1) > flush_timeout_ms) {
 				// TIME OUT!
				if(settings->messageOn) printf("INFO: Flushing failed. (time out)\n\r");	
				return false;
			}
		}
	}
	//flush full input
	else if(flushType == 1){
		while(read(settings->tty_fd,&singleByte,1) > 0){
			// check if time out is reached
			gettimeofday(&t1, NULL);
			if (elapsed_ms(t0, t1) > flush_timeout_ms) {
				// TIME OUT!	
				if(settings->messageOn) printf("INFO: Flushing failed. (time out)\n\r");		
				return false;
			}
		}
	}
	else{
		if(settings->messageOn) printf("INFO: Flushing failed. (invalid flushing type flag)\n\r");
		return false;		
	}
	return true;
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

void resetConfig(struct adjustment *settings) {
    if (settings->tio_config_changed) {
        tcsetattr(settings->tty_fd, TCSANOW, &settings->old_tio);
    }

    close(settings->tty_fd);
}

#endif // RAZORTOOLS_H

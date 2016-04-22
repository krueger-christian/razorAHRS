#ifndef RAZORTOOLS_H
#define RAZORTOOLS_H


#include <sys/time.h>

#include <time.h>

/*-----------------------------------------------------------------*/

	/* because C doesn't know the bool type,
	 *an integer based version is created */
	typedef int bool;
	#define true 1
	#define false 0

/*-----------------------------------------------------------------*/

	/* The struggle is, to transform the binary data of the serial stream
	 * into useful float values. It's just a matter of interpretting ones
	 * and zeros. So we can use the same memory location and don't care if
	 * we put four characters inside or one float value, both cases require
	 * four bytes. We do this using a union structure... */ 
	union rzrBffr{
		float f;
		char ch[4];
	};
	typedef union rzrBffr razorBuffer;

	

/*-----------------------------------------------------------------*/

	/* 
	 *
	 */
	void razorSleep(int milliseconds) {
		struct timespec waitTime;
		waitTime.tv_sec = milliseconds / 1000;
		waitTime.tv_nsec = (milliseconds % 1000) * 1000000;
		nanosleep(&waitTime, NULL);
	}

/*-----------------------------------------------------------------*/

	// streaming mode could be single or continous
	enum streamingMode {single, continous};
	typedef enum streamingMode strMode;

/*-----------------------------------------------------------------*/

	// streaming mode could be single or continous
	enum oFormat {text, binary};
	typedef enum oFormat outputFormat;

/*-----------------------------------------------------------------*/

    /* measuring elapsed time:
	 * During synchronization we need to check the time. Ones to know,
	 * when we have to send a new request to the tracker/board, second 
	 * to stop the attempt after a certain amount of time, when we could 
	 * be shure not having success anymore.   
	*/
	long elapsed_ms(struct timeval start, struct timeval end) {
      return (long) ((end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000);
    }

/*-----------------------------------------------------------------*/


	/* Compares two strings. If the strings are literally are equal,
	 * it returns true (1), otherwise false (0)
	*/
	bool stringCompare(char* s1, char* s2, int length){
		for(int i = 0; i < length; i++){
			if(s1[i] != s2[i]){
				return false;
			}
		}
		return true;	
	}


#endif // RAZORTOOLS_H

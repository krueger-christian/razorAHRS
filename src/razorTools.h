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


#include <sys/time.h>
#include <time.h>

/*-----------------------------------------------------------------*/

/* because C doesn't know the bool type,
 *an integer based version is created */
typedef int bool;
#define true 1
#define false 0

/*-----------------------------------------------------------------*/

// streaming mode could be single or continous

enum streamingMode {
    single, continuous
};
typedef enum streamingMode strMode;

/*-----------------------------------------------------------------*/

// streaming mode could be single or continous

enum oFormat {
    text, binary
};
typedef enum oFormat outputFormat;

/*-----------------------------------------------------------------*/

/* The struggle is, to transform the binary data of the serial stream
 * into useful float values. It's just a matter of interpretting ones
 * and zeros. So we can use the same memory location and don't care if
 * we put four characters inside or one float value, both cases require
 * four bytes. We do this using a union structure... */
union rzrBffr {
    float f;
    char ch[4];
};
typedef union rzrBffr razorBuffer;

/*---------------------------------------------------------*/

struct adjustment {
    struct termios old_tio;
    struct termios old_stdio;
    outputFormat output_Format;
    strMode streaming_Mode;
    bool messageOn;
    bool tio_config_changed;
    bool stdio_config_changed;
    char *port;
    int tty_fd;
    int vt_frequency;
    int waitingTime;
    bool synchronized;
    speed_t baudRate;
};

/*-----------------------------------------------------------------*/

/* data type to store the yaw, pitch and roll
   value of the RazorAHRS tracker */
struct razorData {
    /*
     * Flag that is managed by valueCheck() function
     * true: the current values don't match the valid range
     * false: the current values are inside the valid range*/
    bool data_fail;

    /* array that stores the sensor data
     *
     * values[0] = Yaw
     * values[1] = Pitch
     * values[2] = Roll  */
    float values[3];

    /* union structure to store blocks of 
     *  4 Byte received by the tracker (equal 
     *  to the size of a single float value.
     *  for further information look at 
     *  razorTools.h */
    razorBuffer floatBuffer;
};

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

/* 
 * sleep function
 */
void razorSleep(int milliseconds) {
    struct timespec waitTime;
    waitTime.tv_sec = milliseconds / 1000;
    waitTime.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&waitTime, NULL);
}

/*-----------------------------------------------------------------*/

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
bool razorFlush(struct adjustment* settings, int flush_timeout_ms, int flushType) {
    char singleByte = 'D';

    /* variables to store time values
     * used to measure how long 
     * synchronization takes */
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    //flush to end of line
    if (flushType == 0) {
        while (singleByte != '\n') {

            read(settings->tty_fd, &singleByte, 1);

            // check if time out is reached
            gettimeofday(&t1, NULL);
            if (elapsed_ms(t0, t1) > flush_timeout_ms) {
                printf("INFO: Flushing failed. (time out)\n\r"); // TIME OUT!		
                return false;
            }
        }
    }        //flush full input
    else if (flushType == 1) {
        while (read(settings->tty_fd, &singleByte, 1) > 0) {
            // check if time out is reached
            gettimeofday(&t1, NULL);
            if (elapsed_ms(t0, t1) > flush_timeout_ms) {
                printf("INFO: Flushing failed. (time out)\n\r"); // TIME OUT!		
                return false;
            }
        }
    } else {
        printf("INFO: Flushing failed. (invalid flushing type flag)\n\r");
        return false;
    }
    return true;
}

#endif // RAZORTOOLS_H

#ifndef RAZOR_CONFIG_H
#define RAZOR_CONFIG_H

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
const int connect_timeout_ms = 5000;

/* time limit (milliseconds) to wait 
 * during flushing the input line to razor */
const int flush_timeout_ms = 1000;

/* disable/enable the output of program 
 * status reports on the console */
#define message true


/****************************************************************
 ***                END OF USER SELECTION AREA                ***
 ****************************************************************/

#endif // RAZOR_CONFIG_H

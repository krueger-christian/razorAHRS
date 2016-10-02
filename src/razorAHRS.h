#ifndef RAZORAHRS_H
#define RAZORAHRS_H

/* SYNCHRONIZING THE TRACKER
 *
 * -- checks, if tracker is available
 * -- ensures that receiving and sending is synchronous
 * -- returns true, if tracker is synchronized
 * -- returns false, if connection or synchronization failed 
 */
bool synch (razor_thread_manager * manager);

/*-----------------------------------------------------------------*/

/* -- checks if values are inside the valid range [-360, 360]
 * -- starts resynchronization if invalid values are detected
 * -- returns true, if values are valid or resynchronization 
 *    was successfull 
 */
bool valueCheck (razor_thread_manager * manager);
/*-----------------------------------------------------------------*/

/* -- mode for continuous streaming
 * -- read values are stored at manager->data->values,
 *    data structure is protected with mutex manager->data_protect
 */
bool readContinuously (razor_thread_manager * manager);

/*-----------------------------------------------------------------*/

/* -- mode for streaming single frames on request
 * -- not ready for thread use!!!
 * -- read values are stored at manager->data->values,
 *    data structure is protected with mutex manager->data_protect
 */
bool readSingle (razor_thread_manager * manager);

/*-----------------------------------------------------------------*/

/* -- manages wether streaming continuous or on request
 */
void *readingRazor (razor_thread_manager * manager);

/*-----------------------------------------------------------------*/

/* -- creates the basis of the razor reading thread
 * -- returns a pointer to an allocated structure, that is used
 *    to manage the thread and its data transfer. 
 *    For further information look at razorTools.h
 */
razor_thread_manager *razorAHRS (speed_t baudRate, char *port, int mode, int format);

/*-----------------------------------------------------------------*/

/* -- simply starts the razor thread
 * -- return 0 if successfull, else -1
 */
int razorAHRS_start (razor_thread_manager * manager);

/*-----------------------------------------------------------------*/

/* -- not for interface use! 
 * -- to stop the the thread use:
 *    razorAHRS_stop(razor_thread_manager *manager)
 * -- manages the end of the razor thread
 * -- frees allocated space
 * -- returns 0 if succesfull
 */
int razorAHRS_quit (razor_thread_manager * manager);

/*-----------------------------------------------------------------*/

/*  stops the razor thread
 */
void razorAHRS_stop (razor_thread_manager * manager);

/*-----------------------------------------------------------------*/

/*  requests a single data frame
 *  returns  0 if successfull
 *  returns -1 if failed
 *  works only in single frame streaming mode (mode = 1)
 *    --> initialize with razorAHRS(<baudrate>, <port name>, 1)
 */
int razorAHRS_request (razor_thread_manager * manager);

/*-----------------------------------------------------------------*/

/* -- Printer thread function, is called by razorPrinter_start
 * -- !!! Attention !!! not save for parallel use with other threads
 * -- mining the same data
 */
void *razorPrinter (void *args);

/*-----------------------------------------------------------------*/

/* -- output current values on stdout
 * -- !!! Attention !!! not save for parallel use with other threads
 * -- mining the same data
 */
void razorPrinter_start (razor_thread_manager * manager, pthread_t * printer);

/*-----------------------------------------------------------------*/

/* -- stops the output of the values on stdout
 */
int razorPrinter_stop ();

#endif

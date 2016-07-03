/*****************************************************************
 *                                                               *
 *                                                               *
 *               -- DEMONSTRATION PROGRAMM --                    *
 *                                                               *
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
 *     in the file razorAHRS.c or on github:                     *    
 *     https://github.com/krueger-christian/razorAHRS            *
 *                                                               *
 ****************************************************************/

#include "razorAHRS.c"

/*-----------------------------------------------------------------*/

int main(int argc, char* argv[]) {
    printf("_________________________________________________\r\n");
    printf("\n    RAZOR AHRS – Headtracker Reader\n");
    printf("_________________________________________________\r\n\n");

    printf("_________________________________________________\r\n");
    printf("\n  !\n\r    CHOOSE RUNNING MODE\n\r");
    printf("       PRESS     C    FOR CONTINUOUS OUTPUT\n\r");
    printf("       PRESS     S    FOR SINGLE FRAME OUTPUT\n\r");
    printf("       PRESS     Q    TO QUIT\n\r");
    printf("      (CONFIRM WITH ENTER)\n\n\n\r");

    char console = 'D';
    int mode;

    while (1) {
        // if new data is available on the console...
        scanf("%c", &console);

        // quit
        if ((console == 'q') || (console == 'Q')) return -1;

		// mode: contonuous streaming
        if ((console == 'c') || (console == 'C')) {
            mode = 0;
            break;
        }
        // mode: read one frame per request
        if ((console == 's') || (console == 'S')) {
            mode = 1;
            break;
        } 
        printf("\n  !\n\r    INVALID INPUT\n\n\r");
    }

    razor_thread_manager* manager = razorAHRS(B57600, argv[1], mode);


    
	if(razorAHRS_start(manager) < 0) return -1;

	pthread_join(manager->thread, NULL);

    printf("\n_________________________________________________\r\n");
    return 0;
}

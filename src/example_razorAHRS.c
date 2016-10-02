/*****************************************************************
 *                                                               *
 *                                                               *
 *               -- DEMONSTRATION PROGRAMM --                    *
 *                                                               *
 *                                                               *
  *                                                               *
 *     C-coded functions to read the the measuring data of       *
 *     the 9 Degree of Measurement Attitude and Heading          *
 *     Reference System of Sparkfun's "9DOF Razor IMU"           *
 *     and "9DOF Sensor Stick"                                   *
 *                                                               *
 *     a former version, used as reference and coded in C++      *
 *     was written by Peter Bartz:                               *
 *     https://github.com/ptrbrtz/razor-9dof-ahrs                *
 *                                                               *
 *     Quality & Usability Lab, TU Berlin                        *
 *     & Deutsche Telekom Laboratories                           *
 *     Christian Krüger                                          *
 *     2016                                                      * 
 *                                                               *
 *     further informations:                                     *
 *     https://github.com/krueger-christian/razorAHRS            *
 *                                                               *
 ****************************************************************/

#include "razorAHRS.c"

/*-----------------------------------------------------------------*/

int main (int argc, char *argv[]) {
  printf ("_________________________________________________\r\n");
  printf ("\n    RAZOR AHRS – Headtracker Reader\n");
  printf ("_________________________________________________\r\n\n");

  printf ("_________________________________________________\r\n");
  printf ("\n  !\n\r    CHOOSE RUNNING MODE\n\r");
  printf ("       PRESS     C    FOR CONTINUOUS OUTPUT\n\r");
  printf ("       PRESS     S    FOR SINGLE FRAME OUTPUT\n\r");
  printf ("       PRESS     Q    QUIT\n\r");
  printf ("\n\r");
  printf ("       (CONFIRM WITH ENTER)\n\n\n\r");

  char console = 'D';
  int mode;
  int format;

  while (1) {
    // if new data is available on the console...
    scanf ("%c", &console);

    // quit
    if ((console == 'q') || (console == 'Q'))
      return -1;

    // mode: continuous streaming
    if ((console == 'c') || (console == 'C')) {
      mode = 0;
      break;
    }
    // mode: read one frame per request
    if ((console == 's') || (console == 'S')) {
      mode = 1;
      break;
    }
    printf ("\n  !\n\r    INVALID INPUT\n\n\r");
  }

  printf ("\n  !\n\r    CHOOSE STREAMING FORMAT\n\r");
  printf ("       PRESS     0    FOR  4 BYTE  PER FRAME (integer values)\n\r");
  printf ("       PRESS     1    FOR 12 BYTES PER FRAME (floating point values)\n\r");
  printf ("       PRESS     Q    QUIT\n\r");
  printf ("\n\r");
  printf ("       (CONFIRM WITH ENTER)\n\n\n\r");

  while (1) {
    // if new data is available on the console...
    scanf ("%c", &console);

    // quit
    if ((console == 'q') || (console == 'Q'))
      return -1;

    // mode: continuous streaming
    if (console == '0') {
      format = 0;
      break;
    }
    // mode: read one frame per request
    if (console == '1') {
      format = 1;
      break;
    }
    //  printf("\n  !\n\r    INVALID INPUT\n\n\r");
  }
  /* creating a management structure containing
   * a setting structure and a data structure as well as
   * mutexes to ensure thread save use
   */
  razor_thread_manager *manager = razorAHRS (B57600, argv[1], mode, format);

  // starting the tracker
  if (razorAHRS_start (manager) < 0)
    return -1;

  // starting a printer to output the data on standart output
  pthread_t printer;
  razorPrinter_start (manager, &printer);

  console = 'D';

  if (mode == 0) {
    while (manager->razor_is_running) {
      if (read (STDIN_FILENO, &console, 1) > 0) {
        if (console == '\n')
          razorAHRS_stop (manager);
      }
      razorSleep (20);
    }

    pthread_join (manager->thread, NULL);

  } else if (mode == 1) {
    printf ("\n\n\r");
    printf ("    PRESS      ENTER  FOR SINGLE FRAME OUTPUT\n\r");
    printf ("    PRESS  Q + ENTER  TO QUIT\n\n\r");

    while (manager->razor_is_running) {
      if (read (STDIN_FILENO, &console, 1) > 0) {
        if ((console == 'q') || (console == 'Q'))
          razorAHRS_stop (manager);
        if (console == '\n')
          razorAHRS_request (manager);
      }
      razorSleep (20);
    }
  }


  printf ("\n_________________________________________________\r\n");
  return 0;
}

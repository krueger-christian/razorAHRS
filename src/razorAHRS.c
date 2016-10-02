/*****************************************************************
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

#ifndef RAZORAHRS_C
#define RAZORAHRS_C

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>             // needed for memset
#include <sys/time.h>
#include <pthread.h>

/* includes several types of storing 
 * data and settings as well as the
 * functions
 *   - razorsleep( int milliseconds)
 *   - long elapsed_ms(struct timeval start, struct timeval end),
 *     returning the difference between two points in time
 *   - stdio_Config(),
 *     configuring the standart output (not necessary)
 *   - tio_Config(int tty_fd, speed_t baudRate)
 *     configuring the termios controlled serial output
 *   - resetConfig(struct adjustment *settings)
 */
#include "razorTools.h"

#include "razorBitIO.c"

#include "razorAHRS.h"
#include "config.h"             // includes user settings

#define debug 0

const char *token_reference = "#SYNCH00\r\n";
const int token_length = 10;


/*----------------------------------------------------------------------------------------------------*/

// function to synchronize incoming bytes with the tracker
bool synch (razor_thread_manager * manager) {

  if (manager->settings->messageOn)
    printf ("  !\n    SYNCHRONIZING: ");

  char input = 'D';             // Buffer to store one byte

  // Get some space to store the token we receive after requesting
  char *token;
  token = calloc (token_length, sizeof (char));

  tcflush (manager->settings->tty_fd, TCIFLUSH);

  for (int i = 0; (i <= 10) && (write (manager->settings->tty_fd, "#s00", 4) != 4); i++) {
    razorSleep (20);
  }

  struct timeval t0, t1, t2;
  gettimeofday (&t0, NULL);
  t1 = t0;

  // make sure, control variable is set to 0
  size_t token_pos = 0;

  /* SYNCHRONIZATION
   * Looking for correct token. */
  while (1) {
    // check if input available...
    if (read (manager->settings->tty_fd, &input, 1) > 0) {
      /* ...is the first byte equal to the first
       * character of the reference token */
      if (input == '#') {
        token_pos++;

        token[0] = input;

        // ... first byte matchs, so get the next bytes
        while (token_pos < token_length) {
          if (read (manager->settings->tty_fd, &input, 1) > 0) {
            token[token_pos] = input;
            token_pos++;
          }
        }

        /* if received token is equal to the reference token
         * the variable synchronized is set to true */
        manager->settings->synchronized = (strncmp (token, token_reference, 10) == 0) ? true : false;

        if (manager->settings->synchronized == true) {
          if (manager->settings->messageOn)
            printf ("__okay.\n\n\r");
          free (token);
          return true;
        }
      }
    }

    gettimeofday (&t2, NULL);
    if (elapsed_ms (t1, t2) > 200) {
      // 200ms elapsed since last request and no answer -> request synch again
      // (this happens when DTR is connected and Razor resets on connect)

      tcflush (manager->settings->tty_fd, TCIFLUSH);

      for (int i = 0; (i <= 10) && (write (manager->settings->tty_fd, "#s00", 4) != 4); i++) {
        razorSleep (20);
      }

      t1 = t2;
    }
    if (elapsed_ms (t0, t2) > connect_timeout_ms) {     //timeout?
      manager->settings->synchronized = false;
      if (manager->settings->messageOn)
        printf ("___failed. (time out)\n\n\r"); // TIME OUT!               
      free (token);
      return false;
    }
    token_pos = 0;
  }

}

/*----------------------------------------------------------------------------------------------------*/

bool valueCheck (razor_thread_manager * manager) {

  pthread_mutex_lock (&manager->data_protect);
  pthread_mutex_lock (&manager->settings_protect);

  int yaw = manager->data->values[0];
  int pitch = manager->data->values[1];
  int roll = manager->data->values[2];

  if (yaw < 0)
    yaw *= -1;
  if (pitch < 0)
    pitch *= -1;
  if (roll < 0)
    roll *= -1;

  if ((yaw > 360) || (pitch > 360) || (roll > 360)) {
    if (synch (manager) == false)
      manager->data->data_fail = true;
  } else
    manager->data->data_fail = false;

  pthread_mutex_unlock (&manager->data_protect);
  pthread_mutex_unlock (&manager->settings_protect);

  return !(manager->data->data_fail);
}

/*----------------------------------------------------------------------------------------------------*/

bool readContinuously (razor_thread_manager * manager) {

  struct adjustment *settings = manager->settings;
  struct razorData *data = manager->data;

  char singleByte = 'D';
  int result = 0;
  int bufferInput = 0;
  size_t values_pos = 0;

  // ensure that currently only this function changes razor settings
  pthread_mutex_lock (&manager->settings_protect);

  // enable output continuous stream
  write (settings->tty_fd, "#o1", 3);
  razorSleep (20);

  // define output mode
  if (settings->output_Format == fourbyte)
    write (settings->tty_fd, "#ol", 3);
  else
    write (settings->tty_fd, "#ob", 3);
  razorSleep (20);

  tcflush (manager->settings->tty_fd, TCIFLUSH);

  if (synch (manager) == false) {
    pthread_mutex_unlock (&manager->settings_protect);
    return false;
  }

  while (manager->razor_is_running) {
    result = read (settings->tty_fd, &singleByte, 1);

#if debug
    printf ("Inside buffer: %d \t read bytes: %d\n\r", bufferInput, result);
#endif

    if (result == 1) {
      // ensure that currently only this function changes razor data
      pthread_mutex_lock (&manager->data_protect);
      data->buffer.ch[bufferInput] = singleByte;
      bufferInput++;

      if (bufferInput == 4) {
        if (settings->output_Format == fourbyte) {
          if (dewrappingValues (data->buffer.l, data) == -1) {
            if (synch (manager) == false) {
              pthread_mutex_unlock (&manager->data_protect);
              pthread_mutex_unlock (&manager->settings_protect);
              return false;
            }
          } else
            values_pos = 3;
        } else {
          data->values[values_pos] = data->buffer.f;
          values_pos++;
        }
        bufferInput = 0;
      }
      pthread_mutex_unlock (&manager->data_protect);
    } else if ((result > 1) && manager->settings->messageOn)
      printf ("INFO reading error (more bytes then requested)\r\n");

    // if 3 byte are read put them into the data structure
    if (values_pos == 3) {

      /* before storing the input, check if it fits 
       * in the valid range */
      pthread_mutex_unlock (&manager->settings_protect);
      if (valueCheck (manager) == false)
        return false;
      pthread_mutex_lock (&manager->settings_protect);

      pthread_mutex_lock (&manager->data_protect);

      // signal that the data was updated
      manager->dataUpdated = true;
      pthread_mutex_unlock (&manager->data_protect);
      pthread_cond_broadcast (&manager->data_updated);

      values_pos = 0;
      pthread_mutex_unlock (&manager->settings_protect);
      razorSleep (20);
      pthread_mutex_lock (&manager->settings_protect);
    }
  }

  pthread_mutex_unlock (&manager->settings_protect);
  return true;
}

/*----------------------------------------------------------------------------------------------------*/

bool readSingle (razor_thread_manager * manager) {

  struct adjustment *settings = manager->settings;
  struct razorData *data = manager->data;

  char singleByte = 'D';
  int result = 0;
  int bufferInput = 0;
  size_t values_pos = 0;

  // ensure that currently only this function changes razor settings
  pthread_mutex_lock (&manager->settings_protect);

  // disable output continuous stream
  write (settings->tty_fd, "#o0", 3);
  razorSleep (20);

  // define output mode
  if (settings->output_Format == fourbyte)
    write (settings->tty_fd, "#ol", 3);
  else
    write (settings->tty_fd, "#ob", 3);
  razorSleep (20);

  tcflush (manager->settings->tty_fd, TCIFLUSH);

  if (synch (manager) == false) {
    pthread_mutex_unlock (&manager->settings_protect);
    return false;
  }

  /* variables to store time values
   * used to measure how long 
   * requesting takes */
  struct timeval t0, t1, t2;

  while (manager->razor_is_running) {

    while (!(manager->data->dataRequest)) {
      pthread_cond_wait (&manager->update, &manager->settings_protect);
    }

    // try to send request
    if (write (settings->tty_fd, "#f", 2) != 2) {
      // ... in case of failed sending
      printf ("INFO: unable to send request\n\r");
      pthread_mutex_lock (&manager->data_protect);
      manager->data->dataRequest = false;
      pthread_mutex_unlock (&manager->data_protect);
    }

    gettimeofday (&t0, NULL);
    t1 = t0;

    while (manager->data->dataRequest) {
      result = read (settings->tty_fd, &singleByte, 1);

#if debug
      printf ("Inside buffer: %d \t read bytes: %d\n\r", bufferInput, result);
#endif

      if (result == 1) {
        // ensure that currently only this function changes razor data
        pthread_mutex_lock (&manager->data_protect);
        data->buffer.ch[bufferInput] = singleByte;
        bufferInput++;

        if (bufferInput == 4) {
          if (settings->output_Format == fourbyte) {

            if (dewrappingValues (data->buffer.l, data) == -1) {
#if debug
              printf ("GET: %ld\r\n", data->buffer.l);
              bitprinter (data->buffer.l, 32);
#endif

              if (manager->settings->messageOn)
                printf ("INFO: reading error\n\r");

              if (synch (manager) == false) {
                pthread_mutex_unlock (&manager->settings_protect);
                pthread_mutex_unlock (&manager->data_protect);
                return false;
              }
            } else {
              values_pos = 3;
#if debug
              printf ("GET: %ld\r\n", data->buffer.l);
              bitprinter (data->buffer.l, 32);
#endif
            }
          } else {
            data->values[values_pos] = data->buffer.f;
            values_pos++;
          }
          bufferInput = 0;
        }
        pthread_mutex_unlock (&manager->data_protect);
      } else if ((result > 1) && manager->settings->messageOn)
        printf ("INFO reading error (more bytes then requested)\r\n");

      // if new data is available on the serial port, print it out
      if (values_pos == 3) {
        /* before storing the input, check if it fits 
         * in the valid range */
        pthread_mutex_unlock (&manager->settings_protect);
        if (valueCheck (manager) == false)
          return false;
        pthread_mutex_lock (&manager->settings_protect);

        // signal that the data was updated
        pthread_mutex_lock (&manager->data_protect);
        manager->dataUpdated = true;
        manager->data->dataRequest = false;
        pthread_mutex_unlock (&manager->data_protect);

        pthread_cond_broadcast (&manager->data_updated);

        values_pos = 0;
        pthread_mutex_unlock (&manager->settings_protect);
        razorSleep (20);
        pthread_mutex_lock (&manager->settings_protect);
      }

      gettimeofday (&t2, NULL);

      if (elapsed_ms (t1, t2) > 200) {
        // 200ms elapsed since last request and no answer -> request synch again
        // (this happens when DTR is connected and Razor resets on connect)
        write (settings->tty_fd, "#f", 2);
        values_pos = 0;
        bufferInput = 0;
        t1 = t2;
      }
      // check if time out is reached 
      if (elapsed_ms (t0, t2) > connect_timeout_ms) {
        // TIME OUT!
        if (settings->messageOn)
          printf ("INFO: request failed. (time out)\n\r");
        manager->data->dataRequest = false;
      }
    }
  }

  pthread_mutex_unlock (&manager->settings_protect);

  return true;
}

/*----------------------------------------------------------------------------------------------------*/

void *calibratingRazor (razor_thread_manager * manager) {

  bool calibrating = true;
  bool sending_request = true;

  /* 0: Accelerometer
   * 1: Magnetometer
   * 2: Gyrometer */
  int sensor = 0;

  /* 0: x-axis maximum
   * 1: x-axis minimum
   * 2: y-axis maximum
   * 3: y-axis minimum
   * 4: z-axis maximum
   * 5: z-axis minimum */
  int step = 0;

  /*      xmax  xmin  ymax  ymin  zmax  zmin  ¦  acc
   *      xmax  xmin  ymax  ymin  zmax  zmin  ¦  mag
   *      xave  xave  yave  yave  zave  zave  ¦  gyr
   */
  float calibMat[3][6] = { {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0, 0.0} }

  pthread_mutex_lock (&manager->settings_protect);

  // enable continuous streaming
  write (settings->tty_fd, "#o1", 3);
  razorSleep (20);

  // enable binary streaming
  write (settings->tty_fd, "#ob", 3);
  razorSleep (20);

  tcflush (manager->settings->tty_fd, TCIFLUSH);

  if (synch (manager) == false) {
    pthread_mutex_unlock (&manager->settings_protect);
    return false;
  }


  while (calibrating) {

    if (sending_request) {
      sending_request = false;
      send_request (sensor, step);
    }
    result = read (settings->tty_fd, &singleByte, 1);

    if (result == 1) {
      // ensure that currently only this function changes razor data
      pthread_mutex_lock (&manager->data_protect);
      manager->data->buffer.ch[bufferInput] = singleByte;
      bufferInput++;

      if (bufferInput == 4) {
        calibMat[sensor][step] = manager->data->buffer.f;
        bufferInput = 0;

        // signal that the data was updated
        manager->dataUpdated = true;
        pthread_mutex_unlock (&manager->data_protect);
        pthread_cond_broadcast (&manager->data_updated);
      }
    } else if (result > 1) {
      pthread_mutex_unlock (&manager->settings_protect);
      calibrating = false;      // TODO: maybe return false ?
    }

    if (manager->data_request) {
      step++;
      if (step > 5) {
        step = 0;
        sensor++;
      }
      sending_request = true;
    }
  }

  razorAHRS_quit (manager);
  pthread_exit (NULL);
}

/*----------------------------------------------------------------------------------------------------*/

void *readingRazor (razor_thread_manager * manager) {

  struct adjustment *settings = manager->settings;
  struct razorData *data = manager->data;

  pthread_mutex_lock (&manager->settings_protect);
  if (settings->streaming_Mode == continuous) {
    pthread_mutex_unlock (&manager->settings_protect);
    readContinuously (manager);
  } else if (settings->streaming_Mode == single) {
    pthread_mutex_unlock (&manager->settings_protect);
    readSingle (manager);
  } else {
    pthread_mutex_unlock (&manager->settings_protect);
    printf ("INFO: No streaming mode selected!");
    razorAHRS_quit (manager);
    pthread_exit (NULL);
  }

  razorAHRS_quit (manager);
  pthread_exit (NULL);
}

/*----------------------------------------------------------------------------------------------------*/


/* mode: 0 -> single (frame on request)
 *       1 -> continuous
 *
 * format: 0 -> integer (4 Byte per frame) 
 *         1 -> floating point (12 Byte per frame)
 */
razor_thread_manager *razorAHRS (speed_t baudRate, char *port, int mode, int format) {
  razor_thread_manager *manager;
  manager = (razor_thread_manager *) calloc (1, sizeof (razor_thread_manager));

  // setting description that is used during the whole process
  manager->settings = (struct adjustment *) calloc (1, sizeof (struct adjustment));

  // construction to store the data
  //struct razorData *data;
  manager->data = (struct razorData *) calloc (1, sizeof (struct razorData));

  pthread_mutex_init (&manager->settings_protect, NULL);
  pthread_mutex_init (&manager->data_protect, NULL);
  pthread_cond_init (&manager->data_updated, NULL);
  pthread_cond_init (&manager->update, NULL);

  // saving port id and name in the settings
  manager->settings->tty_fd = open (port, O_RDWR | O_NONBLOCK);
  manager->settings->baudRate = baudRate;
  manager->settings->port = port;
  manager->settings->streaming_Mode = (mode == 1) ? single : continuous;
  manager->settings->output_Format = (format == 1) ? binary : fourbyte;
  manager->settings->messageOn = message;
  manager->dataUpdated = false;

  return manager;
}

/*----------------------------------------------------------------------------------------------------*/

int razorAHRS_calibration (razor_thread_manager * manager) {

  pthread_mutex_lock (&manager->settings_protect);

  //saving current termios configurations of tty_fd
  if (tcgetattr (manger->settings->tty_fd, &manager->settings->old_tio) != 0) {
    manager->settings->tio_config_changed = false;

    /*reactivating the previous configurations of STDOUT_FILENO 
       because of breaking the process */
    resetConfig (manager->settings);

    return -1;
  }

  tio_Config (manager->settings->tty_fd, manager->settings->baudRate);
  manager->settings->tio_config_changed = true;

  pthread_mutex_unlock (&manager->settings_protect);

  manager->razor_is_running = true;

  manager->thread_id = pthread_create (&manager->thread, NULL, (void *) &calibratingRazor, manager);

  return 0;
}

/*----------------------------------------------------------------------------------------------------*/

int razorAHRS_start (razor_thread_manager * manager) {

  struct adjustment *settings = manager->settings;

  pthread_mutex_lock (&manager->settings_protect);

  //saving current termios configurations of tty_fd
  if (tcgetattr (settings->tty_fd, &settings->old_tio) != 0) {
    settings->tio_config_changed = false;
    printf ("INFO: Saving configuration of xx failed.\n\r\
      --> tcgetattr(fd, &old_tio)\r\n", settings->port);

    /*reactivating the previous configurations of STDOUT_FILENO 
       because of breaking the process */
    resetConfig (settings);

    return -1;
  }

  tio_Config (settings->tty_fd, settings->baudRate);
  settings->tio_config_changed = true;

  pthread_mutex_unlock (&manager->settings_protect);

  manager->razor_is_running = true;

  manager->thread_id = pthread_create (&manager->thread, NULL, (void *) &readingRazor, manager);

  return 0;
}

/*----------------------------------------------------------------------------------------------------*/

int razorAHRS_quit (razor_thread_manager * manager) {

  bool err = false;

  // reactivating previous configurations of tty_fd and the STDOUT_FILENO
  pthread_mutex_lock (&manager->settings_protect);

  // reactivate text mode
  write (manager->settings->tty_fd, "#ot", 3);

  resetConfig (manager->settings);
  pthread_mutex_unlock (&manager->settings_protect);

  err = (pthread_mutex_destroy (&manager->settings_protect) != 0) ? true : false;
  err = (pthread_mutex_destroy (&manager->data_protect) != 0) ? true : false;
  err = (pthread_cond_destroy (&manager->data_updated) != 0) ? true : false;

  free (manager->settings);
  free (manager->data);
  return (err) ? -1 : 0;
}

/*----------------------------------------------------------------------------------------------------*/

/*  stops the razor thread
 */
void razorAHRS_stop (razor_thread_manager * manager) {
  manager->razor_is_running = false;
}

/*----------------------------------------------------------------------------------------------------*/

/*  requests a single data frame
 */
int razorAHRS_request (razor_thread_manager * manager) {
  if (manager->settings->streaming_Mode != single)
    return -1;
  pthread_mutex_lock (&(manager->data_protect));
  manager->data->dataRequest = true;
  pthread_mutex_unlock (&(manager->data_protect));
  pthread_cond_broadcast (&(manager->update));
  return 0;
}

/*----------------------------------------------------------------------------------------------------*/

void *razorPrinter (void *args) {

  razor_thread_manager *manager = (razor_thread_manager *) args;

  while ((manager->razor_is_running) && (manager->printer_is_running)) {

    pthread_mutex_lock (&manager->data_protect);
    while (!manager->dataUpdated) {
      pthread_cond_wait (&manager->data_updated, &manager->data_protect);
    }
    pthread_mutex_unlock (&manager->data_protect);

    pthread_mutex_lock (&manager->data_protect);
    printf ("YAW = %6.1f \t PITCH = %6.1f \t ROLL = %6.1f \r\n", manager->data->values[0], manager->data->values[1], manager->data->values[2]);
    manager->dataUpdated = false;
    pthread_mutex_unlock (&manager->data_protect);
  }


  pthread_exit (NULL);
}

/*----------------------------------------------------------------------------------------------------*/

void razorPrinter_start (razor_thread_manager * manager, pthread_t * printer) {
  manager->printer_is_running = true;
  pthread_create (printer, NULL, (void *) &razorPrinter, manager);
}

/*----------------------------------------------------------------------------------------------------*/

int razorPrinter_stop (razor_thread_manager * manager) {
  manager->printer_is_running = false;
  return 0;
}

#endif // RAZORAHRS_C

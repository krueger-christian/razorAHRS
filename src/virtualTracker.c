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

enum STREAMINGMODE { STREAMINGMODE_SINGLE, STREAMINGMODE_CONTINOUS };
enum STREAMINGFORMAT { STREAMINGFORMAT_ASCII, STREAMINGFORMAT_BINARY };

struct vt_adjustment {
  struct termios old_tio;
  struct termios old_stdio;

  enum STREAMINGFORMAT output_Format;
  enum STREAMINGMODE output_Mode;

  char *vt_port;
  int tty_fd;
  int vt_frequency;
  unsigned int waitingTime; //in ms
  speed_t vt_baudRate;
};

/** 
Computes the elapsed time in ms.
*/
unsigned long elapsed_ms (struct timeval start, struct timeval end) {
  return (long) ((end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000);
}

void send_token_sync (struct vt_adjustment setting, unsigned char sync_id1, unsigned char sync_id2) {
  unsigned char sync_message[] = {'#', 'S', 'Y', 'N', 'C', 'H', sync_id1, sync_id2, '\r', '\n'};
  printf ("sending: %s", sync_message);

  write (setting.tty_fd, sync_message, 10); //TODO: Replace 10
}

bool virtualTracker (unsigned int frequency, speed_t baudRate, char *port) {
  //Setup socket
  struct vt_adjustment setting;
  memset (&setting, 0, sizeof (setting));
  setting.vt_port = port;
  setting.vt_baudRate = baudRate;
  setting.vt_frequency = frequency;
  setting.output_Format = STREAMINGFORMAT_ASCII;
  setting.output_Mode = STREAMINGMODE_CONTINOUS;
  setting.waitingTime = 1000 / frequency;

  struct termios stdio;
  memset (&stdio, 0, sizeof (stdio));
  stdio.c_iflag = 0;
  stdio.c_oflag = 0;
  stdio.c_cflag = 0;
  stdio.c_lflag = 0;
  stdio.c_cc[VMIN] = 1;
  stdio.c_cc[VTIME] = 10;
  tcsetattr (STDOUT_FILENO, TCSANOW, &stdio);
  tcsetattr (STDOUT_FILENO, TCSAFLUSH, &stdio);
  fcntl (STDIN_FILENO, F_SETFL, O_NONBLOCK);    // make the reads non-blocking

  setting.tty_fd = open (setting.vt_port, O_RDWR | O_NONBLOCK);

  struct termios tio;
  memset (&tio, 0, sizeof (tio));
  tio.c_iflag = 0;
  tio.c_oflag = 0;
  tio.c_cflag = CS8 | CREAD | CLOCAL;   // 8n1, see termios.h for more information
  tio.c_lflag = 0;
  tio.c_cc[VMIN] = 1;
  tio.c_cc[VTIME] = 10;
  cfsetospeed (&tio, setting.vt_baudRate);
  cfsetispeed (&tio, setting.vt_baudRate);
  tcsetattr (setting.tty_fd, TCSANOW, &tio);

  printf ("Connecting to: %s (%d) \r\n", setting.vt_port, setting.tty_fd);

  //Prepare for writing

  unsigned char read_byte;
  unsigned int read_byte_length = 0;

  bool request = false;

  while (true) {
    read_byte_length = read (setting.tty_fd, &read_byte, 1);

    // INPUT CHECK
    if ((read_byte_length == 1) && (read_byte = '#')) {
      printf ("\r\nreading: %c", read_byte);
      read_byte_length = read (setting.tty_fd, &read_byte, 1);

      if ((read_byte_length == 1) && (read_byte == 's')) {
        printf ("%c\n\r", read_byte);

        // Sync Request
        unsigned char sync_id[] = {' ', ' '};
        read (setting.tty_fd, &sync_id, 2);

        send_token_sync (setting, sync_id[0], sync_id[1]);

      } else if ((read_byte_length == 1) && (read_byte == 'o')) {
        printf ("%c", read_byte);
        read_byte_length = read (setting.tty_fd, &read_byte, 1);

        printf ("%c\r\n", read_byte);

        // set output mode to STREAMINGFORMAT_BINARY (3 * 4 bytes = three floating point values)
        if ((read_byte_length == 1) && (read_byte == 'b'))
          setting.output_Format = STREAMINGFORMAT_BINARY;
        // set output mode to string (STREAMINGFORMAT_ASCII)
        else if ((read_byte_length == 1) && (read_byte == 't'))
          setting.output_Format = STREAMINGFORMAT_ASCII;
        else if ((read_byte_length == 1) && (read_byte == '1'))
          setting.output_Mode = STREAMINGMODE_CONTINOUS;
        else if ((read_byte_length == 1) && (read_byte == '0'))
          setting.output_Mode = STREAMINGMODE_SINGLE;
      } else if ((read_byte_length == 1) && (read_byte == 'f')) {
        printf ("%c", read_byte);
        request = true;
      }
    }

    if (setting.output_Mode == STREAMINGMODE_CONTINOUS || request == true) {
      float data[3];

      ssize_t len = snprintf (NULL, 0, "#YPR=%3.2f,%3.2f,%3.2f\r\n", data[0], data[1], data[2]);
      char output[len];
      sprintf (output, "#YPR=%3.2f,%3.2f,%3.2f\r\n", data[0], data[1], data[2]);

      switch (setting.output_Format) {
        case STREAMINGFORMAT_BINARY:
          write (setting.tty_fd, data, 12); //12? It should be shorter, or?
          break;
        case STREAMINGFORMAT_ASCII:
          write (setting.tty_fd, output, len);
        break;
      }
      request = false;
    }
    usleep (setting.waitingTime * 1000);
  }

  close (setting.tty_fd);

  return true;
}

#ifndef CONFIG_H
#define CONFIG_H

void stdio_Config(){

        struct termios stdio;

        memset(&stdio,0,sizeof(stdio));
        stdio.c_iflag=0;
        stdio.c_oflag=0;
        stdio.c_cflag=0;
        stdio.c_lflag=0;
        stdio.c_cc[VMIN]=1;
        stdio.c_cc[VTIME]=10;
        tcsetattr(STDOUT_FILENO,TCSANOW,&stdio);
        tcsetattr(STDOUT_FILENO,TCSAFLUSH,&stdio);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);       // make the reads non-blocking

}

/*-----------------------------------------------------------------*/

void tio_Config(int tty_fd, speed_t baudRate){

	    struct termios tio;

        memset(&tio,0,sizeof(tio));
        tio.c_iflag=0;
        tio.c_oflag=0;
        tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
        tio.c_lflag=0;
        tio.c_cc[VMIN]=1;
        tio.c_cc[VTIME]=10;
        cfsetospeed(&tio,baudRate);           // 57600 baud
        cfsetispeed(&tio,baudRate);          // 57600 baud
        tcsetattr(tty_fd,TCSANOW,&tio);
}

#endif // CONFIG_H

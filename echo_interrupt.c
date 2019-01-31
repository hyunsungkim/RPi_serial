#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

#define BAUDRATE B1000000

volatile int serialAvailable = 0;

void signal_handler_IO(int status)
{
	serialAvailable = 1;
}


int main()
{
	int fd;
	struct termios newtio;
	struct sigaction saio;
	char buf[256];

	fd = open("/dev/ttyAMA0", O_RDWR|O_NOCTTY);
	if(fd<0) {
		fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
		printf("Make sure you are executing in sudo.\r\n");
	}
	usleep(250000);
	
	memset(&saio, 0, sizeof(saio));	
	saio.sa_handler = signal_handler_IO;
	saio.sa_restorer = NULL;
	sigaction(SIGIO, &saio, NULL);

	fcntl(fd, F_SETOWN, getpid());
	fcntl(fd, F_SETFL, FASYNC);

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = BAUDRATE|CS8|CLOCAL|CREAD;
	newtio.c_iflag = ICRNL;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;
	
//	speed_t baudRate = B1000000;
//	cfsetispeed(&newtio, baudRate);
//	cfsetospeed(&newtio, baudRate);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	while(1) {
		if(serialAvailable) {
			serialAvailable = 0;
			int cnt = read(fd, buf, 256);
			buf[cnt] = '\0';
			write(fd, "echo: ", 6);
			write(fd, buf, cnt);
			write(fd, "\r\n", 2);
			printf("Received: %s\r\n", buf);
		}
	}
	return 0;
}

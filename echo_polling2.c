#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

#define BAUDRATE B1000000

int main()
{
	int fd;
	struct termios newtio;
	struct pollfd poll_handler;
	char buf[256];

	fd = open("/dev/ttyAMA0", O_RDWR|O_NOCTTY);
	if(fd<0) {
		fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
		printf("Make sure you are executing in sudo.\r\n");
	}
	usleep(250000);

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

	poll_handler.fd = fd;
	poll_handler.events = POLLIN|POLLERR;
	poll_handler.revents = 0;

	while(1) {
		if(poll((struct pollfd*)&poll_handler, 1, 1000) > 0) {
			if(poll_handler.revents & POLLIN) {
				int cnt = read(fd, buf, 256);
				buf[cnt] = '\0';
				write(fd, "echo: ", 6);
				write(fd, buf, cnt);
				write(fd, "\r\n", 2);
				printf("Data Received: %s\r\n", buf);
			}
			else if(poll_handler.revents & POLLERR) {
				printf("Error in communication. Abort program\r\n");
				break;
			}
		}
	}
	close(fd);
	return 0;
}

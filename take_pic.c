#include <stdio.h>

int main()
{
	printf("cheeze");
	system("raspistill -w 640 -h 480 -t 10 -o test.jpg");
	return 0;
}

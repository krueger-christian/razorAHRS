#include "razorAHRS.c"

/*-----------------------------------------------------------------*/

int main(int argc,char** argv)
{
	printf("_________________________________________________\r\n");
	printf("\n    RAZOR AHRS – Headtracker Reader\n");
	printf("_________________________________________________\r\n\n");
	
	razorAHRS(B57600, "/dev/ttyUSB0");

	printf("\n_________________________________________________\r\n");

	return 0;
}
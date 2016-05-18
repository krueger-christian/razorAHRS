#include "razorAHRS.c"

/*-----------------------------------------------------------------*/

int main(int argc,char* argv[])
{
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

	while(1){
			// if new data is available on the console...
    		scanf("%c", &console);

			// pressed q or Q --> quit
			if ((console == 'q' ) || (console == 'Q')) return -1;

			// pressed spacebar --> read one frame
			else if((console == 'c') || (console == 'C')) {
				mode = 0;
				break;
			}
			else if((console == 's') || (console == 'S')) {
				mode = 1;
				break;
			}
			else printf("\n  !\n\r    INVALID INPUT\n\n\r");
	}

	if( razorAHRS(B57600, argv[1], mode) == 0) 
	{	
		printf("\n_________________________________________________\r\n");
		return 0;
	}
	else
	{
		printf("\n_________________________________________________\r\n");
		return -1;
	}
}

#include "virtualTracker.c"

int main(int argc,char* argv[]){

	/*
	char c = 'D';
	printf("\n------------------------------\r\n");
	printf("Running virtual tracker? (y/n)\r\n");
	scanf("%c", &c);
	if(c == 'y'){	
		printf("Run...\r\n\n");
		// virtualTracker(frequency, Baudrate, port)
		if( virtualTracker(10, B57600, argv[1]) != 0 )
		{
			printf("\rRunning virtual Tracker failed.\r\n");
			return -1;
		}
		else return 0;
	}
	else if(c == 'n')
	{
		printf("You decided not to run virtual tracker.\r\n");
		return 0;
	}
	else
	{
		printf("invalid answer\r\n");
		return -1;
	}
	*/

	if( virtualTracker(10, B57600, argv[1]) != 0 )
	{
		printf("\rRunning virtual Tracker failed.\r\n");
		return -1;
	}
	else return 0;
	
}

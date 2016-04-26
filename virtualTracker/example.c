#include "virtualTracker.c"

int main(){
	
	char c = 'D';
	printf("\n------------------------------\r\n");
	printf("Running virtual tracker? (y/n)\r\n");
	scanf("%c", &c);
	if(c == 'y'){	
		printf("Run...\r\n\n");
		// virtualTracker(frequency, Baudrate, port)
		if(virtualTracker(10, B57600, "/dev/pts/2") == false) printf("Running virtual Tracker failed.\r\n");
	}
	else if(c == 'n')printf("You decided not to run virtual tracker.\r\n");
	else printf("invalid answer\r\n");
	return 0;

}

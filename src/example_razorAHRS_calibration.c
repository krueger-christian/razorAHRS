
#include "razorAHRS_calibration.c"

int main(int argc,char* argv[]){
	razorAHRS_calibration(B57600, argv[1]);
	return 0;
}

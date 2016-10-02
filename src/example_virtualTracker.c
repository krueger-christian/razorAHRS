#include "virtualTracker.c"

int main(int argc, char* argv[]) {
    if (virtualTracker(20, B57600, argv[1]) != 0) {
        printf("\rRunning virtual Tracker failed.\r\n");
        return -1;
    }
    return 0;
}

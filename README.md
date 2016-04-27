# razorAHRS
Program to read binary data stream of the Razor AHRS headtracker, written in C.

Compile all files with shell command
	$ sh compile.sh
It will compile automatically the source code located at /src an save the binary program files at /bin .

If you want to start the reader razorAHRS connected to a virtual tracker:
	$ sh start.sh
It will 
	1. create a null modem
	2. start a virtual tracker
	3. start the reader and print out the data until you finish by typing spacebar and enter

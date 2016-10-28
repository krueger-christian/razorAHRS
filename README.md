Parser for headtracker razorAHRS
===

Program to read binary data stream of the [RazorAHRS](https://github.com/ptrbrtz/razor-9dof-ahrs) headtracker, written in C.

Contains a parser and a virtual headtracker.

Build
---

```bash
$ cmake . && make
```

Compile manually
---
```bash
$ gcc /src/main_razorAHRS.c -Wall -D_REENTRANT -lpthread -o /bin/main_razorAHRS
```

Run it (the reader is wrapped in a main function to demonstration):
---
```bash
$ ./bin/main_razorAHRS <port name> 
```

or

```bash
$ sh start_razorAHRS.sh
```

Using parser in your code
===

All functions use the struct thread_parameter as argument.

Streaming modes
---
STREAMINGMODE_ONREQUEST
STREAMINGMODE_CONTINUOUS

Streaming formats
---
STREAMINGFORMAT_ASCII		: expecting data as string (undefined frame size)
STREAMINGFORMAT_BINARY_FLOAT	: expecting 3 floating point values (12 byte per frame)
STREAMINGFORMAT_BINARY_CUSTOM	: expecting 3 integer values wrapped in a custum format (4 Byte per frame)

Basic functions
---
Preparing and getting the necessary struct argument (Baudrate is usually 57600):
	$ struct thread_parameter *parameter = razorAHRS(speed_t baudRate, char* port, <streaming mode>, <streaming format> )

starting the reader:
	$ razorAHRS_start(struct_parameter *parameter)
	returns 0 if successful, otherwise -1

stopping the reader:
	$ razorAHRS_quit(struct_parameter *parameter)
	returns 0 if succesful, otherwise -1

requesting a single frame (works only with mode STREAMINGMODE_ONREQUEST):
	$ razorAHRS_request(struct_parameter *parameter)
	returns 0 if successful, otherwise -1


Additional functions
---

provide a value printer on stdout:
	razorPrinter(struct thread_parameter *parameter);

start to print out the read values:
	razorPrinter_start(struct thread_parameter *parameter)

stop to print out the read values:
	razorPrinter_stop(struct thread_parameter *parameter)


Virtual Tracker
---

If you want to start the reader razorAHRS connected to a virtual tracker, make sure you got socat:
```bash
$ sudo apt install socat 
```

```bash
$ sh connect_virtual_ports.sh
```
It will:
1. create two connected ports via Socat
2. print out the port names

Read the printed port names (something like /dev/pts/1) and go on:
```bash
$ ./bin/main_razorAHRSvirtual <first port name, e.g. /dev/pts/1>
```

Because the virtual tracker prints out its values it is recommended to use another terminal for the reader otherwise it is nearly impossible to compare send and received data.
```bash
$ ./bin/main_razorAHRS <second port name, e.g. /dev/pts/2>
```

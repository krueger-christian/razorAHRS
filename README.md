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

Run it:
---
```bash
$ ./bin/main_razorAHRS <port name> 
```

or

```bash
$ sh start_razorAHRS.sh
```




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

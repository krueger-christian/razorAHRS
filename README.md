# Parser for headtracker razorAHRS

##### Program to read binary data stream of the [RazorAHRS](https://github.com/ptrbrtz/razor-9dof-ahrs) headtracker, written in C. Contains a parser and a virtual headtracker.


***
***

## Installation

#### Build
```bash
$ cmake . && make
```

#### Compile manually
```bash
$ gcc /src/main_razorAHRS.c -Wall -D_REENTRANT -lpthread -o /bin/main_razorAHRS
```

## Start

#### Preparing the tracker
Open the file Arduino/Razor_AHRS/Razor_AHRS.ino and select your Hardware under "USER SETUP AREA" / "HARDWARE OPTIONS". (If you have a SparkFun 9DOF Razor IMU and problems to decide wether you have version SEN-10125 or SEN-10736, have a close look at the Magnetometer module that is a little black brick under the printed label "9DOF RAZOR" and right sided to the GND-connector. If this module has 4 pins on each side it is the version SEN-10736, if it has 5 pins it is the version 10125.) Upload the firmware using Arduino.

Maybe you want to calibrate the tracker, you can do it manually. Therefore you can find a good [tutorial](https://github.com/ptrbrtz/razor-9dof-ahrs/wiki/Tutorial#sensor-calibration) by Peter Bartz. The other option is to use a graphical semi-automatic calibration assistent (See _Calibration Assistant_ below ).

#### Run it (the reader is wrapped in a main function to demonstration):
```bash
$ ./bin/main_razorAHRS <port name> 
```

or

```bash
$ sh start_razorAHRS.sh
```

***

## RAZOR commands

##### The tracking system can be controlled via certain commands send via serial as ASCII-Strings. The basic format is: "#<mode><params>" – Set mode ('o'utput,'f'rame,'s'ynchronize) and parameters. The available options are:

#### Streaming output

* __"#o0"__
  * DISABLE continuous streaming output.
  * Also see #f below.
* __"#o1"__
  * ENABLE continuous streaming output.


#### Angles output

* __"#ob"__
  * Output angles in BINARY format 
  * yaw/pitch/roll as binary float, so one output frame is 3x4 = 12 bytes long
* __"#ot"__
  * Output angles in TEXT format
  * Output frames have form like "#YPR=-142.28,-5.38,33.52", 
  * followed by carriage return and line feed "\r\n"
* __"#ol"__
  * Output angles in a custom binary format. Send as 32 bit LONG format.
  * Also see streaming formats below.


#### Sensor calibration
 
* __"#oc"__
  * Go to CALIBRATION output mode.
* __"#on"__
  * When in calibration mode, go on to calibrate NEXT sensor.


#### Sensor data output

* __"#osct"__
  * Output CALIBRATED SENSOR data of all 9 axes in TEXT format.
  * One frame consist of three lines - one for each sensor: acc, mag, gyr.
* __"#osrt"__
  * Output RAW SENSOR data of all 9 axes in TEXT format.
  * One frame consist of three lines - one for each sensor: acc, mag, gyr.
* __"#osbt"__
  * Output BOTH raw and calibrated SENSOR data of all 9 axes in TEXT format.
  * One frame consist of six lines - like #osrt and #osct combined (first RAW, then CALIBRATED).
  * NOTE: This is a lot of number-to-text conversion work for the little 8MHz chip on the Razor boards.
  * In fact it's too much and an output frame rate of 50Hz can not be maintained. #osbb.
* __"#oscb"__
  * Output CALIBRATED SENSOR data of all 9 axes in BINARY format.
  * One frame consist of three 3x3 float values = 36 bytes. Order is: acc x/y/z, mag x/y/z, gyr x/y/z.
* __"#osrb"__
  * Output RAW SENSOR data of all 9 axes in BINARY format.
  * One frame consist of three 3x3 float values = 36 bytes. Order is: acc x/y/z, mag x/y/z, gyr x/y/z.
* __"#osbb"__
  * Output BOTH raw and calibrated SENSOR data of all 9 axes in BINARY format.
  * One frame consist of 2x36 = 72 bytes - like #osrb and #oscb combined (first RAW, then CALIBRATED).


#### Error message output

* __"#oe0"__
  * Disable ERROR message output.
* __"#oe1"__
  * Enable ERROR message output.


#### Requesting single frame

* __"#f"__
  * Request one output frame - useful when continuous output is disabled and updates are
    required in larger intervals only. Though #f only requests one reply, replies are still
    bound to the internal 20ms (50Hz) time raster. So worst case delay that #f can add is 19.99ms.


#### Synchronize
* __"#s<xy>"__
  * Request synch token - useful to find out where the frame boundaries are in a continuous
    binary stream or to see if tracker is present and answering. The tracker will send
    _"#SYNCH<xy>\r\n"_ in response (so it's possible to read using a readLine() function).
    x and y are two mandatory but arbitrary bytes that can be used to find out which request
    the answer belongs to.


***

## Using parser in your code

All functions use the struct thread_parameter as argument.


#### Streaming modes

* STREAMINGMODE_ONREQUEST
* STREAMINGMODE_CONTINUOUS


#### Streaming formats

* STREAMINGFORMAT_ASCII
  * expecting data as string (undefined frame size)
* STREAMINGFORMAT_BINARY_FLOAT
  * expecting 3 floating point values (12 byte per frame)
* STREAMINGFORMAT_BINARY_CUSTOM
  * expecting 3 integer values wrapped in a custum format (4 Byte per frame)


The custom binary format was created to minimize the amount of send data which reduces the power consumption. While beeing connected via cable it doesn't matter. But while using Bluetooth and a battery it extends the time until you have to charge.
The accuracy of the values are theoretically not as good as within the binary floating point format, because the custom format is based on integers. Practically it doesn't matter because the calculated sensor data is not that much precise.


|   BITS    | 31 downto 22 | 21 downto 12 | 11 downto 2 | 1 downto 0
|-----------|--------------|--------------|-------------|-------------
| __VALUE__ | yaw          | pitch        | roll        | checksum



The checksum is equal to the sum of the ones of all values at the first bit position. e.g.:

| VALUE    | decimal | binary      | first bit  |
| -------- | -------:| -----------:|:----------:|
| yaw      |     3   | 00000 00011 |     1      |
| pitch    |    51   | 00001 10011 |     1      |
| roll     |   128   | 00100 00100 |     0      |
|__________|_________|_____________|____________|
| checksum |     2   |          10 | 1+1+0 = 2  |  



### reader functions

* __Preparing the reader__
  * Returns the struct that is used as argument in all other functions.
  * Baudrate is usually 57600.
```C
struct thread_parameter *parameter = razorAHRS(speed_t baudRate, char* port, <streaming mode>, <streaming format> )
```

* __Starting the reader__
  * Returns 0 if successful, otherwise -1
```C
razorAHRS_start(struct_parameter *parameter)
```	

* __Stopping the reader__
  * returns 0 if succesful, otherwise -1
```C
razorAHRS_quit(struct_parameter *parameter)
```

* __Requesting a single frame__
 * ! works only with mode STREAMINGMODE_ONREQUEST !
 * returns 0 if successful, otherwise -1
```C
razorAHRS_request(struct_parameter *parameter)
```


### printing value functions

* __provide a value printer on stdout__
```C
razorPrinter(struct thread_parameter *parameter);
```

* __Starting the printer__
```C
razorPrinter_start(struct thread_parameter *parameter)
```

* __stopping the printer__
```C
razorPrinter_stop(struct thread_parameter *parameter)
```


***

## Calibration Assistant

!!! The following description is not working yet, because the calibration assistant is under construction.

* Make sure, you load the Arduino code on the tracker already (See _preparing the tracker_).
* Connect the tracker to your computer and find out which port is used for it.

```bash
./bin/calibration_assistant <port>
```


***


## Virtual Tracker

Make sure you got socat:
```bash
$ sudo apt install socat 
```

Preparing the ports:
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


***
***

## Credits

__Authors__
* Christian Krüger
* Dennis Guse

__Institution__
* Quality & Usability Lab, Deutsche Telekom Laboratories, TU Berlin

__Date__
* November 2016

__former version (source)__
* [https://github.com/ptrbrtz/razor-9dof-ahrs](https://github.com/ptrbrtz/razor-9dof-ahrs)







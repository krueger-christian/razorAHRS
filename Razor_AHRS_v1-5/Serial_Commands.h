/*--------- SERIAL COMMANDS THAT THE FIRMWARE UNDERSTANDS -----------------------
---------------------------------------------------------------------------------

            OUTPUT - STREAMING MODE
---------------------------------------------------------------------------------
  "#o0"   | DISABLE continuous streaming output. Also see #f below.
----------+----------------------------------------------------------------------
  "#o1"   | ENABLE continuous streaming output.
---------------------------------------------------------------------------------

            OUTPUT - ANGLES FORMAT
---------------------------------------------------------------------------------
  "#ob"   | Output angles in BINARY format (yaw/pitch/roll as binary float, so
          | one output frame is 3x4 = 12 bytes long).
----------+----------------------------------------------------------------------
  "#ot"   | Output angles in TEXT format (Output frames have form like
          | "#YPR=-142.28,-5.38,33.52", followed by carriage return and line
          | feed [\r\n]).
---------------------------------------------------------------------------------

            OUTPUT - SENSOR CALIBRATION
---------------------------------------------------------------------------------
  "#oc"   | Go to CALIBRATION output mode.
----------+----------------------------------------------------------------------
  "#on"   | When in calibration mode, go on to calibrate NEXT sensor.
---------------------------------------------------------------------------------

            OUTPUT - SENSOR DATA
---------------------------------------------------------------------------------
  "#osct" | Output CALIBRATED SENSOR data of all 9 axes in TEXT format. One
          | frame consist of three lines - one for each sensor: acc, mag, gyr.
----------+----------------------------------------------------------------------
  "#osrt" | Output RAW SENSOR data of all 9 axes in TEXT format. One frame
          | consist of three lines - one for each sensor: acc, mag, gyr.
----------+----------------------------------------------------------------------
  "#osbt" | Output BOTH raw and calibrated SENSOR data of all 9 axes in TEXT
          | format. One frame consist of six lines - like #osrt and #osct
          | combined (first RAW, then CALIBRATED).
          | NOTE: This is a lot of number-to-text conversion work for the 
          | little 8MHz chip on the Razor boards. In fact it's too much and an
          | output frame rate of 50Hz can not be maintained. #osbb.
----------+----------------------------------------------------------------------
  "#oscb" | Output CALIBRATED SENSOR data of all 9 axes in BINARY format.
          | One frame consist of three 3x3 float values = 36 bytes. Order is:
          | acc x/y/z, mag x/y/z, gyr x/y/z.
----------+----------------------------------------------------------------------
  "#osrb" | Output RAW SENSOR data of all 9 axes in BINARY format.
          | One frame consist of three 3x3 float values = 36 bytes. Order is: 
          | acc x/y/z, mag x/y/z, gyr x/y/z.
----------+----------------------------------------------------------------------
  "#osbb" | Output BOTH raw and calibrated SENSOR data of all 9 axes in 
          | BINARY format. One frame consist of 2x36 = 72 bytes - like #osrb 
          | and #oscb combined (first RAW, then CALIBRATED).
---------------------------------------------------------------------------------

            OUTPUT - ERROR MESSAGES
---------------------------------------------------------------------------------
  "#oe0"  | Disable ERROR message output.
----------+----------------------------------------------------------------------
  "#oe1"  | Enable ERROR message output.
---------------------------------------------------------------------------------

            REQUEST A FRAME
---------------------------------------------------------------------------------
  "#f"    | Request one output frame - useful when continuous output is disabled 
          | and updates are required in larger intervals only. Though #f only 
          | requests one reply, replies are still bound to the internal 20ms 
          | (50Hz) time raster. So worst case delay that #f can add is 19.99ms.
---------------------------------------------------------------------------------

            SYNCHRONIZE
---------------------------------------------------------------------------------
 "#s<xy>" | Request synch token - useful to find out where the frame boundaries 
          | are in a continuous binary stream or to see if tracker is present 
          | and answering. The tracker will send "#SYNCH<xy>\r\n" in response
          | (so it's possible to read using a readLine() function). x and y are 
          | two mandatory but arbitrary bytes that can be used to find out which
          | request the answer belongs to.
---------------------------------------------------------------------------------

  ("#C" and "#D" - Reserved for communication with optional Bluetooth module.)

  Newline characters are not required. So you could send "#ob#o1#s", which
  would set binary output mode, enable continuous streaming output and request
  a synch token all at once.

  The status LED will be on if streaming output is enabled and off otherwise.

  Byte order of binary output is little-endian: 
  least significant byte comes first.
*/

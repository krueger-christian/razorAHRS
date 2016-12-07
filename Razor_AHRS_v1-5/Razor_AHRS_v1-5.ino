/***************************************************************************************************************
* Razor AHRS Firmware v1.5
* 9 Degree of Measurement Attitude and Heading Reference System
* for Sparkfun "9DOF Razor IMU" (SEN-10125 and SEN-10736)
* and "9DOF Sensor Stick" (SEN-10183, 10321 and SEN-10724)
***************************************************************************************************************/

/*-------------+-----------------------+-------------------------+
 hardware      |    9 DOF RAZOR IMU    |   9 DOF SENSOR STICK    |
---------------+-----------------------+-------------------------+
 version       | SEN-10125 | SEN-10736 | SEN-10183 | SEN-10321   |
---------------+-----------+-----------|-----------+-------------+
 magnetometer  |  HMC5843  | HMC5883L  | HMC5843   | HMC5883L    |
---------------+-------------------------------------------------+
 accelerometer |                    ADXL345                      |
---------------+-------------------------------------------------+
 gyrometer     |                   ITG-3200                      |
---------------+-------------------------------------------------*/

/* 
 *Arduino IDE: Select board "Arduino Pro or Pro Mini (3.3v, 8Mhz) w/ATmega328"
 */

/*
  Axis definition (differs from definition printed on the board!):
    X axis pointing forward (towards the short edge with the connector holes)
    Y axis pointing to the right
    and Z axis pointing down.

  Positive yaw   : clockwise
  Positive roll  : right wing down
  Positive pitch : nose up

  Transformation order: first yaw then pitch then roll.
*/

/* 
 *  a list of serial commands that the firmware understands is included in Serial_Commands.h (see tabs)
 *  listings of the license and the history is included in License_And_History.h (see tabs)
*/



/*****************************************************************/
/*********** USER SETUP AREA! Set your options here! *************/
/*****************************************************************/

// HARDWARE OPTIONS
/*****************************************************************/
// Select your hardware here by uncommenting one line!
//#define HW__VERSION_CODE 10125 // SparkFun "9DOF Razor IMU" version "SEN-10125" (HMC5843 magnetometer)
  #define HW__VERSION_CODE 10736 // SparkFun "9DOF Razor IMU" version "SEN-10736" (HMC5883L magnetometer)
//#define HW__VERSION_CODE 10183 // SparkFun "9DOF Sensor Stick" version "SEN-10183" (HMC5843 magnetometer)
//#define HW__VERSION_CODE 10321 // SparkFun "9DOF Sensor Stick" version "SEN-10321" (HMC5843 magnetometer)
//#define HW__VERSION_CODE 10724 // SparkFun "9DOF Sensor Stick" version "SEN-10724" (HMC5883L magnetometer)


// OUTPUT OPTIONS
/*****************************************************************/
// Set your serial port baud rate used to send out data here!
#define OUTPUT__BAUD_RATE 57600

// Sensor data output interval in milliseconds
// This may not work, if faster than 20ms (=50Hz)
// Code is tuned for 20ms, so better leave it like that
#define OUTPUT__DATA_INTERVAL 20  // in milliseconds

// Output mode definitions (do not change)
#define OUTPUT__MODE_CALIBRATE_SENSORS 0 // Outputs sensor min/max values as text for manual calibration
#define OUTPUT__MODE_ANGLES 1 // Outputs yaw/pitch/roll in degrees
#define OUTPUT__MODE_SENSORS_CALIB 2 // Outputs calibrated sensor values for all 9 axes
#define OUTPUT__MODE_SENSORS_RAW 3 // Outputs raw (uncalibrated) sensor values for all 9 axes
#define OUTPUT__MODE_SENSORS_BOTH 4 // Outputs calibrated AND raw sensor values for all 9 axes
// Output format definitions (do not change)
#define OUTPUT__FORMAT_TEXT 0 // Outputs data as text
#define OUTPUT__FORMAT_BINARY 1 // Outputs data as binary float
#define OUTPUT__FORMAT_FOURBYTE 2 // Outputs data as binary 32 Bit long 
                                  // (31 downto 22: yaw; 21 downto 12: pitch; 11 downto 2: roll; 1 downto 0: checksum)

// Select your startup output mode and format here!
int output_mode = OUTPUT__MODE_ANGLES;
int output_format = OUTPUT__FORMAT_TEXT;

// Select if serial continuous streaming output is enabled per default on startup.
#define OUTPUT__STARTUP_STREAM_ON true  // true or false

// If set true, an error message will be output if we fail to read sensor data.
// Message format: "!ERR: reading <sensor>", followed by "\r\n".
boolean output_errors = false;  // true or false

// Bluetooth
// You can set this to true, if you have a Rovering Networks Bluetooth Module attached.
// The connect/disconnect message prefix of the module has to be set to "#".
// (Refer to manual, it can be set like this: SO,#)
// When using this, streaming output will only be enabled as long as we're connected. That way
// receiver and sender are synchronzed easily just by connecting/disconnecting.
// It is not necessary to set this! It just makes life easier when writing code for
// the receiving side. The Processing test sketch also works without setting this.
// NOTE: When using this, OUTPUT__STARTUP_STREAM_ON has no effect!
#define OUTPUT__HAS_RN_BLUETOOTH false  // true or false


// SENSOR CALIBRATION
/*****************************************************************/
// How to calibrate? Read the tutorial at http://dev.qu.tu-berlin.de/projects/sf-razor-9dof-ahrs
// Put MIN/MAX and OFFSET readings for your board here!

// Angle correction
// Open Processing file Razor_AHRS_test, make sure Ardino Serial Monitor is closed
// point x-axis (connector holes) to screen and press "a"
// point y-axis away from screen, that means hold long right side against (parallel to) screen
// note yaw value
// put the IMU on a plane desk, check with a water level that x- and y-axis of the desk are horizontal
// note the pitch value
// note the roll value 
// float yaw_up = 0; // noted yawn value
// float pitch_up = 0; // noted pitch value
// float roll_up = 0; // noted roll value

// DEBUG OPTIONS
/*****************************************************************/
// When set to true, gyro drift correction will not be applied
#define DEBUG__NO_DRIFT_CORRECTION true
// Print elapsed time after each I/O loop
#define DEBUG__PRINT_LOOP_TIME false
// When set to true, sensor data will be available without correction
#define DEBUG_RAW_SENSORS false


/*****************************************************************/
/****************** END OF USER SETUP AREA!  *********************/
/*****************************************************************/

// Check if hardware version code is defined
#ifndef HW__VERSION_CODE
// Generate compile error
#error YOU HAVE TO SELECT THE HARDWARE YOU ARE USING! See "HARDWARE OPTIONS" in "USER SETUP AREA" at top of Razor_AHRS.ino!
#endif

#include <Wire.h>
#include "License_And_History.h"
#include "Serial_Commands.h"


union long_buffer
{
  byte b[4];
  char c[4];
  long l;
  float f;
};

union short_buffer
{
  byte b[2];
  char c[2];
  short s;
};

// Don't change struct calibration_set- it effects the EEPROM
struct calibration_set
{
  int acc_min[3];
  int acc_max[3];

  int mag_min[3];
  int mag_max[3];

  float gyr[3];
};

// Don't change struct ext_calibration_set - it effects the EEPROM
struct ext_calibration_set
{
  float ellipsoid_center[3];
  float ellipsoid_transform[3][3];
};

// Stuff
#define STATUS_LED_PIN 13  // Pin number of status LED
#define GRAVITY 256.0f // "1G reference" used for DCM filter and accelerometer calibration
#define TO_RAD(x) (x * 0.01745329252)  // *pi/180
#define TO_DEG(x) (x * 57.2957795131)  // *180/pi

int   ACCEL_X_MIN = -250;
int   ACCEL_X_MAX =  250;
int   ACCEL_Y_MIN = -250;
int   ACCEL_Y_MAX =  250;
int   ACCEL_Z_MIN = -250;
int   ACCEL_Z_MAX =  250;
float ACCEL_X_OFFSET = ((ACCEL_X_MIN + ACCEL_X_MAX) / 2.0f);
float ACCEL_Y_OFFSET = ((ACCEL_Y_MIN + ACCEL_Y_MAX) / 2.0f);
float ACCEL_Z_OFFSET = ((ACCEL_Z_MIN + ACCEL_Z_MAX) / 2.0f);
float ACCEL_X_SCALE  = (GRAVITY / (ACCEL_X_MAX - ACCEL_X_OFFSET));
float ACCEL_Y_SCALE  = (GRAVITY / (ACCEL_Y_MAX - ACCEL_Y_OFFSET));
float ACCEL_Z_SCALE  = (GRAVITY / (ACCEL_Z_MAX - ACCEL_Z_OFFSET));

int   MAGN_X_MIN = -500;
int   MAGN_X_MAX =  500;
int   MAGN_Y_MIN = -500;
int   MAGN_Y_MAX =  500;
int   MAGN_Z_MIN = -500;
int   MAGN_Z_MAX =  500;
float MAGN_X_OFFSET = ((MAGN_X_MIN + MAGN_X_MAX) / 2.0f);
float MAGN_Y_OFFSET = ((MAGN_Y_MIN + MAGN_Y_MAX) / 2.0f);
float MAGN_Z_OFFSET = ((MAGN_Z_MIN + MAGN_Z_MAX) / 2.0f);
float MAGN_X_SCALE  = (100.0f / (MAGN_X_MAX - MAGN_X_OFFSET));
float MAGN_Y_SCALE  = (100.0f / (MAGN_Y_MAX - MAGN_Y_OFFSET));
float MAGN_Z_SCALE  = (100.0f / (MAGN_Z_MAX - MAGN_Z_OFFSET));

bool  CALIBRATION__MAGN_USE_EXTENDED = false;
float magn_ellipsoid_center[3] = {0.0, 0.0, 0.0};
float magn_ellipsoid_transform[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  
float GYRO_AVERAGE_OFFSET_X = 0.0;
float GYRO_AVERAGE_OFFSET_Y = 0.0;
float GYRO_AVERAGE_OFFSET_Z = 0.0;

// Gain for gyroscope (ITG-3200)
#define GYRO_GAIN 0.06957 // Same gain on all axes
#define GYRO_SCALED_RAD(x) (x * TO_RAD(GYRO_GAIN)) // Calculate the scaled gyro readings in radians per second

// DCM parameters
#define Kp_ROLLPITCH 0.02f
#define Ki_ROLLPITCH 0.00002f
#define Kp_YAW 1.2f
#define Ki_YAW 0.00002f

// Sensor variables
float accel[3];  // Actually stores the NEGATED acceleration (equals gravity, if board not moving).
float accel_min[3];
float accel_max[3];

float magnetom[3];
float magnetom_min[3];
float magnetom_max[3];
float magnetom_tmp[3];

float gyro[3];
float gyro_average[3];
int gyro_num_samples = 0;

// DCM variables
float MAG_Heading;
float Accel_Vector[3] = {0, 0, 0}; // Store the acceleration in a vector
float Gyro_Vector[3] = {0, 0, 0}; // Store the gyros turn rate in a vector
float Omega_Vector[3] = {0, 0, 0}; // Corrected Gyro_Vector data
float Omega_P[3] = {0, 0, 0}; // Omega Proportional correction
float Omega_I[3] = {0, 0, 0}; // Omega Integrator
float Omega[3] = {0, 0, 0};
float errorRollPitch[3] = {0, 0, 0};
float errorYaw[3] = {0, 0, 0};
float DCM_Matrix[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
float Update_Matrix[3][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
float Temporary_Matrix[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

// Euler angles
float yaw;
float pitch;
float roll;

// DCM timing in the main loop
unsigned long timestamp;
unsigned long timestamp_old;
float G_Dt; // Integration time for DCM algorithm

// More output-state variables
boolean output_stream_on;
boolean output_single_on;
int curr_calibration_sensor = 0;
int calibration_step = 0;
boolean reset_calibration_session_flag = true;
int num_accel_errors = 0;
int num_magn_errors = 0;
int num_gyro_errors = 0;

struct calibration_set* calib_set;
struct ext_calibration_set* ext_calib_set;

void setCalibrationData(char *ID)
{
  getCalibration(ID, calib_set); 
  
  ACCEL_X_MIN = calib_set->acc_min[0];
  ACCEL_X_MAX = calib_set->acc_max[0];
  ACCEL_Y_MIN = calib_set->acc_min[1];
  ACCEL_Y_MAX = calib_set->acc_max[1];
  ACCEL_Z_MIN = calib_set->acc_min[2];
  ACCEL_Z_MAX = calib_set->acc_max[2];
  ACCEL_X_OFFSET = ((ACCEL_X_MIN + ACCEL_X_MAX) / 2.0f);
  ACCEL_Y_OFFSET = ((ACCEL_Y_MIN + ACCEL_Y_MAX) / 2.0f);
  ACCEL_Z_OFFSET = ((ACCEL_Z_MIN + ACCEL_Z_MAX) / 2.0f);
  ACCEL_X_SCALE  = (GRAVITY / (ACCEL_X_MAX - ACCEL_X_OFFSET));
  ACCEL_Y_SCALE  = (GRAVITY / (ACCEL_Y_MAX - ACCEL_Y_OFFSET));
  ACCEL_Z_SCALE  = (GRAVITY / (ACCEL_Z_MAX - ACCEL_Z_OFFSET));
  
  MAGN_X_MIN = calib_set->mag_min[0];
  MAGN_X_MAX = calib_set->mag_max[0];
  MAGN_Y_MIN = calib_set->mag_min[1];
  MAGN_Y_MAX = calib_set->mag_max[1];
  MAGN_Z_MIN = calib_set->mag_min[2];
  MAGN_Z_MAX = calib_set->mag_max[2];
  MAGN_X_OFFSET = ((MAGN_X_MIN + MAGN_X_MAX) / 2.0f);
  MAGN_Y_OFFSET = ((MAGN_Y_MIN + MAGN_Y_MAX) / 2.0f);
  MAGN_Z_OFFSET = ((MAGN_Z_MIN + MAGN_Z_MAX) / 2.0f);
  MAGN_X_SCALE  = (100.0f / (MAGN_X_MAX - MAGN_X_OFFSET));
  MAGN_Y_SCALE  = (100.0f / (MAGN_Y_MAX - MAGN_Y_OFFSET));
  MAGN_Z_SCALE  = (100.0f / (MAGN_Z_MAX - MAGN_Z_OFFSET));
  
  GYRO_AVERAGE_OFFSET_X = calib_set->gyr[0];
  GYRO_AVERAGE_OFFSET_Y = calib_set->gyr[1];
  GYRO_AVERAGE_OFFSET_Z = calib_set->gyr[2];

  free(calib_set);
}

void setExtCalibrationData(char *ID)
{
  getExtMagCalibration(ID, ext_calib_set);
  CALIBRATION__MAGN_USE_EXTENDED = true;
  for(int i=0; i < 3; i++) magn_ellipsoid_center[i] = ext_calib_set->ellipsoid_center[i];
  for(int i=0; i < 3; i++)
    for(int j=0; j < 3; j++)
      magn_ellipsoid_transform[j][i] = ext_calib_set->ellipsoid_transform[j][i];
  free(ext_calib_set);
}

void resetCalibrationData()
{
  ACCEL_X_MIN = -250;
  ACCEL_X_MAX =  250;
  ACCEL_Y_MIN = -250;
  ACCEL_Y_MAX =  250;
  ACCEL_Z_MIN = -250;
  ACCEL_Z_MAX =  250;
  ACCEL_X_OFFSET = ((ACCEL_X_MIN + ACCEL_X_MAX) / 2.0f);
  ACCEL_Y_OFFSET = ((ACCEL_Y_MIN + ACCEL_Y_MAX) / 2.0f);
  ACCEL_Z_OFFSET = ((ACCEL_Z_MIN + ACCEL_Z_MAX) / 2.0f);
  ACCEL_X_SCALE  = (GRAVITY / (ACCEL_X_MAX - ACCEL_X_OFFSET));
  ACCEL_Y_SCALE  = (GRAVITY / (ACCEL_Y_MAX - ACCEL_Y_OFFSET));
  ACCEL_Z_SCALE  = (GRAVITY / (ACCEL_Z_MAX - ACCEL_Z_OFFSET));

  MAGN_X_MIN = -500;
  MAGN_X_MAX =  500;
  MAGN_Y_MIN = -500;
  MAGN_Y_MAX =  500;
  MAGN_Z_MIN = -500;
  MAGN_Z_MAX =  500;
  MAGN_X_OFFSET = ((MAGN_X_MIN + MAGN_X_MAX) / 2.0f);
  MAGN_Y_OFFSET = ((MAGN_Y_MIN + MAGN_Y_MAX) / 2.0f);
  MAGN_Z_OFFSET = ((MAGN_Z_MIN + MAGN_Z_MAX) / 2.0f);
  MAGN_X_SCALE  = (100.0f / (MAGN_X_MAX - MAGN_X_OFFSET));
  MAGN_Y_SCALE  = (100.0f / (MAGN_Y_MAX - MAGN_Y_OFFSET));
  MAGN_Z_SCALE  = (100.0f / (MAGN_Z_MAX - MAGN_Z_OFFSET));
  
  GYRO_AVERAGE_OFFSET_X = 0.0;
  GYRO_AVERAGE_OFFSET_Y = 0.0;
  GYRO_AVERAGE_OFFSET_Z = 0.0;
}

void resetExtCalibrationData()
{
 CALIBRATION__MAGN_USE_EXTENDED = false;
 magn_ellipsoid_center[0] = magn_ellipsoid_center[1] = magn_ellipsoid_center[2] = 0.0;
 for(int i=0; i<3; i++)
    for(int j=0; j<3; j++) magn_ellipsoid_transform[i][j] = 0.0;
}

void storeCalibration(char ID[4])
{
  if(calib_set == NULL) calib_set = (struct calibration_set*) calloc(1, sizeof(struct calibration_set));  

  calib_set->acc_min[0] = ACCEL_X_MIN;
  calib_set->acc_min[1] = ACCEL_Y_MIN;
  calib_set->acc_min[2] = ACCEL_Z_MIN;
  
  calib_set->acc_max[0] = ACCEL_X_MAX;
  calib_set->acc_max[1] = ACCEL_Y_MAX;
  calib_set->acc_max[2] = ACCEL_Z_MAX;

  calib_set->mag_min[0] = MAGN_X_MIN;
  calib_set->mag_min[1] = MAGN_Y_MIN;
  calib_set->mag_min[2] = MAGN_Z_MIN;
  
  calib_set->mag_max[0] = MAGN_X_MAX;
  calib_set->mag_max[1] = MAGN_Y_MAX;
  calib_set->mag_max[2] = MAGN_Z_MAX;

  calib_set->gyr[0] = GYRO_AVERAGE_OFFSET_X;
  calib_set->gyr[1] = GYRO_AVERAGE_OFFSET_Y;
  calib_set->gyr[2] = GYRO_AVERAGE_OFFSET_Z;

  putCalibration(calib_set, ID);
  free(calib_set);

  if(CALIBRATION__MAGN_USE_EXTENDED)
  {
    if(ext_calib_set == NULL) ext_calib_set = (struct ext_calibration_set*) calloc(1, sizeof(struct ext_calibration_set));
    for(int i=0; i<3; i++) ext_calib_set->ellipsoid_center[i] = magn_ellipsoid_center[i];
    for(int i=0; i<3; i++)
      for(int j=0; i<3; i++)
        ext_calib_set->ellipsoid_transform[i][j] = magn_ellipsoid_transform[i][j];

    putExtMagCalibration(ext_calib_set, ID);
    free(ext_calib_set);
  }
}

void keepCalibratedValue(int cal_step, int cal_sens)
{
  switch(cal_sens)
  {
    case 0: // accelerometer
      switch(cal_step)
      {
        case 0: // x maximum
          ACCEL_X_MAX = (int) accel_max[0];
          break;
        case 1: // x minimum
          ACCEL_X_MIN = (int) accel_min[0];
          break;
        case 2: // y maximum
          ACCEL_Y_MAX = (int) accel_max[1]; 
          break;
        case 3: // y minimum
          ACCEL_Y_MIN = (int) accel_min[1];
          break;
        case 4: // z maximum
          ACCEL_Z_MAX = (int) accel_max[2];
          break;
        case 5: // z minimum
          ACCEL_X_MIN = (int) accel_min[2];
          break;
      }
      break;
    case 1: // magnetometer
      switch(cal_step)
      {
        case 0: // x maximum
          MAGN_X_MAX = (int) magnetom_max[0];
          break;
        case 1: // x minimum
          MAGN_X_MIN = (int) magnetom_min[0];
          break;
        case 2: // y maximum
          MAGN_Y_MAX = (int) magnetom_max[1];
          break;
        case 3: // y minimum
          MAGN_Y_MIN = (int) magnetom_min[1];
          break;
        case 4: // x maximum
          MAGN_Z_MAX = (int) magnetom_max[2];
          break;
        case 5: // z minimum
          MAGN_Z_MIN = (int) magnetom_min[2];
          break;
      }
      break;
    case 2: // gyrometer
      GYRO_AVERAGE_OFFSET_X = (gyro_average[0] / (float) gyro_num_samples);
      GYRO_AVERAGE_OFFSET_Y = (gyro_average[1] / (float) gyro_num_samples);
      GYRO_AVERAGE_OFFSET_Z = (gyro_average[2] / (float) gyro_num_samples);
      break;
  }
}

void read_sensors()
{
  Read_Gyro(); // Read gyroscope
  Read_Accel(); // Read accelerometer
  Read_Magn(); // Read magnetometer
}

// Read every sensor and record a time stamp
// Init DCM with unfiltered orientation
// TODO re-init global vars?
void reset_sensor_fusion()
{
  float temp1[3];
  float temp2[3];
  float xAxis[] = {1.0f, 0.0f, 0.0f};

  read_sensors();
  timestamp = millis();

  // GET PITCH
  // Using y-z-plane-component/x-component of gravity vector
  pitch = -atan2(accel[0], sqrt(accel[1] * accel[1] + accel[2] * accel[2]));

  // GET ROLL
  // Compensate pitch of gravity vector
  Vector_Cross_Product(temp1, accel, xAxis);
  Vector_Cross_Product(temp2, xAxis, temp1);
  // Normally using x-z-plane-component/y-component of compensated gravity vector
  // roll = atan2(temp2[1], sqrt(temp2[0] * temp2[0] + temp2[2] * temp2[2]));
  // Since we compensated for pitch, x-z-plane-component equals z-component:
  roll = atan2(temp2[1], temp2[2]);

  // GET YAW
  Compass_Heading();
  yaw = MAG_Heading;

  // Init rotation matrix
  init_rotation_matrix(DCM_Matrix, yaw, pitch, roll);
}

// Apply calibration to raw sensor readings
void compensate_sensor_errors()
{
  // Compensate accelerometer error
  accel[0] = (accel[0] - ACCEL_X_OFFSET) * ACCEL_X_SCALE;
  accel[1] = (accel[1] - ACCEL_Y_OFFSET) * ACCEL_Y_SCALE;
  accel[2] = (accel[2] - ACCEL_Z_OFFSET) * ACCEL_Z_SCALE;

  // Compensate magnetometer error
#if CALIBRATION__MAGN_USE_EXTENDED == true
  for (int i = 0; i < 3; i++)
    magnetom_tmp[i] = magnetom[i] - magn_ellipsoid_center[i];
  Matrix_Vector_Multiply(magn_ellipsoid_transform, magnetom_tmp, magnetom);
#else
  magnetom[0] = (magnetom[0] - MAGN_X_OFFSET) * MAGN_X_SCALE;
  magnetom[1] = (magnetom[1] - MAGN_Y_OFFSET) * MAGN_Y_SCALE;
  magnetom[2] = (magnetom[2] - MAGN_Z_OFFSET) * MAGN_Z_SCALE;
#endif

  // Compensate gyroscope error
  gyro[0] -= GYRO_AVERAGE_OFFSET_X;
  gyro[1] -= GYRO_AVERAGE_OFFSET_Y;
  gyro[2] -= GYRO_AVERAGE_OFFSET_Z;
}

// Reset calibration session if reset_calibration_session_flag is set
void check_reset_calibration_session()
{
  // Raw sensor values have to be read already, but no error compensation applied

  // Reset this calibration session?
  if (!reset_calibration_session_flag) return;

  // Reset acc and mag calibration variables
  for (int i = 0; i < 3; i++) {
    accel_min[i] = accel_max[i] = accel[i];
    magnetom_min[i] = magnetom_max[i] = magnetom[i];
  }

  // Reset gyro calibration variables
  gyro_num_samples = 0;  // Reset gyro calibration averaging
  gyro_average[0] = gyro_average[1] = gyro_average[2] = 0.0f;
 
  reset_calibration_session_flag = false;
}

void turn_output_stream_on()
{
  output_stream_on = true;
  digitalWrite(STATUS_LED_PIN, HIGH);
}

void turn_output_stream_off()
{
  output_stream_on = false;
  digitalWrite(STATUS_LED_PIN, LOW);
}

// Blocks until another byte is available on serial port
unsigned char readChar()
{
  while (Serial.available() < 1) { } // Block
  return Serial.read();
}

void setup()
{
  // Init serial output
  Serial.begin(OUTPUT__BAUD_RATE);

  // Init status LED
  pinMode (STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  // Init sensors
  delay(50);  // Give sensors enough time to start
  I2C_Init();
  Accel_Init();
  Magn_Init();
  Gyro_Init();

  // Read sensors, init DCM algorithm
  delay(20);  // Give sensors enough time to collect data
  reset_sensor_fusion();

  char* lastID = (char*) calloc(4, sizeof(char));
  getLastSetID(lastID);

  setCalibrationData(lastID);
  setExtCalibrationData(lastID);

  free(lastID);
  
  // Init output
#if (OUTPUT__HAS_RN_BLUETOOTH == true) || (OUTPUT__STARTUP_STREAM_ON == false)
  turn_output_stream_off();
#else
  turn_output_stream_on();
#endif
}

// Main loop
void loop()
{
  // Read incoming control messages
  if (Serial.available() >= 2)
  {
    if (Serial.read() == '#') // Start of new control message
    {
      int command = Serial.read(); // Commands
      if (command == 'f') // request one output _f_rame
        output_single_on = true;
      else if (command == 's') // _s_ynch request
      {
        // Read ID
        byte id[2];
        id[0] = readChar();
        id[1] = readChar();

        // Reply with synch message
        Serial.print("#SYNCH");
        Serial.write(id, 2);
        Serial.println();
      }
      else if (command == 'o') // Set _o_utput mode
      {
        char output_param = readChar();
        if (output_param == 'n')  // Calibrate _n_ext sensor
        {
          curr_calibration_sensor = (curr_calibration_sensor + 1) % 3;
          calibration_step = 0;
          reset_calibration_session_flag = true;
        }
        else if (output_param == 'x') // Calibrate next a_x_is
        {
          keepCalibratedValue(calibration_step, curr_calibration_sensor);
          calibration_step++;
          reset_calibration_session_flag = true;
        }
        else if (output_param == 't') // Output angles as _t_ext
        {
          output_mode = OUTPUT__MODE_ANGLES;
          output_format = OUTPUT__FORMAT_TEXT;
        }
        else if (output_param == 'b') // Output angles in _b_inary format
        {
          output_mode = OUTPUT__MODE_ANGLES;
          output_format = OUTPUT__FORMAT_BINARY;
        }
        else if (output_param == 'l') // Output angles in 32 Bit format (= _l_ong format)
        {
          output_mode = OUTPUT__MODE_ANGLES;
          output_format = OUTPUT__FORMAT_FOURBYTE;
        }
        else if (output_param == 'r') // _r_eset current sensor calibration
        {
          if(output_mode == OUTPUT__MODE_CALIBRATE_SENSORS)
          {
            curr_calibration_sensor = 0;
            reset_calibration_session_flag = true;
          }
        }
        else if (output_param == 'c') // Go to _c_alibration mode
        {
          output_mode = OUTPUT__MODE_CALIBRATE_SENSORS;
          calibration_step = 0;
          curr_calibration_sensor = 0;
          reset_calibration_session_flag = true;
        }
        else if (output_param == 's') // Output _s_ensor values
        {
          char values_param = readChar();
          char format_param = readChar();
          if (values_param == 'r')  // Output _r_aw sensor values
            output_mode = OUTPUT__MODE_SENSORS_RAW;
          else if (values_param == 'c')  // Output _c_alibrated sensor values
            output_mode = OUTPUT__MODE_SENSORS_CALIB;
          else if (values_param == 'b')  // Output _b_oth sensor values (raw and calibrated)
            output_mode = OUTPUT__MODE_SENSORS_BOTH;

          if (format_param == 't') // Output values as _t_text
            output_format = OUTPUT__FORMAT_TEXT;
          else if (format_param == 'b') // Output values in _b_inary format
            output_format = OUTPUT__FORMAT_BINARY;
        }
        else if (output_param == '0') // Disable continuous streaming output
        {
          turn_output_stream_off();
          reset_calibration_session_flag = true;
        }
        else if (output_param == '1') // Enable continuous streaming output
        {
          reset_calibration_session_flag = true;
          turn_output_stream_on();
        }
        else if (output_param == 'e') // _e_rror output settings
        {
          char error_param = readChar();
          if (error_param == '0') output_errors = false;
          else if (error_param == '1') output_errors = true;
          else if (error_param == 'c') // get error count
          {
            Serial.print("#AMG-ERR:");
            Serial.print(num_accel_errors); Serial.print(",");
            Serial.print(num_magn_errors); Serial.print(",");
            Serial.println(num_gyro_errors);
          }
        }
      }
      else if(command == 'w')
      {
        byte cal_id[4];
        cal_id[0] = readChar();
        cal_id[1] = readChar();
        cal_id[2] = readChar();
        cal_id[3] = readChar();

        storeCalibration(cal_id);
      }
      else if(command == 'e') // Set _e_llipsoid
      {
          union long_buffer buff;
          for(int i=0; i<3; i++)
          {
            buff.f  = 0.0;
            buff.b[0] = readChar();
            buff.b[1] = readChar();
            buff.b[2] = readChar();
            buff.b[3] = readChar();
            magn_ellipsoid_center[i] = buff.f;
          }
          
          for(int i=0; i<3; i++)
          {
            for(int j=0; j<3; j++)
            {
              buff.f  = 0.0;
              buff.b[0] = readChar();
              buff.b[1] = readChar();
              buff.b[2] = readChar();
              buff.b[3] = readChar();
              magn_ellipsoid_transform[j][i] = buff.f;            
            }
          }
      }
#if OUTPUT__HAS_RN_BLUETOOTH == true
      // Read messages from bluetooth module
      // For this to work, the connect/disconnect message prefix of the module has to be set to "#".
      else if (command == 'C') // Bluetooth "#CONNECT" message (does the same as "#o1")
        turn_output_stream_on();
      else if (command == 'D') // Bluetooth "#DISCONNECT" message (does the same as "#o0")
        turn_output_stream_off();
#endif // OUTPUT__HAS_RN_BLUETOOTH == true
    }
    else
    { } // Skip character
  }

  // Time to read the sensors again?
  if ((millis() - timestamp) >= OUTPUT__DATA_INTERVAL)
  {
    timestamp_old = timestamp;
    timestamp = millis();
    if (timestamp > timestamp_old)
      G_Dt = (float) (timestamp - timestamp_old) / 1000.0f; // Real time of loop run. We use this on the DCM algorithm (gyro integration time)
    else G_Dt = 0;

    // Update sensor readings
    read_sensors();

    if (output_mode == OUTPUT__MODE_CALIBRATE_SENSORS)  // We're in calibration mode
    {
      check_reset_calibration_session();  // Check if this session needs a reset
      if (output_stream_on || output_single_on) output_calibration(curr_calibration_sensor);
    }
    else if (output_mode == OUTPUT__MODE_ANGLES)  // Output angles
    {
      // Apply sensor calibration
      compensate_sensor_errors();

      // Run DCM algorithm
      Compass_Heading(); // Calculate magnetic heading
      Matrix_update();
      Normalize();
      Drift_correction();
      Euler_angles();

      if (output_stream_on || output_single_on) output_angles();
    }
    else  // Output sensor values
    {
      if (output_stream_on || output_single_on) output_sensors();
    }

    output_single_on = false;

#if DEBUG__PRINT_LOOP_TIME == true
    Serial.print("loop time (ms) = ");
    Serial.println(millis() - timestamp);
#endif
  }
#if DEBUG__PRINT_LOOP_TIME == true
  else
  {
    Serial.println("waiting...");
  }
#endif
}

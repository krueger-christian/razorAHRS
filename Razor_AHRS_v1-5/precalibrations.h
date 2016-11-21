#ifndef PRECAL
#define PRECAL

////////////////////////////////////////////////////////////////////
// CALIBRATION DATA FOR DIFFERENT IMU BOARDS AVAILABLE AT QU-LABS //
////////////////////////////////////////////////////////////////////

/////////////
// default //
/////////////

  
  #define ACCEL_X_MIN ((float) -250)
  #define ACCEL_X_MAX ((float) 250)
  #define ACCEL_Y_MIN ((float) -250)
  #define ACCEL_Y_MAX ((float) 250)
  #define ACCEL_Z_MIN ((float) -250)
  #define ACCEL_Z_MAX ((float) 250)

  // "magn x,y,z (min/max) = -511.00/581.00  -516.00/568.00  -489.00/486.00"
  #define MAGN_X_MIN ((float) -500)
  #define MAGN_X_MAX ((float) 500)
  #define MAGN_Y_MIN ((float) -500)
  #define MAGN_Y_MAX ((float) 500)
  #define MAGN_Z_MIN ((float) -500)
  #define MAGN_Z_MAX ((float) 500)
  
  #define CALIBRATION__MAGN_USE_EXTENDED true
  const float magn_ellipsoid_center[3] = {0.0, 0.0, 0.0};
  const float magn_ellipsoid_transform[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  
  #define GYRO_AVERAGE_OFFSET_X ((float) 0.0)
  #define GYRO_AVERAGE_OFFSET_Y ((float) 0.0)
  #define GYRO_AVERAGE_OFFSET_Z ((float) 0.0)

  float yaw_up = 0.0; // noted yawn value
  float pitch_up = 0.0; // noted pitch value
  float roll_up = 0.0; // noted roll value


//////////
// 0520 //
//////////

  /*
  #define ACCEL_X_MIN ((float) -275)
  #define ACCEL_X_MAX ((float) 269)
  #define ACCEL_Y_MIN ((float) -283)
  #define ACCEL_Y_MAX ((float) 287)
  #define ACCEL_Z_MIN ((float) -286)
  #define ACCEL_Z_MAX ((float) 244)

  // "magn x,y,z (min/max) = -511.00/581.00  -516.00/568.00  -489.00/486.00"
  #define MAGN_X_MIN ((float) -255)
  #define MAGN_X_MAX ((float) 566)
  #define MAGN_Y_MIN ((float) -439)
  #define MAGN_Y_MAX ((float) 503)
  #define MAGN_Z_MIN ((float) -465)
  #define MAGN_Z_MAX ((float) 288)
  
  #define CALIBRATION__MAGN_USE_EXTENDED true
  const float magn_ellipsoid_center[3] = {157.716, -47.7701, -88.7542};
  const float magn_ellipsoid_transform[3][3] = {{0.910957, 0.0126340, 0.0110979}, {0.0126340, 0.905756, 0.0220118}, {0.0110979, 0.0220118, 0.992599}};
  
  #define GYRO_AVERAGE_OFFSET_X ((float) -47.5)
  #define GYRO_AVERAGE_OFFSET_Y ((float) 52.6)
  #define GYRO_AVERAGE_OFFSET_Z ((float) 2.95)

  float yaw_up = 0.0; // noted yawn value
  float pitch_up = 0.0; // noted pitch value
  float roll_up = 0.0; // noted roll value
  */


//////////
// 0499 //
//////////

  /*
  #define ACCEL_X_MIN ((float) -288)
  #define ACCEL_X_MAX ((float) 232)
  #define ACCEL_Y_MIN ((float) -251)
  #define ACCEL_Y_MAX ((float) 276)
  #define ACCEL_Z_MIN ((float) -264)
  #define ACCEL_Z_MAX ((float) 250)
  
  #define CALIBRATION__MAGN_USE_EXTENDED true
  const float magn_ellipsoid_center[3] = {226.878, -86.5913, -79.5690};
  const float magn_ellipsoid_transform[3][3] = {{0.884645, 0.0170150, 0.00830839}, {0.0170150, 0.924206, 0.00954216}, {0.00830839, 0.00954216, 0.997819}};
  
  #define GYRO_AVERAGE_OFFSET_X ((float) -60.9)
  #define GYRO_AVERAGE_OFFSET_Y ((float) 55.37)
  #define GYRO_AVERAGE_OFFSET_Z ((float) 14.7)

  float yaw_up = -80; // noted yawn value
  float pitch_up = 22; // noted pitch value
  float roll_up = 154; // noted roll value
  */


   
///////////
// IMU 2 //
///////////

  /*
  #define ACCEL_X_MIN ((float) -280)
  #define ACCEL_X_MAX ((float) 274)
  #define ACCEL_Y_MIN ((float) -300)
  #define ACCEL_Y_MAX ((float) 298)
  #define ACCEL_Z_MIN ((float) -271)
  #define ACCEL_Z_MAX ((float) 255)

  #define CALIBRATION__MAGN_USE_EXTENDED true // IMU 2
  const float magn_ellipsoid_center[3] = {173.289, -41.4755, -84.9911};
  const float magn_ellipsoid_transform[3][3] = {{0.886106, 0.00978716, 0.0179369}, {0.00978716, 0.893935, 0.0192148}, {0.0179369, 0.0192148, 0.993081}};

  #define GYRO_AVERAGE_OFFSET_X ((float) -51.67)
  #define GYRO_AVERAGE_OFFSET_Y ((float) -11.02)
  #define GYRO_AVERAGE_OFFSET_Z ((float) -2.08)
  */



//////////
// 0485 //
//////////

  // calibration for Sennheiser HD485 (Inv.Nr. 0001)
  /*
  #define ACCEL_X_MIN ((float) -273)
  #define ACCEL_X_MAX ((float) 263)
  #define ACCEL_Y_MIN ((float) -269)
  #define ACCEL_Y_MAX ((float) 269)
  #define ACCEL_Z_MIN ((float) -267)
  #define ACCEL_Z_MAX ((float) 252)

  #define CALIBRATION__MAGN_USE_EXTENDED true
  const float magn_ellipsoid_center[3] = {56.5795, -2.71042, 2.62838};
  const float magn_ellipsoid_transform[3][3] = {{0.908602, 0.00707055, -0.00369838}, {0.00707055, 0.923283, 0.00415908}, {-0.00369838, 0.00415908, 0.999653}};

  #define GYRO_AVERAGE_OFFSET_X ((float) -6.99)
  #define GYRO_AVERAGE_OFFSET_Y ((float) -13.42)
  #define GYRO_AVERAGE_OFFSET_Z ((float) -15.99)

  float yaw_up = 7; // noted yawn value
  float pitch_up = 5; // noted pitch value
  float roll_up = 5; // noted roll value
  */

//////////
// 0484 // NOCHT NICHT FERTIG!!!
//////////

  // calibration for Sennheiser HD485 (Inv.Nr. 0001)
  /*
  #define ACCEL_X_MIN ((float) -273)
  #define ACCEL_X_MAX ((float) 263)
  #define ACCEL_Y_MIN ((float) -269)
  #define ACCEL_Y_MAX ((float) 269)
  #define ACCEL_Z_MIN ((float) -267)
  #define ACCEL_Z_MAX ((float) 252)

  #define CALIBRATION__MAGN_USE_EXTENDED true
  const float magn_ellipsoid_center[3] = {56.5795, -2.71042, 2.62838};
  const float magn_ellipsoid_transform[3][3] = {{0.908602, 0.00707055, -0.00369838}, {0.00707055, 0.923283, 0.00415908}, {-0.00369838, 0.00415908, 0.999653}};

  #define GYRO_AVERAGE_OFFSET_X ((float) -6.99)
  #define GYRO_AVERAGE_OFFSET_Y ((float) -13.42)
  #define GYRO_AVERAGE_OFFSET_Z ((float) -15.99)

  float yaw_up = 5; // noted yawn value
  float pitch_up = 0; // noted pitch value
  float roll_up = -1; // noted roll value
  */
# endif  

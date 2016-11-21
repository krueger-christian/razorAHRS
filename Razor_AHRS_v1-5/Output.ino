/* This file is part of the Razor AHRS Firmware */

// adding a number to the 32bit output data format
long addingBits(int number, long  bitarray, int startbit){
  if(number > 360) return bitarray;

  long tmp = (long) number;
  tmp <<= startbit;

  bitarray |= tmp;

  return bitarray;
} 

long wrappingValues(int value_1, int value_2, int value_3){

  int checksum = 0;
  if(value_1 & 1) checksum++;
  if(value_2 & 1) checksum++;
  if(value_3 & 1) checksum++;

  long bitarray = 0;
  bitarray = addingBits(value_1 + 180, bitarray, 22);
  bitarray = addingBits(value_2 + 180, bitarray, 12);
  bitarray = addingBits(value_3 + 180, bitarray,  2);
  bitarray = addingBits(checksum, bitarray, 0);

  return bitarray;
}


// Output angles: yaw, pitch, roll
void output_angles()
{
  float final_yaw = TO_DEG(yaw) - yaw_up;
  float final_pitch = TO_DEG(pitch) - pitch_up;
  float final_roll = TO_DEG(roll) - roll_up;
  
  if (output_format == OUTPUT__FORMAT_BINARY)
  {
    float ypr[3];  
    ypr[0] = final_yaw; // TO_DEG(yaw);
    ypr[1] = final_pitch; // TO_DEG(pitch);
    ypr[2] = final_roll; // TO_DEG(roll);
    Serial.write((byte*) ypr, 12);  // No new-line
  }
  if (output_format == OUTPUT__FORMAT_FOURBYTE)
  {
    buff.l = wrappingValues( (int) final_yaw, (int) final_pitch, (int) final_roll);
    Serial.write(buff.b, 4);  // No new-lines
  }
  else if (output_format == OUTPUT__FORMAT_TEXT)
  {
      Serial.print("#YPR=");
      Serial.print(final_yaw); Serial.print(","); // yaw, around z-axis
      Serial.print(final_pitch); Serial.print(","); // pitch, around y-axis
      Serial.print(final_roll); Serial.println(); // roll, around x-axis
  }
}

void output_calibration(int calibration_sensor)
{
  if(output_format == OUTPUT__FORMAT_BINARY){

    char mode[2];
    if (calibration_sensor == 0)  // Accelerometer
    {
      if(calibration_step == 0) // finding x-maximum
      {
        if (accel[0] > accel_max[0]) accel_max[0] = accel[0];
        //buff.f = accel_max[0];
        //buff.f = 15.5;
        Serial.write((byte*) buff.b, 4);
      }
      else if(calibration_step == 1) // finding x-minimum
      {
        if (accel[0] < accel_min[0]) accel_min[0] = accel[0];
        //buff.f = accel_min[0];
        buff.f = -18.3;
        Serial.write((byte*) buff.b, 4);
      }
      else if(calibration_step == 2) // finding y-maximum
      {
        if (accel[1] > accel_max[1]) accel_max[1] = accel[1];
        buff.f = accel_max[1];
        Serial.write((byte*) buff.b, 4);
      }
      else if(calibration_step == 3) // finding y-minimum
      {
        if (accel[1] < accel_min[1]) accel_min[1] = accel[1];
        buff.f = accel_min[1]; 
        Serial.write((byte*) buff.b, 4);
      }
      if(calibration_step == 4) // finding z-maximum
      {
        if (accel[2] > accel_max[2]) accel_max[2] = accel[2];
        buff.f = accel_max[2];
        Serial.write((byte*) buff.b, 4);
      }
      else if(calibration_step == 5) // finding z-minimum
      {
        if (accel[2] < accel_min[2]) accel_min[2] = accel[2];
        buff.f = accel_min[2];
        Serial.write((byte*) buff.b, 4);
      }
    }
    else if (calibration_sensor == 1)  // Magnetometer
    {
      if(calibration_step == 0) // finding x-maximum
      {
        if (magnetom[0] > magnetom_max[0]) magnetom_max[0] = magnetom[0];
        buff.f = magnetom_max[0];
        Serial.write((byte*) buff.b, 4);
      }
      else if(calibration_step == 1) // finding x-minimum
      {
        if (magnetom[0] < magnetom_min[0]) magnetom_min[0] = magnetom[0];
        buff.f = magnetom_min[0];
        Serial.write((byte*) buff.b, 4);
      }
      else if(calibration_step == 2) // finding y-maximum
      {
        if (magnetom[1] > magnetom_max[1]) magnetom_max[1] = magnetom[1];
        buff.f = magnetom_max[1];
        Serial.write((byte*) buff.b, 4);
      }
      else if(calibration_step == 3) // finding y-minimum
      {
        if (magnetom[1] < magnetom_min[1]) magnetom_min[1] = magnetom[1];
        buff.f = magnetom_min[1];
        Serial.write((byte*) buff.b, 4);
      }
      else if(calibration_step == 4) // finding z-maximum
      {
        if (magnetom[2] > magnetom_max[2]) magnetom_max[2] = magnetom[2];
        buff.f = magnetom_max[2];
        Serial.write((byte*) buff.b, 4);
      }
      else if(calibration_step == 5) // finding z-minimum
      {
        if (magnetom[2] < magnetom_min[2]) magnetom_min[2] = magnetom[2];
        buff.f = magnetom_min[2];
        Serial.write((byte*) buff.b, 4);
      }
    }
    else if (calibration_sensor == 2)  // Gyroscope
    {
      // Average gyro values
      for (int i = 0; i < 3; i++)
        gyro_average[i] += gyro[i];
      gyro_num_samples++;

      float gyr_val[3];
      gyr_val[0] = (gyro_average[0] / (float) gyro_num_samples);
      gyr_val[1] = (gyro_average[1] / (float) gyro_num_samples);
      gyr_val[2] = (gyro_average[2] / (float) gyro_num_samples);
      
      Serial.write((byte*) gyr_val, 12);
    }
  }
  else if(output_format == OUTPUT__FORMAT_TEXT)
  {
    if (calibration_sensor == 0)  // Accelerometer
    {
      // Output MIN/MAX values
      Serial.print("accel x,y,z (min/max) = ");
      for (int i = 0; i < 3; i++) {
        if (accel[i] < accel_min[i]) accel_min[i] = accel[i];
        if (accel[i] > accel_max[i]) accel_max[i] = accel[i];
        Serial.print(accel_min[i]);
        Serial.print("/");
        Serial.print(accel_max[i]);
        if (i < 2) Serial.print("  ");
        else Serial.println();
      }
    }
    else if (calibration_sensor == 1)  // Magnetometer
    {
      // Output MIN/MAX values
      Serial.print("magn x,y,z (min/max) = ");
      for (int i = 0; i < 3; i++) {
        if (magnetom[i] < magnetom_min[i]) magnetom_min[i] = magnetom[i];
        if (magnetom[i] > magnetom_max[i]) magnetom_max[i] = magnetom[i];
        Serial.print(magnetom_min[i]);
        Serial.print("/");
        Serial.print(magnetom_max[i]);
        if (i < 2) Serial.print("  ");
        else Serial.println();
      }
    }
    else if (calibration_sensor == 2)  // Gyroscope
    {
      // Average gyro values
      for (int i = 0; i < 3; i++)
        gyro_average[i] += gyro[i];
      gyro_num_samples++;
        
      // Output current and averaged gyroscope values
      Serial.print("gyro x,y,z (current/average) = ");
      for (int i = 0; i < 3; i++) {
        Serial.print(gyro[i]);
        Serial.print("/");
        Serial.print(gyro_average[i] / (float) gyro_num_samples);
        if (i < 2) Serial.print("  ");
        else Serial.println();
      }
    }
  }
}

void output_sensors_text(char raw_or_calibrated)
{
  Serial.print("#A-"); Serial.print(raw_or_calibrated); Serial.print('=');
  Serial.print(accel[0]); Serial.print(",");
  Serial.print(accel[1]); Serial.print(",");
  Serial.print(accel[2]); Serial.println();

  Serial.print("#M-"); Serial.print(raw_or_calibrated); Serial.print('=');
  Serial.print(magnetom[0]); Serial.print(",");
  Serial.print(magnetom[1]); Serial.print(",");
  Serial.print(magnetom[2]); Serial.println();

  Serial.print("#G-"); Serial.print(raw_or_calibrated); Serial.print('=');
  Serial.print(gyro[0]); Serial.print(",");
  Serial.print(gyro[1]); Serial.print(",");
  Serial.print(gyro[2]); Serial.println();
}

void output_sensors_binary()
{
  Serial.write((byte*) accel, 12);
  Serial.write((byte*) magnetom, 12);
  Serial.write((byte*) gyro, 12);
}

void output_sensors()
{
  if (output_mode == OUTPUT__MODE_SENSORS_RAW)
  {
    if (output_format == OUTPUT__FORMAT_BINARY)
      output_sensors_binary();
    else if (output_format == OUTPUT__FORMAT_TEXT)
      output_sensors_text('R');
  }
  else if (output_mode == OUTPUT__MODE_SENSORS_CALIB)
  {
    // Apply sensor calibration
    compensate_sensor_errors();
    
    if (output_format == OUTPUT__FORMAT_BINARY)
      output_sensors_binary();
    else if (output_format == OUTPUT__FORMAT_TEXT)
      output_sensors_text('C');
  }
  else if (output_mode == OUTPUT__MODE_SENSORS_BOTH)
  {
    if (output_format == OUTPUT__FORMAT_BINARY)
    {
      output_sensors_binary();
      compensate_sensor_errors();
      output_sensors_binary();
    }
    else if (output_format == OUTPUT__FORMAT_TEXT)
    {
      output_sensors_text('R');
      compensate_sensor_errors();
      output_sensors_text('C');
    }
  }
}


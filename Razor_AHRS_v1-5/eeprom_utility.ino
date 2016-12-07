#ifndef EEPROM_UTILITY
#define EEPROM_UTILITY

#include <EEPROM.h>

#define ACC_OFFSET           4
#define MAG_OFFSET           16
#define GYR_OFFSET           28
#define EXT_MAG_CENT_OFFSET  40
#define EXT_MAG_MATR_OFFSET  52

/*  EEPROM STORAGE ORGANISATION
 *   
 *   Each calibration set is sized 76 bytes.
 *   
 *   The first byte (Byte 0) contains the least used set.
 *   Bit 7            Bit 6          Bit 5            Bit 4                                                              Bit 0
 *   +----------------+--------------+----------------+------------------------------------------------------------------------------+
 *   |                |   not used   |    not used    |                              index of last used set                          |
 *   +----------------+--------------+----------------+------------------------------------------------------------------------------+
 *   
 *   
 *   Byte 1          Byte 2           Byte 3         Byte 4          Byte 5           Byte 6          Byte 7             Byte 8
 *   +---------------------------------------------------------------+---------------------------------------------------------------+
 *   |                               ID                              |           ACCEL_MIN_X         |          ACCEL_MAX_X          | 
 *   +-------------------------------+-------------------------------+-------------------------------+-------------------------------+
 *   |           ACCEL_MIN_Y         |          ACCEL_MAX_Y          |           ACCEL_MIN_Z         |          ACCEL_MAX_Z          |
 *   +-------------------------------+-------------------------------+-------------------------------+-------------------------------+
 *   |           MAGNE_MIN_X         |          MAGNE_MAX_X          |           MAGNE_MIN_Y         |          MAGNE_MAX_Y          |
 *   +-------------------------------+-------------------------------+---------------------------------------------------------------+
 *   |           MAGNE_MIN_Z         |          MAGNE_MAX_Z          |                         GYRO_AVERAGE_X                        |  
 *   +---------------------------------------------------------------+---------------------------------------------------------------+
 *   |                         GYRO_AVERAGE_Y                        |                         GYRO_AVERAGE_Z                        |
 *   +---------------------------------------------------------------+---------------------------------------------------------------+
 *   |                 EXTENDED_MAGNETOMETER_CENTER_X                |                 EXTENDED_MAGNETOMETER_CENTER_Y                |  
 *   +---------------------------------------------------------------+---------------------------------------------------------------+
 *   |                 EXTENDED_MAGNETOMETER_CENTER_Z                |                 EXTENDED_MAGNETOMETER_MATRIX_[0][0]           |
 *   +---------------------------------------------------------------+---------------------------------------------------------------+
 *   |                 EXTENDED_MAGNETOMETER_MATRIX_[1][0]           |                 EXTENDED_MAGNETOMETER_MATRIX_[2][0]           |
 *   +---------------------------------------------------------------+---------------------------------------------------------------+
 *   |                 EXTENDED_MAGNETOMETER_MATRIX_[0][1]           |                 EXTENDED_MAGNETOMETER_MATRIX_[1][1]           |
 *   +---------------------------------------------------------------+---------------------------------------------------------------+
 *   |                 EXTENDED_MAGNETOMETER_MATRIX_[2][1]           |                 EXTENDED_MAGNETOMETER_MATRIX_[0][2]           |
 *   +---------------------------------------------------------------+---------------------------------------------------------------+
 *   |                 EXTENDED_MAGNETOMETER_MATRIX_[1][2]           |                 EXTENDED_MAGNETOMETER_MATRIX_[2][2]           |
 *   +---------------------------------------------------------------+---------------------------------------------------------------+
 *                                  
 */

/*
 int ACCEL_X_MIN = -250;
 int ACCEL_X_MAX =  250;
 int ACCEL_Y_MIN = -250;
 int ACCEL_Y_MAX =  250;
 int ACCEL_Z_MIN = -250;
 int ACCEL_Z_MAX =  250;

 int MAGN_X_MIN = -500;
 int MAGN_X_MAX =  500;
 int MAGN_Y_MIN = -500;
 int MAGN_Y_MAX =  500;
 int MAGN_Z_MIN = -500;
 int MAGN_Z_MAX =  500;
  
 bool  CALIBRATION__MAGN_USE_EXTENDED = false;
 float magn_ellipsoid_center[3] = {0.0, 0.0, 0.0};
 float magn_ellipsoid_transform[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};

 float GYRO_AVERAGE_OFFSET_X = 0.0;
 float GYRO_AVERAGE_OFFSET_Y = 0.0;
 float GYRO_AVERAGE_OFFSET_Z = 0.0;
*/

struct id_list
{
  char id[4];
  struct id_list *next;  
};

unsigned short AMOUNT_OF_SETS = (short) (EEPROM.length() / 76);

int getLastSetID(char* ID)
{
  byte  b        =  EEPROM.read(0);
  int   address  =  0;
        address |=  b;
  bool ext_calibrated = (b & 128);
  if((unsigned char) b == 255) return -1;
    
  if(ID == NULL) ID =  (char*) calloc(4, sizeof(char));
  ID[0]    =  (char) EEPROM.read(address);
  ID[1]    =  (char) EEPROM.read(address + 1);
  ID[2]    =  (char) EEPROM.read(address + 2);
  ID[3]    =  (char) EEPROM.read(address + 3);
  return (ext_calibrated) ? 1 : 0;
}


// write calibration set to eeprom
int putCalibration(struct calibration_set *set, char ID[4]){

  if(set == NULL) return -1;
  
  // find free set space
  unsigned char one_byte = 'B';
  bool address_found = false;
  unsigned short address = 0;
  union long_buffer buff;
  union short_buffer short_buff;
  // Does the ID allready exist?
  for(int i = 1; (!address_found) && (i < AMOUNT_OF_SETS); i += 76)
  {
      if(ID[0] == (char) EEPROM.read(i))
        if(ID[1] == (char) EEPROM.read(i+1))
          if(ID[2] == (char) EEPROM.read(i+2))
            if(ID[3] == (char) EEPROM.read(i+3))
            {
              address = i;
              address_found = true;
            }
  }
  // else find free space
  for(int i = 1; (!address_found) && (i < AMOUNT_OF_SETS); i += 76)
  {
      if(0 == (char) EEPROM.read(i))
        if(0 == (char) EEPROM.read(i+1))
          if(0 == (char) EEPROM.read(i+2))
            if(0 == (char) EEPROM.read(i+3))
            {
              address = i;
              address_found = true;
            }
  }  

  if(address_found == false) return -1;

  // write ID
  buff.c[0] = ID[0];
  buff.c[1] = ID[1];
  buff.c[2] = ID[2];
  buff.c[3] = ID[3];
  for(int i = 0; i < 4; i++) EEPROM.write(address + i, buff.b[i]);

  //write acceleroometer data
  for(int i = 0; i < 3; i++)
  {
    short_buff.s = set->acc_min[i];
    EEPROM.write(address + ACC_OFFSET + 4*i, short_buff.b[0]);
    EEPROM.write(address + ACC_OFFSET + 4*i + 1, short_buff.b[1]);
    short_buff.s = set->acc_max[i];
    EEPROM.write(address + ACC_OFFSET + 4*i + 2, short_buff.b[0]);
    EEPROM.write(address + ACC_OFFSET + 4*i + 3, short_buff.b[1]);    
  }

  //write magnetometer data
  for(int i = 0; i < 3; i++)
  {
    short_buff.s = set->mag_min[i];
    EEPROM.write(address + MAG_OFFSET + 4*i, short_buff.b[0]);
    EEPROM.write(address + MAG_OFFSET + 4*i + 1, short_buff.b[1]);
    short_buff.s = set->mag_max[i];
    EEPROM.write(address + MAG_OFFSET + 4*i + 2, short_buff.b[0]);
    EEPROM.write(address + MAG_OFFSET + 4*i + 3, short_buff.b[1]);

  }

  //write gyrometer data
  for(int i = 0; i < 3; i++)
  {
    buff.f = set->gyr[i];
    EEPROM.write(address + GYR_OFFSET + 4*i,     buff.b[0]);
    EEPROM.write(address + GYR_OFFSET + 4*i + 1, buff.b[1]);
    EEPROM.write(address + GYR_OFFSET + 4*i + 2, buff.b[2]);
    EEPROM.write(address + GYR_OFFSET + 4*i + 3, buff.b[3]);
  }
  
  return 0;
}


int putExtMagCalibration(struct ext_calibration_set *set, char* ID)
{
  if((ID == NULL) || (set = NULL)) return -1;
  
  union long_buffer buff;
  // find address of ID's set space
  bool address_found = false;
  unsigned short address = 0;
  // Does the ID allready exist?
  for(int i = 1; (!address_found) && (i < AMOUNT_OF_SETS); i += 76)
  {
      if(ID[0] == (char) EEPROM.read(i))
        if(ID[1] == (char) EEPROM.read(i+1))
          if(ID[2] == (char) EEPROM.read(i+2))
            if(ID[3] == (char) EEPROM.read(i+3))
            {
              address = i;
              address_found = true;
            }
  }
  //else find free space
  for(int i = 1; (!address_found) && (i < AMOUNT_OF_SETS); i += 76)
  {
      if(0 == (char) EEPROM.read(i))
        if(0 == (char) EEPROM.read(i+1))
          if(0 == (char) EEPROM.read(i+2))
            if(0 == (char) EEPROM.read(i+3))
            {
              address = i;
              address_found = true;
            }
  }
  if(address_found == false) return -1;

  //write center data
  for(int i = 0; i < 3; i++)
  {
    buff.f = set->ellipsoid_center[i];
    EEPROM.write(address + EXT_MAG_CENT_OFFSET + 4*i,     buff.b[0]);
    EEPROM.write(address + EXT_MAG_CENT_OFFSET + 4*i + 1, buff.b[1]);
    EEPROM.write(address + EXT_MAG_CENT_OFFSET + 4*i + 2, buff.b[2]);
    EEPROM.write(address + EXT_MAG_CENT_OFFSET + 4*i + 3, buff.b[3]);
  }

  //write transformation matrice data
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      buff.f = set->ellipsoid_transform[j][i];
      EEPROM.write(address + EXT_MAG_MATR_OFFSET + 4*i + 4*j,     buff.b[0]);
      EEPROM.write(address + EXT_MAG_MATR_OFFSET + 4*i + 4*j + 1, buff.b[1]);
      EEPROM.write(address + EXT_MAG_MATR_OFFSET + 4*i + 4*j + 2, buff.b[2]);
      EEPROM.write(address + EXT_MAG_MATR_OFFSET + 4*i + 4*j + 3, buff.b[3]);
    }
  }

  return 0;
}


int getExtMagCalibration(char* ID, struct ext_calibration_set* set)
{  
  if(ID == NULL) return -1;
  
  union long_buffer *buff = (union long_buffer*) calloc(1, sizeof(union long_buffer));
  
  // find address of ID's set space
  bool address_found = false;
  unsigned short address = 0;
  for(int i = 1; (!address_found) && (i < AMOUNT_OF_SETS); i += 76)
  {
      if(ID[0] == (char) EEPROM.read(i))
        if(ID[1] == (char) EEPROM.read(i+1))
          if(ID[2] == (char) EEPROM.read(i+2))
            if(ID[3] == (char) EEPROM.read(i+3))
            {
              address = i;
              address_found = true;
            }
  }

  if(address_found == false) return -1;

  if(set == NULL) set = (struct ext_calibration_set*) calloc(1, sizeof(struct calibration_set));

  //read center data
  for(int i = 0; i < 3; i++)
  {
    buff->b[0] = EEPROM.read(address + EXT_MAG_CENT_OFFSET + 4*i     );
    buff->b[1] = EEPROM.read(address + EXT_MAG_CENT_OFFSET + 4*i + 1 );
    buff->b[2] = EEPROM.read(address + EXT_MAG_CENT_OFFSET + 4*i + 2 );
    buff->b[3] = EEPROM.read(address + EXT_MAG_CENT_OFFSET + 4*i + 3 );
    set->ellipsoid_center[i] = buff->f;
  }

  //read transformation matrice data
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      buff->b[0] = EEPROM.read(address + EXT_MAG_MATR_OFFSET + 4*i + 4*j    );
      buff->b[1] = EEPROM.read(address + EXT_MAG_MATR_OFFSET + 4*i + 4*j + 1 );
      buff->b[2] = EEPROM.read(address + EXT_MAG_MATR_OFFSET + 4*i + 4*j + 2 );
      buff->b[3] = EEPROM.read(address + EXT_MAG_MATR_OFFSET + 4*i + 4*j + 3 );
      set->ellipsoid_transform[j][i] = buff->f;
    }
  }

  return 0;
}



// read calibration set from eeprom
int getCalibration(char ID[4], struct calibration_set *set){
  long_buffer buff;
  short_buffer short_buff;
  unsigned char one_byte = 'B';
  
  // find address of ID's set space
  bool set_space_found = false;
  unsigned short address = 0;
  for(int i = 1; (!set_space_found) && (i < AMOUNT_OF_SETS); i += 76)
  {
      if(ID[0] == (char) EEPROM.read(i))
        if(ID[1] == (char) EEPROM.read(i+1))
          if(ID[2] == (char) EEPROM.read(i+2))
            if(ID[3] == (char) EEPROM.read(i+3))
            {
              address = i;
              set_space_found = true;
            }
  }

  if(set_space_found == false) return -1;

  if(set == NULL) set = (struct calibration_set*) calloc(1, sizeof(struct calibration_set));

  // read acc values
  for(int i = 0; i < 3; i++)
  {
    short_buff.b[0] = EEPROM.read(address + ACC_OFFSET + 4*i);
    short_buff.b[1] = EEPROM.read(address + ACC_OFFSET + 4*i + 1);
    set->acc_min[i] = short_buff.s;
    short_buff.b[0] = EEPROM.read(address + ACC_OFFSET + 4*i + 2);
    short_buff.b[1] = EEPROM.read(address + ACC_OFFSET + 4*i + 3);    
    set->acc_max[i] = short_buff.s;
  }

  // read mag values
  for(int i = 0; i < 3; i++)
  {
    short_buff.b[0] = EEPROM.read(address + MAG_OFFSET + 4*i);
    short_buff.b[1] = EEPROM.read(address + MAG_OFFSET + 4*i + 1);
    set->mag_min[i] = short_buff.s;
    short_buff.b[0] = EEPROM.read(address + MAG_OFFSET + 4*i + 2);
    short_buff.b[1] = EEPROM.read(address + MAG_OFFSET + 4*i + 3);    
    set->mag_max[i] = short_buff.s;
  }

  // read gyr values
  for(int i = 0; i < 3; i++)
  {
    buff.b[0] = EEPROM.read(address + GYR_OFFSET + 4*i);
    buff.b[1] = EEPROM.read(address + GYR_OFFSET + 4*i + 1);
    buff.b[2] = EEPROM.read(address + GYR_OFFSET + 4*i + 2);
    buff.b[3] = EEPROM.read(address + GYR_OFFSET + 4*i + 3);    
    set->gyr[i] = buff.f;
  }
  
  return 0;
}

struct id_list* getIDList()
{
  struct id_list *list = NULL;
  struct id_list *list_root = list;
  char id[4] = {'c', 'h', 'a', 'r'};
  for(int i = 1;i < AMOUNT_OF_SETS; i += 76)
  {
      id[0] = (char) EEPROM.read(i);
      id[1] = (char) EEPROM.read(i+1);
      id[2] = (char) EEPROM.read(i+2);
      id[3] = (char) EEPROM.read(i+3);
      if((id[0] != 0) || (id[1] != 0) || (id[2] != 0) || (id[3] != 0))
      {
        if(list == NULL) list = (struct id_list*) calloc(1, sizeof(struct id_list));
        else
        {
          list->next = (struct id_list*) calloc(1, sizeof(struct id_list));
          list = list->next;
        }
        list->id[0] = id[0];
        list->id[1] = id[1];
        list->id[2] = id[2];
        list->id[3] = id[3];
      }
  }
  
  return list_root;
}

int deleteCalibration(char ID[4])
{
  char id[4] = {'c','h','a','r'};

  for(int i = 1; i < AMOUNT_OF_SETS; i += 76)
  {
      id[0] = (char) EEPROM.read(i);
      id[1] = (char) EEPROM.read(i+1);
      id[2] = (char) EEPROM.read(i+2);
      id[3] = (char) EEPROM.read(i+3);
      if(strcmp(id, ID) == 0)
      {
        for(int j = 0; j < 76; j++) EEPROM.write(i +j, (byte) 0);
        return 0;
      }
  }
  return -1; 
}


  
  
# endif  

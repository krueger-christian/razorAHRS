#include <stdio.h>
#include <stdlib.h>

void bitprinter (long bitarray, int len) {

  int counter = 0;

  for (int i = len - 1; i >= 0; i--) {
    if ((bitarray >> i) & 1)
      printf ("1");
    else
      printf ("0");
    counter++;
    if ((counter % 10) == 0) {
      printf ("  ");
    }
  }

  printf ("\n\r");

}

long addingBits (int number, long bitarray, int startbit) {

  long tmp = number;
  tmp <<= startbit;

  bitarray |= tmp;

  return bitarray;

}

int readingBits (long bitarray, int startbit, int endbit) {
  int number = 0;

  for (int i = startbit; i <= endbit; i++) {
    if ((bitarray >> i) & 1)
      number |= 1 << (i - startbit);
  }

  return number;
}

long wrappingValues (int value_1, int value_2, int value_3) {
  int checksum = 0;
  if (value_1 & 1)
    checksum++;
  if (value_2 & 1)
    checksum++;
  if (value_3 & 1)
    checksum++;

  long bitarray = 0;
  bitarray = addingBits (value_1 + 180, bitarray, 22);
  bitarray = addingBits (value_2 + 180, bitarray, 12);
  bitarray = addingBits (value_3 + 180, bitarray, 2);
  bitarray = addingBits (checksum, bitarray, 0);

  return bitarray;
}

int dewrappingValues (long input, struct razorData *data) {

  int values[3];

  values[0] = readingBits (input, 22, 31);
  values[1] = readingBits (input, 12, 21);
  values[2] = readingBits (input, 2, 11);

  int checksum = 0;
  if (values[0] & 1)
    checksum++;
  if (values[1] & 1)
    checksum++;
  if (values[2] & 1)
    checksum++;

  if (checksum != readingBits (input, 0, 1))
    return -1;

  values[0] -= 180;
  values[1] -= 180;
  values[2] -= 180;
  data->values[0] = (float) values[0];
  data->values[1] = (float) values[1];
  data->values[2] = (float) values[2];

  return 0;
}

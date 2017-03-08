#include "Arduino.h"

#include "common.h"

byte count_digits(int num)
{
  byte count=0;
  while(num)
  {
    num = num / 10;
    count++;
  }
  return count;
}

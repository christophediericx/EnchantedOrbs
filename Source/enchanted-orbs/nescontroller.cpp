#include "Arduino.h"
#include "nescontroller.h"

int nes_clock_pin      = 30;
int nes_latch_pin      = 31;
int nes_data1_pin      = 32; 
int nes_data2_pin      = 33;

void initialize_nes_controllers(void)
{
  pinMode(nes_data1_pin, INPUT);
  pinMode(nes_data2_pin, INPUT); 
  pinMode(nes_clock_pin, OUTPUT);
  pinMode(nes_latch_pin, OUTPUT);
  digitalWrite(nes_clock_pin, LOW);
  digitalWrite(nes_latch_pin, LOW);  
}

void pulsePin(int pin);

byte read_nes_controller(controller c) 
{  
  int nes_data_pin;
  switch (c)
  {
    case controller1:
    {
      nes_data_pin = nes_data1_pin;
      break;  
    }
    case controller2:
    {
      nes_data_pin = nes_data2_pin;
      break;
    }
  }
  
  int result = 0;

  pulsePin(nes_latch_pin);
 
  if (digitalRead(nes_data_pin) == LOW) 
    bitSet(result, NES_A_BUTTON);

  pulsePin(nes_clock_pin);
  if (digitalRead(nes_data_pin) == LOW) 
    bitSet(result, NES_B_BUTTON);
  
  pulsePin(nes_clock_pin);
  if (digitalRead(nes_data_pin) == LOW) 
    bitSet(result, NES_SELECT_BUTTON);

  pulsePin(nes_clock_pin);
  if (digitalRead(nes_data_pin) == LOW) 
    bitSet(result, NES_START_BUTTON);

  pulsePin(nes_clock_pin);
  if (digitalRead(nes_data_pin) == LOW) 
    bitSet(result, NES_UP_BUTTON);
    
  pulsePin(nes_clock_pin);
  if (digitalRead(nes_data_pin) == LOW) 
    bitSet(result, NES_DOWN_BUTTON);

  pulsePin(nes_clock_pin);
  if (digitalRead(nes_data_pin) == LOW) 
    bitSet(result, NES_LEFT_BUTTON);  
    
  pulsePin(nes_clock_pin);
  if (digitalRead(nes_data_pin) == LOW) 
    bitSet(result, NES_RIGHT_BUTTON);

  return result;
}

void pulsePin(int pin)
{
  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);  
}


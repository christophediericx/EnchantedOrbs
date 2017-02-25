#ifndef NESCONTROLLER_H
#define NESCONTROLLER_H

const int NES_A_BUTTON         = 0;
const int NES_B_BUTTON         = 1;
const int NES_SELECT_BUTTON    = 2;
const int NES_START_BUTTON     = 3;
const int NES_UP_BUTTON        = 4;
const int NES_DOWN_BUTTON      = 5;
const int NES_LEFT_BUTTON      = 6;
const int NES_RIGHT_BUTTON     = 7;

enum controller 
{
  controller1,
  controller2
};

void initialize_nes_controllers(void);
byte read_nes_controller(controller);

#endif

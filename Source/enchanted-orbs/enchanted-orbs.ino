#include <SPI.h>
#include <GD.h>

#include "screenmode.h"
#include "gamescreen.h"
#include "titlescreen.h"

mode current_mode;
bool is_initialized = false;

void switch_mode(mode mode)
{
  current_mode = mode;
  is_initialized = false;
}

void setup() 
{
  Serial.begin(9600);
  GD.begin();
  switch_mode(title_screen);
}

void loop() 
{
  void (*screen_init)(void);
  mode (*screen_run)(void);
  mode new_mode;
  
  switch (current_mode)
  {
    case title_screen:
    {
      screen_init = &initialize_titlescreen;
      screen_run = &run_titlescreen;
      break;
    }
    case game_screen:
    {
      screen_init = &initialize_gamescreen;
      screen_run = &run_gamescreen;
      break;
    }
  }

  if (!is_initialized)
  {
    screen_init();
    is_initialized = true;
  }
  new_mode = screen_run();
  switch_mode(new_mode);
}

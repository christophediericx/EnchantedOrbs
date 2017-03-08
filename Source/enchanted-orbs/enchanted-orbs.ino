#include <SPI.h>
#include <GD.h>

#include "common.h"
#include "graphics.h"
#include "nescontroller.h"
#include "screenmode.h"
#include "gamescreen.h"
#include "gameoverscreen.h"
#include "titlescreen.h"

mode current_mode;
bool is_initialized = false;

byte current_level;
uint16_t current_score;

void switch_mode(mode mode)
{
  current_mode = mode;
  is_initialized = false;
}

void setup(void) 
{
  Serial.begin(9600);
  GD.begin();
  initialize_nes_controllers();
  load_sprites();
  switch_mode(title_screen);
}

void loop(void) 
{
  void (*screen_init)(void);
  mode (*screen_run)(void);
  mode new_mode;
  
  switch (current_mode)
  {
    case title_screen:
    {
      screen_init = &initialize_title_screen;
      screen_run = &run_title_screen;
      break;
    }
    case game_screen:
    {
      screen_init = &initialize_game_screen;
      screen_run = &run_game_screen;
      break;
    }
    case game_over_screen:
    {
      screen_init = &initialize_game_over_screen;
      screen_run = &run_game_over_screen;
      break;
    }
  }

  if (!is_initialized)
  {
    screen_init();
    is_initialized = true;
  }
  new_mode = screen_run();
  hide_sprites();
  switch_mode(new_mode);
}

#include "Arduino.h"
#include <SPI.h>
#include <GD.h>

#include "gamescreen.h"
#include "graphics.h"
#include "screenmode.h"

void initialize_gamescreen(void)
{
  load_background(game_screen);
}

mode run_gamescreen(void)
{
  delay(100000);
  return title_screen;
}


#include "Arduino.h"
#include <SPI.h>
#include <GD.h>

#include "gamescreen.h"
#include "graphics.h"
#include "screenmode.h"

void initialize_gamescreen(void)
{
  load_background(game_screen);
  load_sprites(game_screen);
}

mode run_gamescreen(void)
{
  GD.__wstartspr(0);
  for (int anim = 0; anim < 4; anim++)
    gamescreen_draw_sprite(random(400), random(300), anim, 0);
  GD.__end();  
  delay(100000);
  return title_screen;
}


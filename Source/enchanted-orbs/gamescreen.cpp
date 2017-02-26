#include "Arduino.h"
#include <SPI.h>
#include <GD.h>

#include "gamescreen.h"
#include "graphics.h"
#include "screenmode.h"

enum orb_type
{
  green,
  red,
  blue,
  yellow,
  transparant
};

struct orb
{
  int type;
} orbs[84];

void initialize_gamescreen(void)
{
  load_background(game_screen);
  load_sprites(game_screen);
}

void draw_orbs()
{
  int i;
  int row = 0;
  int y = 0;
  int x = 0;
  int posx, posy;
  byte tp;
  for (i = 0; i < 84; i++) 
  {
    if (i % 7 == 0 && i > 1) y++;
    x = i - (y * 7);
    posx = 49 + (x * 18);
    posy = 49 + (y * 18);
    tp = orbs[i].type;
    GD.sprite(i, posx, posy, tp , 0, 0);
  }
}

void initialize_orbs()
{
  int row, col;
  int i = 0;
  for (i = 0; i < 84; i++)
    orbs[i].type = transparant;
}

void set_orb(int x, int y, orb_type tp)
{
  int pos = (y * 7) + x;
  orbs[pos].type = tp;
}

mode run_gamescreen(void)
{
  randomSeed(analogRead(0));
  initialize_orbs();

  int num_rows = 4;
  int col, row;
  int r;
  for (row = 0; row < num_rows; row++)
  {
    for (col = 0; col < 7; col++)
    {
      r = random(0, 4);
      set_orb(col, row, orb_type(r));
    }
  }
  
  while (true)
  {
    GD.waitvblank();
    draw_orbs();
  }
  return title_screen;
}


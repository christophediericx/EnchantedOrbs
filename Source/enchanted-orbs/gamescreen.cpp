#include "Arduino.h"
#include <SPI.h>
#include <GD.h>

#include "audio.h"
#include "gamescreen.h"
#include "graphics.h"
#include "nescontroller.h"
#include "screenmode.h"

int hero_x = 3;
const int HERO_Y = 11;
int last_button_reacted_to;

enum sprite_type
{
  green,
  red,
  blue,
  yellow,
  fullwhite,
  transparent,
  hero
};

enum move_direction
{
  left,
  right
};

struct sprite
{
  int type;
} sprites[85];

void initialize_gamescreen(void)
{
  load_background(game_screen);
  load_sprites(game_screen);
}

void draw_sprites()
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
    tp = sprites[i].type;
    GD.sprite(i, posx, posy, tp , 0, 0);
  }
}

void set_sprite(int x, int y, sprite_type tp)
{
  int pos = (y * 7) + x;
  sprites[pos].type = tp;
}

void initialize_orbs()
{
  int row, col;
  int i = 0;
  for (i = 0; i < 84; i++)
    sprites[i].type = transparent;
}

void initialize_hero()
{
  set_sprite(hero_x, HERO_Y, hero);
}

void randomize_seed()
{
  randomSeed(analogRead(0));  
}

void fill_random_rows(int num_rows)
{
  int col, row;
  int r;
  for (row = 0; row < num_rows; row++)
  {
    for (col = 0; col < 7; col++)
    {
      r = random(0, 4);
      set_sprite(col, row, sprite_type(r));
    }
  }  
}

void move_hero(move_direction md)
{
  switch (md)
  {
    case left:
    {
      if (hero_x > 0)
      {
        set_sprite(hero_x, HERO_Y, transparent);
        hero_x--;
        set_sprite(hero_x, HERO_Y, hero);
      }
      break;
    }
    case right:
    {
      if (hero_x < 6)
      {
        set_sprite(hero_x, HERO_Y, transparent);
        hero_x++;
        set_sprite(hero_x, HERO_Y, hero);
      }      
      break;
    }
  }
}

void react_to_input()
{
  byte controller_state = read_nes_controller(controller1);
  if (!bitRead(controller_state, last_button_reacted_to))
  {
    if (bitRead(controller_state, NES_LEFT_BUTTON)) 
    {
      move_hero(left);
      last_button_reacted_to = NES_LEFT_BUTTON;
    }
    else if (bitRead(controller_state, NES_RIGHT_BUTTON)) 
    {
      move_hero(right);
      last_button_reacted_to = NES_RIGHT_BUTTON;          
    }
    else if (controller_state == 0) 
    {
      last_button_reacted_to = 0;
    }
  }  
}

mode run_gamescreen(void)
{
  unsigned long current_time;
  randomize_seed();
  initialize_orbs();
  initialize_hero();
  fill_random_rows(4);

  while (true)
  {
    GD.waitvblank();

    react_to_input();
    draw_sprites();

    
  }
  return title_screen;
}


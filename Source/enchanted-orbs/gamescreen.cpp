#include "Arduino.h"
#include <SPI.h>
#include <GD.h>

#include "audio.h"
#include "gamescreen.h"
#include "graphics.h"
#include "nescontroller.h"
#include "screenmode.h"

/* 
 *  Sprites  0  -  84     playing field (orbs, player, arrow)
 *  Sprites 85  -  93     characters of level indication (level 1 - level 99)
 *  
 */

int current_level = 1;
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
  hero,
  arrow_head,
  arrow_dot
};

enum move_direction
{
  left,
  right
};

struct sprite
{
  int type;
} sprites[84];

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
  int i;
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

int get_clear_space_above_hero()
{
  int space_for_arrow = 0;
  int y = 0;
  for (y = ((HERO_Y - 1) * 7) + hero_x; y >= 0; y -= 7)
  {
    if (sprites[y].type < 5) break;
    space_for_arrow++;
  }  
  return space_for_arrow;
}

// Clear arrow pointing upwards from hero
void clear_arrow()
{
  int space_for_arrow = get_clear_space_above_hero();
  int pos = 0;
  int i;
  if (space_for_arrow > 0)
  {
    pos = ((HERO_Y - space_for_arrow) * 7) + hero_x;
    while (space_for_arrow > 0)
    {
      sprites[pos].type = transparent;
      pos += 7;
      space_for_arrow--;
    }
  }
}

// Render arrow pointing upwards from hero
void render_arrow()
{
  int space_for_arrow = get_clear_space_above_hero();
  int pos = 0;
  int i;
  if (space_for_arrow > 0)
  {
    pos = ((HERO_Y - space_for_arrow) * 7) + hero_x;
    sprites[pos].type = arrow_head;
    if (space_for_arrow > 1)
    {
      for (i = 0; i < space_for_arrow - 1; i++) 
      {
        pos += 7;
        sprites[pos].type = arrow_dot;  
      }
    }
  }  
}

void move_hero(move_direction md)
{
  // Clear arrowhead pointing upwards
  clear_arrow();
  
  // Clear current hero sprite
  set_sprite(hero_x, HERO_Y, transparent);
  
  // Adjust position
  switch (md)
  {
    case left:
    {
      if (hero_x > 0) hero_x--;
      break;
    }
    case right:
    {
      if (hero_x < 6) hero_x++;
      break;
    }
  }

  // Set hero sprite at new position
  set_sprite(hero_x, HERO_Y, hero);
  render_arrow();
}

void react_to_input()
{
  // We want to avoid repeating input, so we store the 'last_button_reacted_to'
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

void write_text(String text, int len, int spriteIdx, int xpos, int ypos)
{
  int i;
  int c;
  for (i = 0; i < len; i++) 
  {
    c = text[i];
    // Map characters to sprite indexes
    if (c > 64 && c < 94) c -= 56; // A - Z
    else if (c > 47 && c < 58) c -= 11; // 0 - 9
    else if (c == 32) continue; // SPACE
    GD.sprite(spriteIdx + i, xpos + (i * 16), ypos, c, 0, 0);
  }
}

void update_level_text(int level)
{
  String t = "LEVEL ";
  t.concat(level);
  int baseChars = 7;
  if (level > 10) baseChars++;
  write_text(t, baseChars , 85, 225, 32);  
}

void initialize_text() 
{
  update_level_text(current_level);
}

mode run_gamescreen(void)
{
  unsigned long current_time;
  randomize_seed();
  initialize_orbs();
  initialize_hero();
  initialize_text();
  fill_random_rows(5);
  render_arrow();
  
  while (true)
  {
    GD.waitvblank();
    react_to_input();
    draw_sprites();
  }
  return title_screen;
}


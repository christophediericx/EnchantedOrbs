#include "Arduino.h"
#include <SPI.h>
#include <GD.h>

#include "audio.h"
#include "common.h"
#include "gamescreen.h"
#include "graphics.h"
#include "nescontroller.h"
#include "screenmode.h"

/* 
 *  Sprites   0  -   84     playing field (orbs, player, arrow)
 *  Sprites  85  -   93     characters of level indication ("LEVEL XXX") (1-128)
 *  Sprites  94  -   99     characters of score text ("SCORE:")  
 *  Sprites 100  -  104     5 digits of score ("XXXXX")
 */

const byte HERO_Y = 11;
byte hero_x, last_button_reacted_to;

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

void initialize_game_screen(void)
{
  current_level = 1;
  current_score = 0;
  last_button_reacted_to = 0;
  hero_x = 3;
  load_background(game_screen);
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

bool drop_one_row()
{
  // Copy existing rows downwards
  int col, row, pos, r, next_pos;
  byte tp;
  bool is_game_over = false;
  for (row = 11; row >= 0; row--)
  {
    for (col = 0; col < 7; col++)
    {
      pos = (row * 7) + col;
      tp = sprites[pos].type;
      next_pos = pos + 7;

      // don't replace hero sprite
      if (sprites[next_pos].type == hero) continue;
      
      // also, don't copy over transparent sprites
      if (sprites[pos].type == transparent) continue;
      
      sprites[next_pos].type = tp;

      // the game is over if an orb enters the last line
      is_game_over = (next_pos >= (HERO_Y * 7)) && (tp < 5);
      if (is_game_over) return true;      
    }
  }      
  // Now add one random row at the top
  for (col = 0; col < 7; col++)
  {
    r = random(0, 4);
    set_sprite(col, 0, sprite_type(r));
  }
  return false;
}

void update_level_text(byte level)
{
  write_text("LEVEL", 5, 85, 240, 32);
  byte digits = count_digits(level);
  if (digits == 0) digits = 1;

  if (digits < 3) write_text("0", 1, 90, 336, 32);
  if (digits < 2) write_text("0", 1, 91, 352, 32);
  write_text(String(level), digits, 93 - digits, 384 - (digits * 16), 32);  
}

void initialize_score_text()
{
  write_text("SCORE:", 6, 94, 288, 64);
}

void initialize_text() 
{
  update_level_text(current_level);
  initialize_score_text();
}

mode run_game_screen(void)
{
  unsigned long current_time;
  randomize_seed();
  initialize_orbs();
  initialize_hero();
  initialize_text();
  fill_random_rows(5);
  render_arrow();

  int frame_counter = 0;
  bool game_over = false;
  
  while (true)
  {
    GD.waitvblank();
    frame_counter++;

    if (frame_counter == 0x80)
    {
      frame_counter = 0;
      current_score += 54;
      game_over = drop_one_row();  
      if (game_over) return game_over_screen;
    }
    
    react_to_input();
    draw_sprites();
    render_score(current_score, 304, 96, 100);
  }
  return title_screen;
}


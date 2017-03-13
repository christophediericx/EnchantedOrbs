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
 *  Sprites 100  -  105     5 digits of score ("XXXXXX")
 *  Sprites 110  -  129     orbs that are grabbed
 */

const byte SPRITE_OFFSET_GATHER_AREA = 110;

const byte HERO_Y = 11;
const byte ORBS_CLEARED_FOR_LEVEL_UP = 25;
byte hero_x, last_button_reacted_to;
byte orbs_cleared;

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
  arrow_dot,
  transparent_gatherarea = 48
};

enum move_direction
{
  left,
  right
};

struct sprite
{
  int type;
};

struct sprite sprites_playfield[84];
struct sprite sprites_gatherarea[7];

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
  byte i;
  byte row = 0;
  uint16_t y = 0;
  uint16_t x = 0;
  uint16_t posx, posy;
  byte tp;

  // playfield
  for (i = 0; i < 84; i++) 
  {
    if (i % 7 == 0 && i > 1) y++;
    x = i - (y * 7);
    posx = 49 + (x * 18);
    posy = 49 + (y * 18);
    tp = sprites_playfield[i].type;
    GD.sprite(i, posx, posy, tp , 0, 0);
  }

  // gather area
  for (i = 0; i < 7; i++) 
  {
    posx = 49 + (i * 18);
    byte sprite_idx = SPRITE_OFFSET_GATHER_AREA + i;
    tp = sprites_gatherarea[sprite_idx].type;
    GD.sprite(sprite_idx, posx, 283, tp , 0, 0);
  } 
}

void set_sprite_playfield(int x, int y, sprite_type tp)
{
  byte pos = (y * 7) + x;
  sprites_playfield[pos].type = tp;
}

void initialize_playfield()
{
  for (byte i = 0; i < 84; i++)
    sprites_playfield[i].type = transparent;
}

void initialize_gatherarea()
{
  for (byte i = 0; i < 7; i++)
    sprites_gatherarea[SPRITE_OFFSET_GATHER_AREA+i].type = transparent_gatherarea;
}

void initialize_hero()
{
  set_sprite_playfield(hero_x, HERO_Y, hero);
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
      set_sprite_playfield(col, row, sprite_type(r));
    }
  }  
}

byte get_clear_space_above_hero()
{
  byte space_for_arrow = 0;
  byte y = 0;
  for (y = ((HERO_Y - 1) * 7) + hero_x; y >= 0; y -= 7)
  {
    if (sprites_playfield[y].type < 5) break;
    space_for_arrow++;
  }  
  return space_for_arrow;
}

// Clear arrow pointing upwards from hero
void clear_arrow()
{
  byte space_for_arrow = get_clear_space_above_hero();
  int pos = 0;
  int i;
  if (space_for_arrow > 0)
  {
    pos = ((HERO_Y - space_for_arrow) * 7) + hero_x;
    while (space_for_arrow > 0)
    {
      sprites_playfield[pos].type = transparent;
      pos += 7;
      space_for_arrow--;
    }
  }
}

// Render arrow pointing upwards from hero
void render_arrow()
{
  byte space_for_arrow = get_clear_space_above_hero();
  byte pos = 0;
  byte i;
  if (space_for_arrow > 0)
  {
    pos = ((HERO_Y - space_for_arrow) * 7) + hero_x;
    sprites_playfield[pos].type = arrow_head;
    if (space_for_arrow > 1)
    {
      for (i = 0; i < space_for_arrow - 1; i++) 
      {
        pos += 7;
        sprites_playfield[pos].type = arrow_dot;  
      }
    }
  }  
}

void move_hero(move_direction md)
{
  // Clear arrowhead pointing upwards
  clear_arrow();
  
  // Clear current hero sprite
  set_sprite_playfield(hero_x, HERO_Y, transparent);
  
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
  set_sprite_playfield(hero_x, HERO_Y, hero);
  render_arrow();
}

void grab_orb (int pos, byte orbs_to_grab)
{
  sprite_type tp = sprites_playfield[pos].type;

  // animate orb moving down towards hero...
  sprite_type prev_tp = transparent;
  
  while (pos < 70)
  {
    sprites_playfield[pos].type = prev_tp;
    pos += 7;
    prev_tp = sprites_playfield[pos].type;
    sprites_playfield[pos].type = tp;
    draw_sprites();
    GD.waitvblank();
    frame_counter++; 
  }

  // restore last sprite...
  sprites_playfield[pos].type = prev_tp;

  // and shift in the orb to the gather area.
  sprites_gatherarea[116].type = sprites_gatherarea[115].type;
  sprites_gatherarea[115].type = sprites_gatherarea[114].type;
  sprites_gatherarea[114].type = sprites_gatherarea[113].type;
  sprites_gatherarea[113].type = sprites_gatherarea[112].type;
  sprites_gatherarea[112].type = sprites_gatherarea[111].type;
  sprites_gatherarea[111].type = sprites_gatherarea[110].type;
  sprites_gatherarea[110].type = tp;

  draw_sprites();
  GD.waitvblank();
  frame_counter++;   
  Serial.println(pos);
}

void grab_orbs()
{
  sprite_type already_present = sprites_gatherarea[SPRITE_OFFSET_GATHER_AREA].type;
  byte spaces_above_hero = get_clear_space_above_hero();
  byte pos_orb = ((HERO_Y - spaces_above_hero - 1) * 7) + hero_x;
  sprite_type tp_above_hero = sprites_playfield[pos_orb].type;

  byte pos;
  byte orbs_to_grab;
  
  // If what we have is compatible...
  if (already_present == transparent_gatherarea || tp_above_hero == already_present)
  {
    // ... check how many we have ...
    orbs_to_grab = 0;
    for (pos = pos_orb; pos <= 0; pos -=7)
    {
      if (sprites_playfield[pos].type != tp_above_hero) break;
      orbs_to_grab++;
    }
    
    // ... and check it fits in our gathering area
    byte spaces_in_gather_area = 0;
    for (byte i = 0; i < 7; i++)
      if (sprites_gatherarea[SPRITE_OFFSET_GATHER_AREA+i].type == transparent_gatherarea) 
        spaces_in_gather_area++;

    if (orbs_to_grab <= spaces_in_gather_area)
    {
      grab_orb(pos_orb, orbs_to_grab);
    }
    else
    {
      // TODO: play sound
    }
  }
}

void react_to_input()
{
  // We want to avoid repeating input, so we store the 'last_button_reacted_to'
  byte controller_state = read_nes_controller(controller1);
  if (last_button_reacted_to == 255 || !bitRead(controller_state, last_button_reacted_to))
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
    else if (bitRead(controller_state, NES_A_BUTTON))
    {
      grab_orbs();
      last_button_reacted_to = NES_A_BUTTON;
    }
    else if (controller_state == 0) 
    {
      last_button_reacted_to = 255;
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
      tp = sprites_playfield[pos].type;
      next_pos = pos + 7;

      // don't replace hero sprite
      if (sprites_playfield[next_pos].type == hero) continue;
      
      // also, don't copy over transparent sprites
      if (sprites_playfield[pos].type == transparent) continue;
      
      sprites_playfield[next_pos].type = tp;

      // the game is over if an orb enters the last line
      is_game_over = (next_pos >= (HERO_Y * 7)) && (tp < 5);
      if (is_game_over) return true;      
    }
  }      
  // Now add one random row at the top
  for (col = 0; col < 7; col++)
  {
    r = random(0, 4);
    set_sprite_playfield(col, 0, sprite_type(r));
  }
  return false;
}

void update_level(byte level)
{
  write_text("LEVEL", 5, 85, 240, 32);
  byte digits = count_digits(level);
  if (digits == 0) digits = 1;

  if (digits < 3) write_text("0", 1, 90, 336, 32);
  if (digits < 2) write_text("0", 1, 91, 352, 32);
  write_text(String(level), digits, 93 - digits, 384 - (digits * 16), 32);
}

void initialize_score()
{
  write_text("SCORE:", 6, 94, 288, 64);
}

mode run_game_screen(void)
{
  unsigned long current_time;
  randomize_seed();
  initialize_playfield();
  initialize_gatherarea();
  initialize_hero();
  initialize_score();
  update_level(current_level);
  fill_random_rows(3);
  render_arrow();

  orbs_cleared = 0;
  frame_counter = 0;
  bool game_over = false;
  
  while (true)
  {
    GD.waitvblank();
    frame_counter++;

    if (frame_counter == 3000)
    {
      frame_counter = 0;
      orbs_cleared += 12;
      current_score += 110;
      game_over = drop_one_row();  
      if (game_over) return game_over_screen;
    }
    
    react_to_input();

    // Level up if required
    if (orbs_cleared >= ORBS_CLEARED_FOR_LEVEL_UP)
    {
      byte levels_up = orbs_cleared / ORBS_CLEARED_FOR_LEVEL_UP;
      uint16_t new_level = current_level + levels_up;
      if (new_level > 255) new_level = 255;
      update_level(current_level = new_level);
      orbs_cleared = 0;
    }

    render_score(current_score, 288, 96, 100);
    
    draw_sprites();
  }
  return title_screen;
}


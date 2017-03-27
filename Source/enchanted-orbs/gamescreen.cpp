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

void redraw();
void play_sound(int);

void draw_sprites()
{
  byte i;
  byte row = 0;
  uint16_t y = 0;
  uint16_t x = 0;
  uint16_t posx, posy;
  byte tp;

  // Play field
  for (i = 0; i < 84; i++) 
  {
    if (i % 7 == 0 && i > 1) y++;
    x = i - (y * 7);
    posx = 49 + (x * 18);
    posy = 49 + (y * 18);
    tp = sprites_playfield[i].type;
    GD.sprite(i, posx, posy, tp , 0, 0);
  }

  // Gather area
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

void shift_orb_into_gatherarea(sprite_type tp)
{
  sprites_gatherarea[116].type = sprites_gatherarea[115].type;
  sprites_gatherarea[115].type = sprites_gatherarea[114].type;
  sprites_gatherarea[114].type = sprites_gatherarea[113].type;
  sprites_gatherarea[113].type = sprites_gatherarea[112].type;
  sprites_gatherarea[112].type = sprites_gatherarea[111].type;
  sprites_gatherarea[111].type = sprites_gatherarea[110].type;
  sprites_gatherarea[110].type = tp;
}

void grab_orb (int pos, byte orbs_to_grab)
{
  sprite_type tp = sprites_playfield[pos].type;

  // Animate orb moving down towards hero...

  pos = pos - (orbs_to_grab - 1) * 7;

  byte last_pos = 0;

  while (pos < 70)
  {
    sprites_playfield[pos].type = transparent;
    last_pos = pos + (orbs_to_grab * 7);
    if (last_pos < 70)
    {
      sprites_playfield[last_pos].type = tp;
    }
    else
    {
      shift_orb_into_gatherarea(tp);
    }
    redraw();
    pos += 7;
  }
  // Restore last sprite...
  sprites_playfield[pos].type = transparent;
}

void grab_orbs()
{
  sprite_type already_present = sprites_gatherarea[SPRITE_OFFSET_GATHER_AREA].type;
  byte spaces_above_hero = get_clear_space_above_hero();
  byte pos_orb = ((HERO_Y - spaces_above_hero - 1) * 7) + hero_x;
  sprite_type tp_above_hero = sprites_playfield[pos_orb].type;

  byte orbs_to_grab;
  int pos;
  
  // If what we have is compatible...
  if (already_present == transparent_gatherarea || tp_above_hero == already_present)
  {
    // ... check how many we have ...
    orbs_to_grab = 0;
    for (pos = pos_orb; pos >= 0; pos -=7)
    {
      if (sprites_playfield[pos].type != tp_above_hero) break;
      orbs_to_grab++;
    }
    
    // ... and check it fits in our gathering area.
    byte spaces_in_gather_area = 0;
    for (byte i = 0; i < 7; i++)
      if (sprites_gatherarea[SPRITE_OFFSET_GATHER_AREA+i].type == transparent_gatherarea) 
        spaces_in_gather_area++;

    if (orbs_to_grab <= spaces_in_gather_area)
    {
      grab_orb(pos_orb, orbs_to_grab);
    }
    render_arrow();
  }
  else
  {
    play_sound(68);
  }
}

bool drop_one_row()
{
  // Copy existing rows downwards
  int col, row, pos, r, next_pos;
  byte tp;
  bool is_game_over = false;
  for (row = 10; row >= 0; row--)
  {
    for (col = 0; col < 7; col++)
    {
      pos = (row * 7) + col;
      tp = sprites_playfield[pos].type;
      next_pos = pos + 7;

      // Don't replace hero sprite
      if (sprites_playfield[next_pos].type == hero) continue;
      
      // Also, don't copy over transparent sprites
      if (sprites_playfield[pos].type == transparent) continue;
      
      sprites_playfield[next_pos].type = tp;

      // The game is over if any orb would be on the last line
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

void redraw()
{
  draw_sprites();
  render_score(current_score, 288, 96, 100);
  GD.waitvblank();
  frame_counter++;
  audio_counter++;

  if (audio_counter > 48)
  {
    audio_counter = 0;
    clear_audio();
  }  
}

void mark_neighbouring_fields(byte accum[], byte x, byte y, sprite_type tp)
{
  byte pos;
  if (x > 0)
  {
    // Consider orb to the left
    pos = (y * 7) + x - 1;
    if (sprites_playfield[pos].type == tp && accum[pos] == 0)
    {
      accum[pos] = 1;
      mark_neighbouring_fields(accum, x - 1, y, tp);    
    }
    else
    {
      if (accum[pos] == 0) accum[pos] = 2;
    }
  }
  if (x < 6)
  {
    // Consider orb to the right
    pos = (y * 7) + x + 1;
    if (sprites_playfield[pos].type == tp && accum[pos] == 0)
    {
      accum[pos] = 1;
      mark_neighbouring_fields(accum, x + 1, y, tp);
    }
    else
    {
      if (accum[pos] == 0) accum[pos] = 2;
    }
  }
  if (y > 0)
  {
    // Consider orb to the top
    pos = ((y - 1) * 7) + x;
    if (sprites_playfield[pos].type == tp && accum[pos] == 0)
    {
      accum[pos] = 1;
      mark_neighbouring_fields(accum, x, y - 1, tp);
    }
    else
    {
      if (accum[pos] == 0) accum[pos] = 2;
    }
  }
  if (y < 9)
  {
    // Consider orb to the bottom
    pos = ((y + 1) * 7) + x;
    if (sprites_playfield[pos].type == tp && accum[pos] == 0)
    {
      accum[pos] = 1;
      mark_neighbouring_fields(accum, x, y + 1, tp);
    }
    else
    {
      if (accum[pos] == 0) accum[pos] = 2;
    }
  }
}

void collapse_playfield(byte accumulator[])
{
  byte pos, otherpos, tp_counter;
  byte collapsed_col[11];
  sprite_type tp;
    
  for (byte col = 0; col < 7; col++)
  {
    for (byte i = 0; i < 11; i++) 
      collapsed_col[i] = transparent;
      
    byte elements_in_col = 0;
    for (byte row = 0; row < 11; row++)
    {
      pos = (row * 7) + col;
      if (sprites_playfield[pos].type < 5)
        collapsed_col[elements_in_col++] = sprites_playfield[pos].type;
    }
    for (byte i = 0; i < 11; i++)
    {
      pos = (i * 7) + col;   
      tp = sprites_playfield[pos].type;
      sprites_playfield[pos].type = collapsed_col[i];   
      if (tp == transparent && collapsed_col[i] != transparent)
        accumulator[pos] = 1;      
    }
  }
}

void clear_and_score(byte accum[], int sz, int chain)
{
  for (int i = 0; i < sz; i++)
    sprites_playfield[accum[i]].type = fullwhite;

  byte offset = (12 * chain);
  play_sound(80 + offset);
  orbs_cleared += sz;
  current_score += (orbs_cleared * current_level * chain);
  
  redraw();
  redraw();
  play_sound(78 + offset);
  redraw();
  redraw();
  play_sound(76 + offset);
  
  // Replace full white tiles with transparent
  for (byte i = 0; i < 77; i++)
    if (sprites_playfield[i].type == fullwhite)
      sprites_playfield[i].type = transparent;

  redraw();
  redraw();
  play_sound(74 + offset);
  redraw();
  redraw();
  play_sound(72 + offset);

  // Check for tiles which need to drop now
  byte accumulator[77];
  for (byte i = 0; i < 77; i++)
    accumulator[i] = 0;
  
  collapse_playfield(accumulator);
  
  redraw();
  redraw();
  play_sound(70 + offset);
  redraw();
  redraw();
  play_sound(68 + offset);
  
  byte x, y;
  sprite_type tp;
  for (byte i = 0; i < 77; i++)
  {
    if (accumulator[i] == 1)
    {
      y = i / 7;
      x = i - (y * 7);
      tp = sprites_playfield[i].type;

      // If we have 3 horizontal, mark the chain.
      if ((i >= 14 && sprites_playfield[i-7].type == tp && sprites_playfield[i-14].type == tp)
        || (i >= 7 && sprites_playfield[i-7].type == tp && sprites_playfield[i+7].type == tp)
        || (sprites_playfield[i+7].type == tp && sprites_playfield[i+14].type == tp))
      {
        mark_neighbouring_fields(accumulator, x, y, sprites_playfield[i].type);
      }
      else
      {
        accumulator[i] = 2;
      }
    }
  }

  byte sz_chain = 0;
  for (byte i = 0; i < 77; i++)
    if (accumulator[i] == 1) 
      sz_chain++;
  
  if (sz_chain >= 3)
  {
    byte cleared_orbs[sz_chain];
    byte counter = 0;
    for (int i = 0; i < 77; i++)
      if (accumulator[i] == 1)
        cleared_orbs[counter++] = i; 

    clear_and_score(cleared_orbs, sz_chain, 2 * chain);
  }

  render_arrow();
  play_sound(66 + offset);
  redraw();
  redraw();
}

void inspect_playfield()
{
  byte spaces_above_hero = get_clear_space_above_hero();
  int y = (HERO_Y - spaces_above_hero - 1);
  byte pos = (y * 7) + hero_x;
  sprite_type tp = sprites_playfield[pos].type;
  byte accumulator[77];
  for (byte i = 0; i < 77; i++)
    accumulator[i] = 0;

  accumulator[(y * 7) + hero_x] = 1;
  mark_neighbouring_fields(accumulator, hero_x, y, tp);
  byte sz = 0;
  for (int i = 0; i < 77; i++)
    if (accumulator[i] == 1) 
      sz++;
  
  byte cleared_orbs[sz];
  byte counter = 0;
  for (int i = 0; i < 77; i++)
    if (accumulator[i] == 1)
      cleared_orbs[counter++] = i; 

  // If we have cleared at least 3 orbs (and 3 on a straight line up)...
  if (sz >= 3 && pos >= 14 && sprites_playfield[pos-7].type == tp 
    && sprites_playfield[pos-14].type == tp)
  {
    clear_and_score(cleared_orbs, sz, 1);
  }
}

void throw_orbs()
{
  byte orbs_in_gather_area = 7;
  for (byte i = 0; i < 7; i++)
    if (sprites_gatherarea[SPRITE_OFFSET_GATHER_AREA+i].type == transparent_gatherarea)
      orbs_in_gather_area--;

  byte num_orbs = orbs_in_gather_area;
  byte space_above_hero = get_clear_space_above_hero();
  sprite_type tp = sprites_gatherarea[SPRITE_OFFSET_GATHER_AREA].type;
  byte pos = ((HERO_Y - 1) * 7) + hero_x;

  if (orbs_in_gather_area > space_above_hero) 
  {
    play_sound(68);
    return;
  }

  if (orbs_in_gather_area == 0)
  {
    play_sound(68);
    return;
  }
  
  // Animate moving out of gather_area...
  while (orbs_in_gather_area > 0)
  {
    orbs_in_gather_area--;
    sprites_playfield[pos].type = tp;
    sprites_gatherarea[SPRITE_OFFSET_GATHER_AREA+(orbs_in_gather_area)].type = transparent_gatherarea;
    redraw();
    pos -= 7;
  }

  // ... and upwards!
  byte positions_to_shift = space_above_hero - num_orbs;
  while (positions_to_shift-- > 0)
  {
    sprites_playfield[pos].type = tp;
    sprites_playfield[pos + (num_orbs * 7)].type = arrow_dot;
    redraw();
    pos -= 7;
  }

  // Restore arrow
  render_arrow();
  redraw();

  // Check if we scored points!
  inspect_playfield(); 
}

void play_sound(int freq)
{
  audio_counter = 0;
  sawtooth_wave(freq);
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
      play_sound(70);      
      grab_orbs();
      last_button_reacted_to = NES_A_BUTTON;
    }
    else if (bitRead(controller_state, NES_B_BUTTON))
    {
      play_sound(80);        
      throw_orbs();
      last_button_reacted_to = NES_B_BUTTON;
    }
    else if (controller_state == 0) 
    {
      last_button_reacted_to = 255;
    }
    clear_audio();
  }  
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
  fill_random_rows(4);
  render_arrow();

  orbs_cleared = 0;
  frame_counter = 0;
  audio_counter = 0;
  bool game_over = false;
  
  while (true)
  {
    redraw();

    if (frame_counter > (255 - current_level))
    {
      frame_counter = 0;
      bool game_over = drop_one_row();  
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
  }
  return title_screen;
}


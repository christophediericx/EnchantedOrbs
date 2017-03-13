#include "Arduino.h"
#include <SPI.h>
#include <GD.h>

#include "common.h"
#include "graphics.h"

void fade_to_black(bool (*graphics_callback)(void))
{
  for (byte i = 0; i < 32; i++) 
  {
    for (int j = RAM_PAL; j < (RAM_PAL + 2048); j += 2) 
    {
      uint16_t pal = GD.rd16(j);
      byte r = 31 & (pal >> 10);
      byte g = 31 & (pal >> 5);
      byte b = 31 & pal;
      if (r) r--;
      if (g) g--;
      if (b) b--;
      pal = (r << 10) | (g << 5) | b;
      GD.wr16(j, pal);
    }
    graphics_callback();    
    GD.waitvblank();
  }  
}

void load_background(mode screen_mode)
{
  unsigned char* ram_pic;
  unsigned char* ram_chr;
  unsigned char* ram_pal;
  uint16_t size_ram_chr;
  uint16_t size_ram_pal;

  switch (screen_mode)
  {
    case title_screen:
    case game_over_screen: // reuses title screen PAL, CHR, PIC
    {
      ram_pic = (unsigned char*) titlescreen_pic;
      ram_chr = (unsigned char*) titlescreen_chr;
      ram_pal = (unsigned char*) titlescreen_pal;
      size_ram_chr = sizeof(titlescreen_chr);
      size_ram_pal = sizeof(titlescreen_pal);
      break;
    }
    case game_screen:
    {
      ram_pic = (unsigned char*) gamescreen_pic;
      ram_chr = (unsigned char*) gamescreen_chr;
      ram_pal = (unsigned char*) gamescreen_pal;
      size_ram_chr = sizeof(gamescreen_chr);
      size_ram_pal = sizeof(gamescreen_pal);
      break;      
    }
  }

  // special case for game_over screen (since we reuse title screen elements)
  bool is_game_over = (screen_mode == game_over_screen);
  for (byte y = 0; y < 38; y++)
  {
    byte yy = y;
    if (is_game_over) yy = 0;
    GD.copy(RAM_PIC + y * 64, ram_pic + yy * 50, 50);
  }
  GD.copy(RAM_CHR, ram_chr, size_ram_chr);
  GD.copy(RAM_PAL, ram_pal, size_ram_pal);    
}

void load_sprites()
{
  GD.copy(RAM_SPRPAL, spr_pal, sizeof(spr_pal));
  GD.copy(RAM_SPRIMG, spr_img, sizeof(spr_img));
}

void hide_sprites()
{
  for (int i = 0; i < 512; i++) 
    GD.sprite(i, 500, 500, 0 , 0, 0); 
}

void write_text(String text, byte len, uint16_t spriteIdx, uint16_t xpos, uint16_t ypos)
{
  byte c;
  for (byte i = 0; i < len; i++) 
  {
    c = text[i];
    // Map characters to sprite indexes
    if (c > 64 && c < 94) c -= 56; // A - Z
    else if (c > 47 && c <= 58) c -= 11; // 0 - 9 and :
    else if (c == 32) continue; // SPACE
    GD.sprite(spriteIdx + i, xpos + (i * 16), ypos, c, 0, 0);
  }
}

void render_score(uint32_t score, uint16_t x, uint16_t y, uint16_t sprite_idx)
{
  byte num_digits = count_digits(score);
  if (num_digits == 0) num_digits = 1;
  if (num_digits < 6) write_text("0", 1, sprite_idx, x, y);
  if (num_digits < 5) write_text("0", 1, sprite_idx + 1, x + 16, y);
  if (num_digits < 4) write_text("0", 1, sprite_idx + 2, x + 32, y);
  if (num_digits < 3) write_text("0", 1, sprite_idx + 3, x + 48, y);
  if (num_digits < 2) write_text("0", 1, sprite_idx + 4, x + 64, y);
  if (num_digits < 1) write_text("0", 1, sprite_idx + 5, x + 80, y);
  write_text(String(score), num_digits, sprite_idx + 6 - num_digits, x + 96 - (num_digits * 16), y);
}

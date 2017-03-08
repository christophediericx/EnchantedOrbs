#include "Arduino.h"
#include <SPI.h>
#include <GD.h>

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
  int size_ram_chr;
  int size_ram_pal;

  switch (screen_mode)
  {
    case title_screen:
    {
      ram_pic = titlescreen_pic;
      ram_chr = titlescreen_chr;
      ram_pal = titlescreen_pal;
      size_ram_chr = sizeof(titlescreen_chr);
      size_ram_pal = sizeof(titlescreen_pal);
      break;
    }
    case game_screen:
    {
      ram_pic = gamescreen_pic;
      ram_chr = gamescreen_chr;
      ram_pal = gamescreen_pal;
      size_ram_chr = sizeof(gamescreen_chr);
      size_ram_pal = sizeof(gamescreen_pal);
      break;      
    }
  }
  
  for (byte y = 0; y < 38; y++)
  {
    GD.copy(RAM_PIC + y * 64, ram_pic + y * 50, 50);
  }
  GD.copy(RAM_CHR, ram_chr, size_ram_chr);
  GD.copy(RAM_PAL, ram_pal, size_ram_pal);    
}

void load_sprites(mode screen_mode)
{
  unsigned char* ram_spr_pal;
  unsigned char* ram_spr_img;  
  int size_spr_ram_pal;
  int size_spr_ram_img;

  switch (screen_mode)
  {
    case game_screen:
    {
      ram_spr_pal = gamescreen_spr_pal;
      ram_spr_img = gamescreen_spr_img;
      size_spr_ram_pal = sizeof(gamescreen_spr_pal);
      size_spr_ram_img = sizeof(gamescreen_spr_img);
      break;      
    }
  }
  
  GD.copy(RAM_SPRPAL, ram_spr_pal, size_spr_ram_pal);
  GD.copy(RAM_SPRIMG, ram_spr_img, size_spr_ram_img);
}

void clear_sprites()
{
  for (int i = 0; i < 512; i++)
  {
    GD.sprite(i, 500, 500, 0 , 0, 0);  
  }
}


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
  }
  
  for (byte y = 0; y < 38; y++)
  {
    GD.copy(RAM_PIC + y * 64, ram_pic + y * 50, 50);
  }
  GD.copy(RAM_CHR, ram_chr, size_ram_chr);
  GD.copy(RAM_PAL, ram_pal, size_ram_pal);    
}


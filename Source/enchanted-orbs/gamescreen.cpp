#include "Arduino.h"
#include <SPI.h>
#include <GD.h>

#include "screenmode.h"
#include "gamescreen.h"

void copy_from_tilemap(uint8_t tileIndex, uint16_t x, uint16_t y)
{
  const unsigned char *src = tile_indexes + (4 * (tileIndex - 1));
  GD.copy(RAM_PIC + (x * 2) + (y * 64), src, 2);
  GD.copy(RAM_PIC + (x * 2) + ((y + 1) * 64), src + 2, 2);
}

void initialize_gamescreen(void)
{
  GD.copy(RAM_CHR, image_chr, sizeof(image_chr));
  GD.copy(RAM_PAL, image_pal, sizeof(image_pal));
  uint16_t i;
  uint16_t row = 0;
  for (i = 0; i < 19 * 32; i++)
  {
    if (i % 32 == 0 && i > 0) row++;
    copy_from_tilemap(pgm_read_byte(&(map1[i])), i, row);
  }
}

mode run_gamescreen(void)
{
  delay(100000);
  return title_screen;
}


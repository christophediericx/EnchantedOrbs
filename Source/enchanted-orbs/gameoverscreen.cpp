#include "Arduino.h"

#include "common.h"
#include "graphics.h"
#include "screenmode.h"

/* 
 *  Sprites   0  -    9     characters for "GAME OVER"
 *  Sprites  10  -   24     characters for "YOUR SCORE:"
 *  Sprites  25  -   30     characters for score ("XXXXXX")
 */


void render_game_over_text()
{
  String game_over = "GAME OVER";
  write_text(game_over, 9, 0, 128, 96);
  write_text("YOUR SCORE:", 11, 10, 56, 128);
  render_score(current_score, 248, 128, 25);
}

void initialize_game_over_screen(void) 
{
  load_background(game_over_screen);  
  render_game_over_text();
}

bool game_over_screen_fadecallback(void)
{
  return true;
}

mode run_game_over_screen(void)
{
  delay(5000);
  fade_to_black(game_over_screen_fadecallback);
  delay(5000);
  return title_screen;
}

#include "Arduino.h"
#include <SPI.h>
#include <GD.h>

#include "audio.h"

#define SINE 0
#define NOISE 1

struct voice
{
  float f;
  float a;
} voices[48];

void load(void)
{
  byte v;
  unsigned int gg = 0;

  float sum = 0.0;
  for (v = 0; v < 48; v++) 
  {
    sum += voices[v].a;
  }
  float scale = 255.0 / sum;
  for (v = 0; v < 48; v++) 
  {
    byte a = int(voices[v].a * scale);
    GD.voice(v, 0, int(4 * voices[v].f), a, a);
  } 
}

void note(byte voice, byte m, byte vel)
{
  m = m + 12;
  float f0 = 440 * pow(2.0, (m - 69) / 12.0);
  float a0 = vel / 120.;
  float choirA[] = { 3.5, 1.6, .7, 3.7, 1, 2 };
  if (voice < 11) {
    for (int i = 0; i < 6; i++) {
      choirA[i] /= 2.5;
    }
  }
  byte v;
  for (v = 0; v < 6; v++) {
    voices[v + voice].f = (v + 1) * f0;
    voices[v + voice].a = a0 * choirA[v] / 3.7;
  }
}

void play_melody(const uint8_t* melody, uint16_t len, bool (*audio_callback)(void))
{
  bool quit = false;
  while (!quit) 
  {
    int i = 0;
    uint16_t pc = melody;
    while (pc < (melody + len) && !quit) 
    {
      byte a = pgm_read_byte_near(pc++);
      byte b = pgm_read_byte_near(pc++);
      if (a == 255) 
      {
        // pause
        load();
        long started = millis();
        while (millis() < (started + b)) 
        {
          GD.waitvblank();
          if (!audio_callback()) 
          {
            quit = true;
            break;
          }
        }
      } else 
      {
        byte c = pgm_read_byte_near(pc++);
        note(a, b, c);
      }
    }
    clear_audio();
  }
  clear_audio();
};

void sawtooth_wave(int freq)
{
  uint16_t f = pgm_read_word(midi_freq + freq);
  GD.voice(0, SINE, f,     100,    100);
  GD.voice(1, SINE, 2 * f, 100/2,  100/2);
  GD.voice(2, SINE, 3 * f, 100/3,  100/3);
  GD.voice(3, SINE, 4 * f, 100/4,  100/4);
}

void clear_audio()
{
  for (int i = 0; i < 64; i++) 
    GD.voice(i, 0, 0, 0, 0);  
}


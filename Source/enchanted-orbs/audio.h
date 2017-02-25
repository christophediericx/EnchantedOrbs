#ifndef AUDIO_H
#define AUDIO_H

void play_melody(const uint8_t* melody, uint16_t len, bool (*audio_callback)(void));

#endif

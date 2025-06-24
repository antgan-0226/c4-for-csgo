#ifndef __MP3_H
#define __MP3_H

void MP3CMD(uint8_t CMD , uint16_t data);
void mp3_Init(void);
void mp3_start(void);
void mp3_over(void);
void mp3_boom(void);
void mp3_boom_music(void);
void mp3_playMusic(uint8_t musicIndex);

#endif

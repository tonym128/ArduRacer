#ifndef ARDUINO
#ifndef _CROSS_SDL_H_
#define _CROSS_SDL_H_
#include "structs.h"

bool cross_input_up();
bool cross_input_down();
bool cross_input_left();
bool cross_input_right();
bool cross_input_a();
bool cross_input_b();
void cross_input_quit();

void cross_print(int x, int y, int size, char *string);
void cross_drawPixel(int x, int y, bool colour);
void cross_drawVLine(int x, int y, int length, bool colour);
void cross_drawHLine(int x, int y, int length, bool colour);
void cross_drawBitmapTile(int x, int y, int width, int height, int colour, int mirror, float zoom, unsigned char *tile);
bool cross_getPixel(int x, int y);
void cross_setup();
void cross_clear_screen();
bool cross_loop_start();
void cross_loop_end();
unsigned long getFrameMs();
unsigned long getCurrentMs();

void cross_playSound(bool makeSound, uint16_t hertz, uint8_t duration);

void cross_save(int saveStartAddress, SaveData saveData);
SaveData cross_load(int saveStartAddress);

#endif
#endif

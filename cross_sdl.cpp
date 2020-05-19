#ifdef _WIN32
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include <Windows.h>
#include <cwchar>
#include <stdint.h>
#include <math.h> // Added for GCC
#include <iostream>
#include <fstream>
#include <queue>

#define localtime_r(_Time, _Tm) localtime_s(_Tm, _Time)
#include "myfont.hpp"
#endif

#ifndef ARDUINO
#include "cross_sdl.h"

#ifdef _WIN32
#include "SDL2\SDL.h"
#include "SDL2\SDL_main.h"
#include "SDL2\SDL_audio.h"
#else
#include "SDL.h"
#include "SDL_main.h"
#include "SDL_audio.h"
#endif

const Uint8 *keystate;
SDL_Renderer *renderer;
SDL_Window *window;
const int crossWIDTH = 128;
const int crossHEIGHT = 64;
bool consoleBuffer[crossHEIGHT * crossWIDTH];
unsigned long currentTime, frameTime, fps, frameMs = 0;
const unsigned int FRAME_RATE = 30; // Frame rate in frames per second

bool cross_input_up()
{
	return keystate[SDL_SCANCODE_UP];
}
bool cross_input_down()
{
	return keystate[SDL_SCANCODE_DOWN];
}
bool cross_input_left()
{
	return keystate[SDL_SCANCODE_LEFT];
}
bool cross_input_right()
{
	return keystate[SDL_SCANCODE_RIGHT];
}
bool cross_input_a()
{
	return keystate[SDL_SCANCODE_X];
}
bool cross_input_b()
{
	return keystate[SDL_SCANCODE_Z];
}

void cross_input_quit()
{
	if (keystate[SDL_SCANCODE_Q])
	{
		exit(0);
	}
}

void cross_drawPixel(int x, int y, bool colour)
{
	consoleBuffer[x + y * crossWIDTH] = colour;
}

void cross_drawHLine(int x, int y, int length, bool colour)
{
	for (int i = x; i < x + length; i++)
	{
		if (i >= 0 && i < 128 && y >= 0 && y < 64)
		{
			consoleBuffer[i + y * 128] = colour;
		}
	}
}

void cross_drawVLine(int x, int y, int length, bool colour)
{
	for (int i = y; i < y + length; i++)
	{
		if (x >= 0 && x < 128 && i >= 0 && i < 64)
		{
			consoleBuffer[x + i * 128] = colour;
		}
	}
}

void cross_drawObject(int x, int y, int w, int h, bool *object)
{
	int counter = 0;
	for (int y2 = y; y2 < y + h; y2++)
		for (int x2 = x; x2 < x + w; x2++)
			if (x2 >= 0 && x2 < 128 && y2 >= 0 && y2 < 64)
				cross_drawPixel(x2, y2, object[counter++]);
}

void cross_drawObjectDouble(int x, int y, int w, int h, bool *object)
{
	int counter = 0;
	for (int y2 = y; y2 < y + h * 2; y2 += 2)
		for (int x2 = x; x2 < x + w * 2; x2 += 2)
			if (x2 >= 0 && x2 < 128 && y2 >= 0 && y2 < 64)
			{
				cross_drawPixel(x2, y2, object[counter]);
				cross_drawPixel(x2, y2 + 1, object[counter]);
				cross_drawPixel(x2 + 1, y2, object[counter]);
				cross_drawPixel(x2 + 1, y2 + 1, object[counter++]);
			}
}

void cross_drawObjectTriple(int x, int y, int w, int h, bool *object)
{
	int counter = 0;
	for (int y2 = y; y2 < y + h * 3; y2 += 3)
		for (int x2 = x; x2 < x + w * 3; x2 += 3)
			if (x2 >= 0 && x2 < 128 && y2 >= 0 && y2 < 64)
			{
				cross_drawPixel(x2, y2, object[counter]);
				cross_drawPixel(x2, y2 + 1, object[counter]);
				cross_drawPixel(x2, y2 + 2, object[counter]);
				cross_drawPixel(x2 + 1, y2, object[counter]);
				cross_drawPixel(x2 + 1, y2 + 1, object[counter]);
				cross_drawPixel(x2 + 1, y2 + 2, object[counter]);
				cross_drawPixel(x2 + 2, y2, object[counter]);
				cross_drawPixel(x2 + 2, y2 + 1, object[counter]);
				cross_drawPixel(x2 + 2, y2 + 2, object[counter++]);
			}
}

void drawCharacter(int x, int y, char charPos)
{
	bool *character = font(charPos);
	cross_drawObject(x, y, 8, 8, character);
	free(character);
}

void drawCharacterDouble(int x, int y, char charPos)
{
	bool *character = font(charPos);
	cross_drawObjectDouble(x, y, 8, 8, character);
	free(character);
}

void drawCharacterTriple(int x, int y, char charPos)
{
	bool *character = font(charPos);
	cross_drawObjectTriple(x, y, 8, 8, character);
	free(character);
}

void cross_print(int x, int y, int size, char *string)
{
	for (int i = 0; i < static_cast<int>(strlen(string)); i++)
	{
		switch (size)
		{
		case 1:
			drawCharacter(x + 6 * i, y, string[i]);
			break;
		case 2:
			drawCharacterDouble(x + 12 * i, y, string[i]);
			break;
		case 3:
			drawCharacterTriple(x + 18 * i, y, string[i]);
			break;
		default:
			drawCharacterTriple(x + 6 * i, y, string[i]);
		}
	}
}

static void resizeObject(int dimwidth, int dimheight, double zoom, const bool *object, bool *resized)
{
	int counter = 0;
	for (int y = 0; y < dimheight; y++)
	{
		for (int x = 0; x < dimwidth; x++)
		{
			if ((int)(round(x*zoom) + round(y * zoom ) * dimwidth) > 0 && (int)(round(x*zoom) + round(y * zoom ) * dimwidth) < dimheight * dimwidth
			&& (x + y * dimwidth > 0) && x + y * dimwidth < dimwidth * dimheight)
			resized[(int)(round(x*zoom) + round(y * zoom ) * dimwidth)] = object[x + y * dimwidth];
		}
	}
}


void cross_drawBitmapTile(int x, int y, int width, int height, int colour, int mirror, float zoom, unsigned char *tile)
{
	bool *rotated = (bool *)malloc(width * height * sizeof(bool));
	resizeObject(width, height, zoom, (const bool *)tile, rotated);
	int counter = 0;
	if (mirror == 0)
		for (int y2 = y; y2 < y + height; y2++)
		{
			for (int x2 = x; x2 < x + width; x2++)
			{
				if (x2 >= 0 && x2 < 128 && y2 >= 0 && y2 < 64)
				{
					if (((bool *)rotated)[counter] == (bool)colour)
						consoleBuffer[x2 + y2 * 128] = ((bool *)rotated)[counter];
				}

				counter++;
			}
		}
	else if (mirror == 1)
		for (int y2 = y; y2 < y + height; y2++)
		{
			for (int x2 = x + width-1; x2 >= x; x2--)
			{
				if (x2 >= 0 && x2 < 128 && y2 >= 0 && y2 < 64)
				{
					if (((bool *)rotated)[counter] == (bool)colour)
						consoleBuffer[x2 + y2 * 128] = ((bool *)rotated)[counter];
				}

				counter++;
			}
		}

	free(rotated);
}

bool cross_getPixel(int x, int y)
{
	return consoleBuffer[x + y * 128];
}

void cross_setup()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	window = SDL_CreateWindow(
		"Faurxduboy",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		512,
		256,
		0);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
}

void sendToScreen()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	for (int i = 0; i < crossWIDTH * crossHEIGHT; i++)
	{
		int x = i % crossWIDTH;
		int y = i / crossWIDTH;
		if (consoleBuffer[i])
		{
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
			SDL_RenderDrawPoint(renderer, x * 4, y * 4);
			SDL_RenderDrawPoint(renderer, x * 4 + 1, y * 4 + 1);
			SDL_RenderDrawPoint(renderer, x * 4, y * 4 + 1);
			SDL_RenderDrawPoint(renderer, x * 4 + 1, y * 4);
		}
	}
	SDL_RenderPresent(renderer);
}

long getTimeInMillis()
{
	return timeGetTime();
}

void cross_clear_screen()
{
	memset(consoleBuffer, 0, 128 * 64);
}

void cross_playSound(bool makeSound, uint16_t hertz, uint8_t duration) {
};

bool cross_loop_start()
{
	if (frameTime == 0)
		frameTime = getTimeInMillis();

	if ((getTimeInMillis() - frameTime) < 33)
		return false;

	SDL_PumpEvents();
	keystate = SDL_GetKeyboardState(NULL);
	frameTime = currentTime;
	currentTime = getTimeInMillis();
	frameMs = currentTime - frameTime;
	if (frameMs == 0)
		frameMs = 1;

	return true;
}

void cross_loop_end()
{
	sendToScreen();
}

unsigned long getFrameMs()
{
	return frameMs;
}

unsigned long getCurrentMs()
{
	return currentTime;
}

const char *filename = "racer.eeprom";

void cross_save(int saveStartAddress, SaveData data)
{
	SDL_RWops *rw = SDL_RWFromFile(filename, "w");
	if (rw != NULL)
	{
		size_t len = sizeof(data);
		if (SDL_RWwrite(rw, (char *)&data, 1, len) != len)
		{
			return;
		}

		SDL_RWclose(rw);
	}
}

SaveData cross_load(int saveStartAddress)
{
	SaveData *loaddata = new SaveData();

	SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
	if (rw == NULL)
		return *loaddata;

	size_t res_size = (size_t)SDL_RWsize(rw);
	char *fileData = (char *)malloc(res_size + 2);

	size_t nb_read_total = 0, nb_read = 1;
	char *buf = fileData;
	while (nb_read_total < res_size && nb_read != 0)
	{
		nb_read = SDL_RWread(rw, buf, 1, (res_size - nb_read_total));
		nb_read_total += nb_read;
		buf += nb_read;
	}
	SDL_RWclose(rw);
	if (nb_read_total != res_size)
	{
		free(fileData);
	}

	loaddata = (SaveData *)fileData;
	if (loaddata->GameNo == GAMENO && loaddata->SaveVer == SAVEVER)
	{
		return *loaddata;
	}
	else
	{
		free(loaddata);
		loaddata = new SaveData();
	}

	return *loaddata;
}

#endif

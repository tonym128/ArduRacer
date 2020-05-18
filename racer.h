#ifndef _RACER_H_
#define _RACER_H_

// This flag uses the flipped tiles instead of storing images, was too much of a performance killer in testing
// See image.h for implementation
// #define SPACE_SAVER 

#ifdef ARDUINO
#include "cross_arduino.h"
// #include "ATMlib.h"
#include "song.h"
#else
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef unsigned char byte;
#include "cross_sdl.h"
#endif

#include "structs.h"
#include "fixpoint.h"
#include "image.h"
#include "levels.h"
#include "car.h"

void racerSetup();
void racerLoop();

#endif
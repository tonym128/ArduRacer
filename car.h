#ifndef _CAR_H_
#define _CAR_H_
#include "fixpoint.h"

#ifndef ARDUINO
#define PROGMEM 
#endif

const unsigned char PROGMEM car1_00[] ={
0b11111111,
0b11111000,
0b01111000,
0b11000110,
0b10110101,
0b10001100,
0b01110000,
0b00111000,
0b00000110,
0b10000101,
0b10000000,
0b01111111,
0b11110000,
};

const unsigned char PROGMEM car1_01[] ={
0b11111111,
0b11111000,
0b00011110,
0b10110110,
0b00001101,
0b10100000,
0b01100000,
0b01011100,
0b00000111,
0b00010111,
0b11110001,
0b11111111,
0b11110000,
};

const unsigned char PROGMEM car1_02[] ={
0b11111111,
0b11100010,
0b00111010,
0b00101110,
0b00000001,
0b10000111,
0b01100001,
0b11011000,
0b00000110,
0b10001011,
0b10001000,
0b11111111,
0b11110000,
};

const unsigned char PROGMEM car1_03[] ={
0b11111111,
0b11111100,
0b01111100,
0b01011111,
0b00000001,
0b10000001,
0b01101000,
0b00011000,
0b00110111,
0b10101101,
0b11100000,
0b01111111,
0b11110000,
};

const unsigned char PROGMEM car1_04[] ={
0b11111111,
0b11100000,
0b00011010,
0b00010110,
0b00000001,
0b11000000,
0b11100011,
0b00011010,
0b11010110,
0b00110001,
0b11100001,
0b11111111,
0b11110000,
};

const unsigned char PROGMEM car1_05[] ={
0b11111111,
0b11111000,
0b11111110,
0b10001110,
0b00000011,
0b10100000,
0b01100000,
0b01011011,
0b00000110,
0b11010111,
0b10000001,
0b11111111,
0b11110000,
};

const unsigned char PROGMEM car1_06[] ={
0b11111111,
0b11110001,
0b00011101,
0b00010110,
0b00000001,
0b10111000,
0b01101110,
0b00011000,
0b00000111,
0b01000101,
0b11000100,
0b01111111,
0b11110000,
};

const unsigned char PROGMEM car1_07[] ={
0b11111111,
0b11100000,
0b01111011,
0b01011110,
0b11000001,
0b10000001,
0b01101000,
0b00011000,
0b00001111,
0b10100011,
0b11100011,
0b11111111,
0b11110000,
};

const unsigned char PROGMEM car1_00mask[] ={
0b11000000,
0b11000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00100000,
0b00010000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000
};

const unsigned char  PROGMEM car1_01mask[] ={
0b11000000,
0b00110000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000010,
0b00000000,
0b10000000,
0b11111000,
0b00110000,
};

const unsigned char  PROGMEM car1_02mask[] ={
0b00001000,
0b01000000,
0b00010000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b01000010,
0b00010000,
};

const unsigned char  PROGMEM car1_03mask[] ={
0b11100000,
0b11100000,
0b00111000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b11000000,
0b00110000,
0b00000000,
};

const unsigned char  PROGMEM car1_04mask[] ={
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b10000000,
0b01000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00110000,
0b00110000,
};

const unsigned char  PROGMEM car1_05mask[] ={
0b11000001,
0b11110000,
0b00010000,
0b00000100,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b11000000,
0b00110000,
};

const unsigned char  PROGMEM car1_06mask[] ={
0b10000100,
0b00100000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b10000000,
0b00100001,
0b00000000,
};

const unsigned char  PROGMEM car1_07mask[] ={
0b00000000,
0b11000000,
0b00110000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000001,
0b11000000,
0b01110000,
0b01110000,
};

static uint8_t getCarDirection(FIXPOINT radians) {
  if (radians < 10. / 8. * PI)
    return 0;
  else if (radians < 30. / 8. * PI)
    return 1;
  else if (radians < 50. / 8. * PI)
    return 2;
  else if (radians < 70. / 8. * PI)
    return 3;
  else if (radians < 90. / 8. * PI)
    return 4;
  else if (radians < 110. / 8. * PI)
    return 5;
  else if (radians < 130. / 8. * PI)
    return 6;
  else if (radians < 150. / 8. * PI)
    return 7;

  return 0;
}

static unsigned char *getCar(uint8_t direction) {
  switch (direction) {
    case 0: return (unsigned char *)car1_00;
    case 1: return (unsigned char *)car1_01;
    case 2: return (unsigned char *)car1_02;
    case 3: return (unsigned char *)car1_03;
    case 4: return (unsigned char *)car1_04;
    case 5: return (unsigned char *)car1_05;
    case 6: return (unsigned char *)car1_06;
    case 7: return (unsigned char *)car1_07;
  }

  return (unsigned char *)car1_00;
}

static unsigned char *getCarMask(uint8_t direction) {
  switch (direction) {
    case 0: return (unsigned char *)car1_00mask;
    case 1: return (unsigned char *)car1_01mask;
    case 2: return (unsigned char *)car1_02mask;
    case 3: return (unsigned char *)car1_03mask;
    case 4: return (unsigned char *)car1_04mask;
    case 5: return (unsigned char *)car1_05mask;
    case 6: return (unsigned char *)car1_06mask;
    case 7: return (unsigned char *)car1_07mask;
  }
  
  return (unsigned char *)car1_00mask;
}

#endif

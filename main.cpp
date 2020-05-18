#ifndef ARDUINO
#include "racer.h"

int main(int argc, char*argv[])
{
  racerSetup();
  while (1)
    racerLoop();
}
#endif
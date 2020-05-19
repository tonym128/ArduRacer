#ifndef _RACER_H_
#include "racer.h"
#include "images\ArduRacerLogo.h"
#include "images\zoomNumbers.h"
#endif

char string[30];
SaveData saveData;
float ZOOM_TIME = 6000.0f;
GameState *gameState;
unsigned long tFrameMs = 0;

void freeUp(CheckPoint *p)
{
  if (p != nullptr && p->nextcheckpoint != nullptr)
    freeUp(p->nextcheckpoint);
  if (p != nullptr)
    free(p);
}

void setLevelDetails()
{
  gameState->lasttile = 100;
  gameState->lastx = 100;
  gameState->lasty = 100;
  gameState->paused = 0;

  bool done = false;
  gameState->checkpoints = 0;
  gameState->checkpointpassed = 0;

  freeUp(gameState->levelCheckPoints);
  CheckPoint *checkpoint = nullptr;
  CheckPoint *firstcheckpoint = nullptr;

  for (int x = 0; x < 10 && !done; x++)
  {
    for (int y = 0; y < 10 && !done; y++)
    {
      uint8_t levelTile = getLevelTile(gameState->level, x, y);
      if (levelTile == 24)
      {
        gameState->player1.X = FLOAT_TO_FIXP(x * 64.0f);
        gameState->player1.Y = FLOAT_TO_FIXP(y * 64.0f + 32 - 4);
        gameState->player1.rotation = PI / 2;
        //        done = true;
      }
      else if (levelTile == 25)
      {
        gameState->player1.X = FLOAT_TO_FIXP(x * 64.0f + 32 - 4);
        gameState->player1.Y = FLOAT_TO_FIXP(y * 64.0f);
        gameState->player1.rotation = PI;
        //        done = true;
      }
      else if (levelTile == 26 || levelTile == 27)
      {
        if (checkpoint == nullptr)
        {
          checkpoint = new CheckPoint();
          gameState->levelCheckPoints = checkpoint;
        }
        else
        {
          checkpoint->nextcheckpoint = new CheckPoint();
          checkpoint = checkpoint->nextcheckpoint;
        }

        checkpoint->x = x;
        checkpoint->y = y;
        checkpoint->passed = false;
        gameState->checkpoints++;
      }
    }
  }

  gameState->laptimer = false;
  gameState->curlap = 0;
  gameState->newbestLap = false;
  for (int i = 0; i < TIMED_LAPS; i++)
  {
    gameState->laptimes[i] = 0;
  }

  gameState->bestLap = saveData.BestLapTimes[gameState->level-1];

  gameState->player1.offRoad = false;
}

void processGameMode()
{
#ifndef ARDUINO
  cross_input_quit();
#endif

  if (gameState->paused)
  {
    if (cross_input_down())
    {
      gameState->mode = 4;
    }
  }

  if (cross_input_a())
  {
    if (gameState->paused)
      gameState->paused = false;
  }

  if (cross_input_up())
  {
    gameState->paused = true;
    return;
  }

  if (cross_input_left())
  {
    gameState->player1.rotation -= 0.003 * tFrameMs;
  }

  if (cross_input_right())
  {
    gameState->player1.rotation += 0.003 * tFrameMs;
  }

  if (gameState->player1.rotation < 0)
    gameState->player1.rotation = PI * 2;
  if (gameState->player1.rotation > PI * 2)
    gameState->player1.rotation = 0;

  if (gameState->laptimer && !gameState->paused)
  {
    gameState->laptimes[(gameState->curlap)] += tFrameMs;
    if (gameState->laptimes[(gameState->curlap)] > 60000)
    {
      gameState->laptimes[(gameState->curlap)] = 60000;
    }
  }

  if (cross_input_a())
  {
    gameState->player1.acceleration.force += FLOAT_TO_FIXP(0.01 * tFrameMs);
  }

  if (cross_input_b())
  {
    gameState->player1.acceleration.force -= FLOAT_TO_FIXP(0.004 * tFrameMs);
  }
}

void updateGameMode()
{
  if (gameState->paused)
    return;

  gameState->player1.X += xVec2(gameState->player1.acceleration.force, FLOAT_TO_FIXP(gameState->player1.rotation));
  gameState->player1.Y += yVec2(gameState->player1.acceleration.force, FLOAT_TO_FIXP(gameState->player1.rotation));

  int x = FIXP_TO_INT(gameState->player1.X);
  int y = FIXP_TO_INT(gameState->player1.Y);

  bool collision = false;
  if (x > 640)
  {
    gameState->player1.X = FLOAT_TO_FIXP(640.0f);
    collision = true;
  }
  else if (x < 0)
  {
    gameState->player1.X = INT_TO_FIXP(0);
    collision = true;
  }

  if (y > 640)
  {
    gameState->player1.Y = FLOAT_TO_FIXP(640.0f);
    collision = true;
  }
  else if (y < 0)
  {
    gameState->player1.Y = INT_TO_FIXP(0);
    collision = true;
  }

  // Side Hard wall collisions
  if (collision)
  {
    if (gameState->player1.acceleration.force > 0)
      gameState->player1.acceleration.force -= FLOAT_TO_FIXP(0.01 * tFrameMs);
    if (gameState->player1.acceleration.force < 0)
      gameState->player1.acceleration.force += FLOAT_TO_FIXP(0.01 * tFrameMs);
  }

  // Max Speed
  if (gameState->player1.acceleration.force > FLOAT_TO_FIXP(5.0f))
    gameState->player1.acceleration.force = FLOAT_TO_FIXP(5.0f);
  else if (gameState->player1.acceleration.force < FLOAT_TO_FIXP(-2.0f))
    gameState->player1.acceleration.force = FLOAT_TO_FIXP(-2.0f);

  // Drag
  if (gameState->player1.acceleration.force > 0)
  {
    gameState->player1.acceleration.force -= FLOAT_TO_FIXP(0.06f/30.0f * tFrameMs);
    if (gameState->player1.acceleration.force < 0)
      gameState->player1.acceleration.force = 0;
  }
  else if (gameState->player1.acceleration.force < 0)
  {
    gameState->player1.acceleration.force += FLOAT_TO_FIXP(0.06f/30.0f * tFrameMs);
    if (gameState->player1.acceleration.force > 0)
      gameState->player1.acceleration.force = 0;
  }

  // Off Road
  if (gameState->player1.offRoad)
  {
    if (gameState->player1.acceleration.force > FLOAT_TO_FIXP(1.5f))
    {
      gameState->player1.acceleration.force -= FLOAT_TO_FIXP(0.015f * tFrameMs);
    }
    else if (gameState->player1.acceleration.force < FLOAT_TO_FIXP(-1.5f))
    {
      gameState->player1.acceleration.force += FLOAT_TO_FIXP(0.015f * tFrameMs);
    }
  }

  // Tile Logic
  int tilex = (x + 5) / 64;
  int tiley = (y + 5) / 64;
  int tile = getLevelTile(gameState->level, tilex, tiley);

  if (gameState->lastx != tilex || gameState->lasty != tiley)
  {
    // Went through a check point
    if (gameState->lasttile == 26 || gameState->lasttile == 27)
    {
      bool done = false;
      CheckPoint *a = gameState->levelCheckPoints;
      while (a != nullptr && !done)
      {
        if (a->x == gameState->lastx && a->y == gameState->lasty)
        {
          if (!a->passed)
          {
            a->passed = true;
            gameState->checkpointpassed++;
          }
          done = true;
        }
        a = a->nextcheckpoint;
      }
    }

    // Was in start block and no longer in start block
    if (gameState->lasttile == 24 || gameState->lasttile == 25)
    {
      if (gameState->laptimer)
      {
        if (gameState->checkpoints == gameState->checkpointpassed)
        {
          // Finished a lap
          if (gameState->laptimes[(gameState->curlap)] < gameState->bestLap || gameState->bestLap == 0)
          {
            gameState->bestLap = gameState->laptimes[(gameState->curlap)];
            gameState->newbestLap = true;

            // Update Best Lap Time to Rom
            if (saveData.BestLapTimes[gameState->level - 1] == 0 || saveData.BestLapTimes[gameState->level - 1] > gameState->laptimes[(gameState->curlap)])
            {
              saveData.BestLapTimes[gameState->level - 1] = gameState->laptimes[(gameState->curlap)];

            #ifdef ARDUINO
              unsigned int levelTime = pgm_read_dword(LevelTimes[gameState->level - 1] + 2);
            #else
              unsigned int levelTime = LevelTimes[gameState->level - 1][2];
            #endif
              if (saveData.BestLapTimes[gameState->level - 1] < levelTime) { // If the time is better than 3rd place, we can go forwards
                if (saveData.maxLevel < gameState->level+1 && gameState->level < 10) 
                  saveData.maxLevel = gameState->level+1;
              }

              if (gameState->level > saveData.maxLevel)
                saveData.maxLevel = gameState->level;

              cross_save(saveStartAddress, saveData);
            }
          }

          gameState->curlap++;
          gameState->checkpointpassed = 0;

          CheckPoint *point = gameState->levelCheckPoints;
          while (point != nullptr)
          {
            point->passed = false;
            point = point->nextcheckpoint;
          }
        }
      }
      else
      {
        gameState->laptimer = true;
      }
    }
  }

  gameState->lasttile = tile;
  gameState->lastx = tilex;
  gameState->lasty = tiley;
}

bool isKthBitSet(int n, int k)
{
  return (n & (1 << (k - 1)));
}

void setLevelTimeString(char number, unsigned int time) {
#ifdef ARDUINO
  sprintf(string, "%c-%2u.%02u", number, time / 1000, time / 10 % 100);
#else
  sprintf_s(string, "%c-%2u.%02u", number, time / 1000, time / 10 % 100);
#endif
}

void setLevelString() {
#ifdef ARDUINO
  sprintf(string, "Level %d", gameState->level);
#else
  sprintf_s(string, "Level %d", gameState->level);
#endif
}

void displayGameMode()
{
  int x = FIXP_TO_INT(gameState->player1.X);
  int y = FIXP_TO_INT(gameState->player1.Y);
  x -= 60;
  y -= 30;

  if (x < 0)
    x = 0;
  else if (x > 640 - 128)
    x = 640 - 128;

  if (y < 0)
    y = 0;
  else if (y > 640 - 64)
    y = 640 - 64;

  int inlinex = x % 64 * -1;
  int inliney = y % 64 * -1;
  // Draw Road
  for (uint8_t j = y / 64; j < (y / 64) + 4 && j < 10; j++)
  {
    for (uint8_t i = x / 64; i < (x / 64) + 6 && i < 10; i++)
    {
      int levelTile = getLevelTile(gameState->level, i, j);
      #ifdef SPACE_SAVER
      cross_drawBitmapTile(inlinex, inliney, 64, 64, 1, getTileRotation(levelTile), 1.0f, (unsigned char *)getTile(levelTile));
      #else
      cross_drawBitmapTile(inlinex, inliney, (unsigned char *)getTile(levelTile));
      #endif

      if (levelTile > 23)
      {
        switch (levelTile)
        {
        case 24:
          cross_drawVLine(inlinex + 63 - 8, inliney, 64, true);
          break;
        case 25:
          cross_drawHLine(inlinex, inliney + 63 - 8, 64, true);
          break;
        case 26:
          cross_drawHLine(inlinex, inliney + 12, 64, true);
          cross_drawHLine(inlinex, inliney + 13, 64, true);
          cross_drawHLine(inlinex, inliney + 64 - 13, 64, true);
          cross_drawHLine(inlinex, inliney + 64 - 14, 64, true);
          break;
        case 27:
          cross_drawVLine(inlinex + 12, inliney, 64, true);
          cross_drawVLine(inlinex + 13, inliney, 64, true);
          cross_drawVLine(inlinex + 64 - 13, inliney, 64, true);
          cross_drawVLine(inlinex + 64 - 14, inliney, 64, true);
          break;
        }
      }
      inlinex += 64;
    }

    inlinex = x % 64 * -1;
    inliney += 64;
  }

  // Off Road Check (Do it here because we rely on the populated screen
  int carx = FIXP_TO_INT(gameState->player1.X);
  int cary = FIXP_TO_INT(gameState->player1.Y);

  if (carx < 60)
  {
    // use x as is in direct to screen;
  }
  else if (carx > 650 - 68)
  {
    carx = 128 - (650 - carx);
  }
  else
  {
    carx = 60;
  }

  if (cary < 30)
  {
    // use y as is in direct to screen;
  }
  else if (cary > 650 - 36)
  {
    cary = 64 - (650 - cary);
  }
  else
  {
    cary = 30;
  }

  gameState->player1.offRoad = false;
  if (cross_getPixel(carx+5, cary+5) == 1)
  {
    gameState->player1.offRoad = true;
  }

  // Draw Car
  uint8_t carDirection = getCarDirection(((FIXPOINT)(gameState->player1.rotation * 10)));
  unsigned char *car = getCar(carDirection);
  unsigned char *carMask = getCarMask(carDirection);
  byte image;
  byte mask;

  int counter = 0;

  int drawx = 0;
  int drawy = 0;
  int counter2 = 0;
#ifdef ARDUINO
  image = pgm_read_byte_near(car);
  mask = pgm_read_byte_near(carMask);
#else
  image = car[0];
  mask = carMask[0];
#endif

  while (drawy < 10)
  {
    if (!isKthBitSet(mask, 8 - (counter)))
      cross_drawPixel(carx + drawx, cary + drawy, !isKthBitSet(image, 8 - (counter)));

    counter++;
    if (counter == 8)
    {
      counter2++;
#ifdef ARDUINO
      image = pgm_read_byte_near(car + counter2);
      mask = pgm_read_byte_near(carMask + counter2);
#else
      image = car[counter2];
      mask = carMask[counter2];
#endif
      counter = 0;
    }

    drawx++;
    if (drawx == 10)
    {
      drawy++;
      drawx = 0;
    }
  }

  // Display laptimer
#ifdef ARDUINO
  sprintf(string,"L%d/5-%d/%d %2u.%02u", gameState->curlap + 1, gameState->checkpointpassed, gameState->checkpoints, gameState->laptimes[(gameState->curlap)] / 1000, gameState->laptimes[(gameState->curlap)] / 10 % 100);
#else
  sprintf_s(string, "L%d/5-%d/%d %2u.%02u", gameState->curlap + 1, gameState->checkpointpassed, gameState->checkpoints, gameState->laptimes[(gameState->curlap)] / 1000, gameState->laptimes[(gameState->curlap)] / 10 % 100);
#endif

  cross_print(0, 0, 1, string);

  setLevelTimeString(gameState->newbestLap ? '*':'B', saveData.BestLapTimes[gameState->level - 1]);
  cross_print(0, 8, 1, string);

  for (int i = 0; i < 16;i++) {
    cross_drawVLine(128-10-6+i,0,6,0);
  }
  if (gameState->player1.offRoad && gameState->player1.acceleration.force != 0) {
    if (((gameState->laptimes[(gameState->curlap)] / 100) % 3) == 0) 
        cross_playSound(saveData.sound, 100,30);
  }

  for (int i=0;i <= FIXP_TO_FLOAT(gameState->player1.acceleration.force * 3); i++) {
    cross_drawVLine(113+i, 0,7,1);
  } 
  cross_drawHLine(113, 6,15,0);

  if (gameState->paused)
  {
    cross_print(9, 20, 3, " Paused ");
    for (int i = 0; i < 16; i++) cross_drawHLine(0,i+48,64,0);

    cross_print(0, 48, 1, "A / B - Continue");
    cross_print(0, 56, 1, "Down - Level Select");
  }
}

void racerSetup()
{
  saveData = cross_load(saveStartAddress);
  gameState = new GameState();
  gameState->level = 1;
  gameState->laptimer = false;
  gameState->timeout = 0;
  gameState->lastmode = -1;
  gameState->mode = 0;
  
  #ifdef ARDUINO
  // ATM.play(music);
  #endif

  cross_setup();
  cross_loop_end();
}

void processMenu()
{
#ifndef ARDUINO
  cross_input_quit();
#endif

  // Debounce input
  if (gameState->timeout > 0)
  {
    gameState->timeout -= getFrameMs();
  }

  // Check for up button
  else if (cross_input_up() && gameState->timeout <= 0)
  {
    if (gameState->menuItem == 0)
      gameState->menuItem = 3;
    else
      gameState->menuItem -= 1;
    gameState->timeout = 200;
  }

  // Check for down button
  else if (cross_input_down() && gameState->timeout <= 0)
  {
    if (gameState->menuItem == 3)
      gameState->menuItem = 0;
    else
      gameState->menuItem += 1;
    gameState->timeout = 200;
  }
  else if (cross_input_a() && gameState->timeout <= 0)
  {
    gameState->enter = true;
    gameState->timeout = 200;
  }

  else if (cross_input_left() && gameState->timeout <= 0) {
    gameState->mode = 98;
  }

}

void updateMenu()
{
  if (gameState->enter)
  {
    switch (gameState->menuItem)
    {
    case 0:
      // Go To Last Level Finished
      gameState->level = saveData.maxLevel;
      gameState->mode = 4; // Level Start
      break;
    case 1:
      // Go To First Level
      gameState->level = 1;
      gameState->mode = 4; // Level Start
      break;
    case 2:
      // Toggle Sound
      saveData.sound = !saveData.sound;
      gameState->enter = false;
      cross_save(saveStartAddress, saveData);
      break;
    case 3:
      // Clear Data
      gameState->mode = 99;
      break;
    }
  }
}

void displayMenu(int menuItem)
{
  cross_print(90, 30 + 0, 1, "Cont");
  cross_print(90, 30 + 8, 1, "Start");
  cross_print(90, 30 + 16, 1, "Sound");
  cross_print(90, 30 + 24, 1, "Data");

  if (saveData.sound)
    cross_print(90 + 5 * 6, 30 + 16, 1, "*");

  cross_print(84, 30 + menuItem * 8, 1, "*");
}

void displayMap()
{
  for (int y = 0; y < 10; y++)
  {
    for (int x = 0; x < 10; x++)
    {
      #ifdef SPACE_SAVER
      int LevelTile = getLevelTile(gameState->level, x, y);
      cross_drawBitmapTile(x * 6, y * 6, 64, 64, 1, getTileRotation(LevelTile), 0.1f, (unsigned char *)getTile(LevelTile));
      #else
      cross_drawBitmapTile(x * 6, y * 6, 64, 64, 1, 0, 0.1f, (unsigned char *)getTile(getLevelTile(gameState->level, x, y)));
      #endif
    }
  }
}

void drawBestTimes() {
    for (int i = 0; i < 3; i++) {
    #ifdef ARDUINO
      setLevelTimeString(i + 1 + 48, (unsigned int)pgm_read_dword(LevelTimes[gameState->level - 1]+ i));
    #else
      setLevelTimeString(i + 1 + 48, LevelTimes[gameState->level - 1][i]);
    #endif

      cross_print(74, 21+8*i, 1, string);
  }
}


void displayLevelInfo()
{
  gameState->timeout -= getFrameMs(); // 1 second for start screen.
  for (int x = 64; x < 128; x++)
  {
    cross_drawVLine(x, 0, 64, 0);
  }

  setLevelString();
  cross_print(74, 0, 1, string);
  cross_drawHLine(64, 8, 64, 1);
  setLevelTimeString('B', saveData.BestLapTimes[gameState->level - 1]);

  cross_print(74, 10, 1, string);
  cross_drawHLine(64, 19, 64, 1);
  drawBestTimes();
  cross_drawHLine(64, 46, 64, 1);

  if (gameState->timeout <= 0)
  {
    if ((getCurrentMs() / 1000) % 2 == 0)
    {
      cross_print(64, 64 - 16, 1, "Press A");
      cross_print(64, 64 - 8, 1, "To Start");
    }
  }
};

void inputLevelInfo()
{
  if (cross_input_a())
  {
    gameState->mode++;
    gameState->timeout = 100;
  }

  if (cross_input_b())
  {
    gameState->mode = 1;
    gameState->enter = false;
    gameState->timeout = 100;
  }

  if (cross_input_up())
  {
    gameState->timeout = 100;
    if (gameState->level < saveData.maxLevel) gameState->level++;
    gameState->lastmode = -1;
    if (gameState->level > 10)
      gameState->level = 10;
  }

  if (cross_input_down())
  {
    gameState->timeout = 100;
    gameState->level--;
    gameState->lastmode = -1;
    if (gameState->level < 1)
      gameState->level = 1;
  }
}

bool displayLevelZoom()
{
  float time = ((ZOOM_TIME - gameState->timeout) / ZOOM_TIME);
  float zoom = 0.1f + 0.9f * time;
  int pixelSize = (int)floor(6.0f + 58.0f * time);

  float headtox = ((FIXP_TO_FLOAT(gameState->player1.X) - 64) * time * 2);
  float headtoy = ((FIXP_TO_FLOAT(gameState->player1.Y) - 32) * time * 2);

  if (headtox < 0)
    headtox = 0;
  if (headtoy < 0)
    headtoy = 0;

  if (headtox > FIXP_TO_FLOAT(gameState->player1.X) - 64)
    headtox = FIXP_TO_FLOAT(gameState->player1.X) - 64;

  if (headtoy > FIXP_TO_FLOAT(gameState->player1.Y) - 32)
    headtoy = FIXP_TO_FLOAT(gameState->player1.Y) - 32;

  headtox *= (pixelSize / 64.0f);
  headtoy *= (pixelSize / 64.0f);

  int endx = (int)(64 + 64 * (time * 7));
  if (endx > 128)
    endx = 128;

  for (int y = 0; y < 10; y++)
  {
    for (int x = 0; x < 10; x++)
    {
      // Basic Screen Check
      if ((x * pixelSize + pixelSize - headtox) > 0 && (y * pixelSize + pixelSize - headtoy) > 0 && (x * pixelSize - headtox) < endx && (y * pixelSize - headtoy < 64)) {
      #ifdef SPACE_SAVER
      int LevelTile = getLevelTile(gameState->level, x, y);
      cross_drawBitmapTile((int)(x * pixelSize - headtox), (int)(y * pixelSize - headtoy), 64, 64, 1, getTileRotation(LevelTile), zoom, (unsigned char *)getTile(getLevelTile(gameState->level, x, y)));
      #else
      cross_drawBitmapTile((int)(x * pixelSize - headtox), (int)(y * pixelSize - headtoy), 64, 64, 1, 0, zoom, (unsigned char *)getTile(getLevelTile(gameState->level, x, y)));
      #endif
      }
    }
  }

  for (int x = endx; x < 128; x++)
  {
    cross_drawVLine(x, 0, 64, 0);
  }

  return headtoy == (int)(FIXP_TO_FLOAT(gameState->player1.X)) && headtoy == (int)(FIXP_TO_FLOAT(gameState->player1.Y));
}

void drawContinueMenu() {
  if ((getCurrentMs() / 1000) % 2 == 0)
  {
    if (saveData.maxLevel > gameState->level) cross_print(64, 64-16, 1, "A - Next");
    cross_print(64, 64-8, 1, "B - Retry");
  } 
}

void drawTrophySheet() {
  cross_drawHLine(0,13,128,1);
  cross_drawHLine(0,26,128,1);
  cross_drawHLine(0,39,128,1);
  cross_drawHLine(0,52,128,1);
  cross_print(3,17,1,"G");
  cross_print(1,31,1,"S");
  cross_print(3,43,1,"B");

  for (int level = 0; level < 10; level++) {
    #ifdef ARDUINO
    sprintf(string,"%d",level+1);
    #else
    sprintf_s(string,"%d",level+1);
    #endif
    cross_print(16 + level * 11,3,1,string);
    cross_drawVLine(15 + level * 11,0,64,1);
    if (saveData.BestLapTimes[level] > 0)
    for (int i = 0; i < 3; i++) {
      #ifdef ARDUINO
      if (saveData.BestLapTimes[level] < (unsigned int)pgm_read_dword(LevelTimes[level]+ i))
      #else
      if (saveData.BestLapTimes[level] < LevelTimes[level][i])
      #endif
      {
        cross_print(17 + level * 11,4 + (1+i) * 13 ,1,"X");
        break;
      }
    }
  }
}

void drawGoalTimes() {
  setLevelString();
  cross_print(74, 0, 1, string);
  cross_drawHLine(64,8,64,1);

  setLevelTimeString('B', saveData.BestLapTimes[gameState->level - 1]);
  cross_print(74, 10, 1, string);
  cross_drawHLine(64, 19, 64, 1);

  drawBestTimes();

  cross_drawHLine(64, 46, 64, 1);

  drawContinueMenu();
  cross_print(0, 64-7, 1, "L - Times");
}

void drawAllTimes() {
  setLevelString();
  cross_print(74, 0, 1, string);
  cross_drawHLine(64, 8, 64, 1);

  for (int i = 0; i < TIMED_LAPS; i++) {
    setLevelTimeString(i+ 1 + 48, gameState->laptimes[i]);
    cross_print(74,9+8*i,1,string);
  }

  drawContinueMenu();
  cross_print(0, 64-7, 1, "R - Goal");
  cross_drawHLine(64, 64-17, 128-64, 1);
}

void drawTrophy() {
  cross_drawBitmapTile(0 , 0, 32 , 64, 1, 0, 1, (unsigned char *)Trophy);
  cross_drawBitmapTile(32, 0, 32 , 64, 1, 1, 1, (unsigned char *)Trophy);
  
  if (gameState->curlap == 0) {
    drawGoalTimes();
  } else if (gameState->curlap == 1) {
    drawAllTimes();
  };
};

void inputTrophy() {
  if (cross_input_left()) {
    gameState->curlap = 1;
  }

  if (cross_input_right()) {
    gameState->curlap = 0;
  }

  if (cross_input_a()) {
    if (gameState->level < saveData.maxLevel) {
      gameState->level += 1;
      if (gameState->level > 10) gameState->level = 10;
      gameState->mode = 4;
    }
  }

  if (cross_input_b()) {
    gameState->mode = 4;
  }
};

void racerLoop()
{
  if (!cross_loop_start())
    return;

  tFrameMs = getFrameMs();

  switch (gameState->mode)
  {
  case 0: // Intro screen
    if (gameState->lastmode != gameState->mode)
    {
      gameState->lastmode = gameState->mode;
      gameState->timeout = 1000;
      cross_clear_screen();
      cross_drawBitmapTile(0, 0, 128, 64, (unsigned char *)ArduRacerLogo);
    }
    else
    {
      if (tFrameMs < 1000)
        gameState->timeout -= tFrameMs;
      if (gameState->timeout < 0)
      {
        gameState->mode += 1;
      }
    }
    break;
  case 1: // Menu
    if (gameState->lastmode != gameState->mode)
    {
      gameState->lastmode = gameState->mode;
    }
    else
    {
      cross_clear_screen();
      cross_drawBitmapTile(0, 0, 128, 64, (unsigned char *)ArduRacerLogo);
      processMenu();
      updateMenu();
      displayMenu(gameState->menuItem);
    }
    break;
  case 4: // Level Start
    if (gameState->lastmode != gameState->mode)
    {
      gameState->lastmode = gameState->mode;
      gameState->timeout = 1000; // 1 second for start screen.
      cross_clear_screen();
      displayMap();
    }
    else
    {
      displayLevelInfo();
      if (gameState->timeout <= 0)
        inputLevelInfo();
    }

    break;
  case 5: // Zoom in
    if (gameState->lastmode != gameState->mode)
    {
      gameState->lastmode = gameState->mode;
      gameState->timeout = (int)ZOOM_TIME;
      setLevelDetails();
    }
    else
    {
      cross_clear_screen();
      displayLevelZoom();
      gameState->timeout -= getFrameMs();
      if (gameState->timeout <= 0)
      {
        gameState->mode = 6;
      }
    }

    break;
  case 6: // 3,2,1,GO!
    if (gameState->lastmode != gameState->mode)
    {
      setLevelDetails();
      gameState->lastmode = gameState->mode;
      gameState->timeout = (int)3000;
    }
    else
    {
      cross_clear_screen();
      displayGameMode();
      int lastnumber = gameState->timeout == 3000 ? 4 : gameState->timeout / 1000 + 1;
      
      gameState->timeout -= getFrameMs();
      int distance = gameState->timeout % 1000;
      int number = gameState->timeout / 1000 + 1;
      float mod = (1000.0f - distance) / 1000.0f;
      int numMod = (int)(32 * mod);
      cross_drawBitmapTile(32 + numMod, 0 + numMod, 64, 64, 1, 0, 1.0f - mod, (unsigned char *)getNumber(number));
      if (number >= 0 && lastnumber != number) 
        cross_playSound(saveData.sound, 440, 100);
      if (gameState->timeout <= 0)
      {
        gameState->mode = 10;
        cross_playSound(saveData.sound, 440, 255);
      }
    }

    break;
  case 7: // Celebrate! Snow Explosion
    if (gameState->lastmode != gameState->mode)
    {
      gameState->lastmode = gameState->mode;
      gameState->timeout = (int)300;
    }
    else
    {
      cross_clear_screen();
      displayGameMode();
      gameState->timeout -= getFrameMs();
      if (gameState->timeout <= 0)
      {
        gameState->mode = 8;
        // Win Screen / Next Level Select
      }
    }

    break;
  case 8: // Win Screen / Next Level Select
    if (gameState->lastmode != gameState->mode)
    {
      gameState->lastmode = gameState->mode;
      gameState->timeout = (int)1000;
      gameState->curlap = 0;
      gameState->newbestLap = false;
    }
    else
    {
      cross_clear_screen();
      drawTrophy();
      gameState->timeout -= getFrameMs();

      if (gameState->timeout <= 0)
      {
        inputTrophy();
      }
    }

    break;
  case 10: // Play Game
    if (gameState->lastmode != gameState->mode)
    {
      setLevelDetails();
      gameState->laptimer = true;
      gameState->lastmode = gameState->mode;
    }

    cross_clear_screen();
    processGameMode();
    updateGameMode();
    displayGameMode();
    if (gameState->curlap == TIMED_LAPS)
    {
      gameState->mode = 7;
    }
    break;
  case 98: // Trophy Screen
    cross_clear_screen();
    drawTrophySheet();
    if (cross_input_right()) gameState->mode = 0;
    break;
  case 99: // Clear Data
    if (gameState->lastmode != gameState->mode)
    {
      gameState->curlap = 0;
      gameState->lastmode = gameState->mode;
      gameState->timeout = (int)1000;
    } else {
      cross_clear_screen();
      switch (gameState->curlap)
      {
      case 0: // Yes A
        cross_print(0, 0, 3, "A - Delete");
        cross_print(0, 30, 3, "B - Abort");
        gameState->timeout -= getFrameMs();
        if (gameState->timeout <= 0) {
          if (cross_input_a()) {
            gameState->timeout = 1000;
            gameState->curlap++;
          } else if (cross_input_b()) {
            gameState->enter = false;
            gameState->mode = 0;
            gameState->curlap = 0;
            gameState->timeout = 1000;
        }
        }
        break; 
      case 1: // Yes B
        gameState->timeout -= getFrameMs();
        cross_print(0, 0, 3, "B - Delete");
        cross_print(0, 30, 3, "A - Abort");
        if (gameState->timeout <= 0) {
          if (cross_input_b()) {
            gameState->curlap++;
            gameState->timeout = 1000;
          } else if (cross_input_a()) {
            gameState->enter = false;
            gameState->mode = 0;
            gameState->curlap = 0;
            gameState->timeout = 1000;
          }
        }
        break;
      case 2: // Pull the trigger
        gameState->enter = false;
        gameState->mode = 0;
        gameState->curlap = 0;
        SaveData data;

        for (int i = 0; i < LEVELS; i++)
        {
          data.BestLapTimes[i] = 0;
        }
        data.maxLevel = 1;

        cross_save(saveStartAddress, data);
        saveData = data;
        break;
      }
    }
  }

  cross_loop_end();
}

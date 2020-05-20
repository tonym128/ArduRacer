#ifndef _STRUCTS_H_
#define _STRUCTS_H_
#include "fixpoint.h"
constexpr uint8_t TIMED_LAPS = 5;

struct Vec3 {
  FIXPOINT x,y;
  FIXPOINT direction;
  FIXPOINT force;
  FIXPOINT time;
};

struct Player {
  FIXPOINT X,Y;
  double rotation;
  Vec3 acceleration;
  bool offRoad = false;
};

struct CheckPoint {
  uint8_t x;
  uint8_t y;
  bool passed;
  CheckPoint *nextcheckpoint;
};

constexpr uint16_t saveStartAddress = 812;
constexpr uint16_t GAMENO = 1293;
constexpr uint16_t SAVEVER = 2;
constexpr uint8_t LEVELS = 10; // 10 Levels

struct SaveData
{
	int GameNo = GAMENO; // Random Game No
	int SaveVer = SAVEVER; // Save Version
	unsigned int BestLapTimes[LEVELS]; 
  uint8_t maxLevel = 1;
  bool sound = false;
};

struct GameState {

  uint8_t level;
  uint8_t lasttile;
  uint8_t lastx;
  uint8_t lasty;

  unsigned int laptimes[TIMED_LAPS];
  unsigned int bestLap;
  bool newbestLap;
  CheckPoint *levelCheckPoints;
  bool laptimer;
  bool paused;
  bool startCross;
  bool enter;
  uint8_t curlap;
  uint8_t checkpoints;
  uint8_t checkpointpassed;
  Player player1;

  FIXPOINT StageX = FIXP_TO_INT(0), StageY = FIXP_TO_INT(0);

  uint8_t mode = 0;
  uint8_t lastmode = -1;
  uint8_t menuItem = 0;

  int timeout;
};

#endif

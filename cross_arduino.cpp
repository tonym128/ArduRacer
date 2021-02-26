#include "cross_arduino.h"
#include <Arduboy2.h>
#ifndef ESP32
#include <ArduboyTones.h>
#endif

Arduboy2 arduboy;
#ifndef ESP32
ArduboyTones sound(arduboy.audio.enabled);
#endif

#define ARDBITMAP_SBUF arduboy.getBuffer()
#include "ArdBitmap.h"
ArdBitmap<WIDTH, HEIGHT> ardbitmap;
unsigned long currentTime, frameTime, fps, frameMs = 0;
const unsigned int FRAME_RATE = 30; // Frame rate in frames per second

bool begin = false;
void spiffBegin() {
    if (!begin) {
        begin = true;
        Serial.println("SPIFFS Init");
        if (!SPIFFS.begin(true))
        {
            Serial.println("Mount Failed");
            SPIFFS.format();
            Serial.println("File formatted");
            return;
        }
    }
}

char *spiffsLoadFile(char *fileName)
{
  Serial.print("Loading file : ");
  Serial.println(fileName);
  char *prepend = (char *)malloc(strlen(fileName) + 2);
  sprintf(prepend, "/%s", fileName);
  fs::File file = SPIFFS.open(prepend, "r");
  free(prepend);

  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return nullptr;
  }

  if (file.size() == 0)
  {
    Serial.println("Empty file");
    return nullptr;
  }

  char *fileData = (char *)malloc(file.size());
  file.readBytes(fileData, file.size());
  return fileData;
}

bool spiffsSaveFile(char *fileName, char *data)
{
  Serial.print("Saving file :");
  Serial.println(fileName);
  char *prepend = (char *)malloc(strlen(fileName) + 2);
  sprintf(prepend, "/%s", fileName);
  fs::File file = SPIFFS.open(prepend, FILE_WRITE);
  free(prepend);

  if (!file)
  {
    Serial.println("There was an error opening the file for writing");
    return false;
  }

  if (file.write((const uint8_t *)data,sizeof(SaveData)))
  {
    Serial.println("File was written");
  }
  else
  {
    Serial.println("File write failed");
    return false;
  }

  file.close();
  return true;
}

void cross_save(int saveStartAddress, SaveData saveData) {
	SaveData * eepromPointer = reinterpret_cast<SaveData *>(saveStartAddress);
#ifdef ESP32
    spiffsSaveFile("data.sav", (char*)&saveData);
#else
    eeprom_update_block(&saveData, eepromPointer, sizeof(SaveData));
#endif
}

SaveData cross_load(int saveStartAddress) {
  SaveData *saveData = new SaveData();
#ifdef ESP32
    saveData = (SaveData *)spiffsLoadFile("game.sav");
    if (saveData == nullptr) {
        saveData = new SaveData();
        return *saveData;
    }
#else
 	  SaveData* eepromPointer = reinterpret_cast<SaveData *>(saveStartAddress);
	  eeprom_read_block(saveData, eepromPointer, sizeof(SaveData));
#endif

    if (saveData->GameNo == GAMENO && saveData->SaveVer == SAVEVER) {
        return *saveData;
    }

    free(saveData);

    saveData = new SaveData();
    return *saveData;
}

bool cross_input_up()
{
    return arduboy.pressed(UP_BUTTON);
}
bool cross_input_down()
{
    return arduboy.pressed(DOWN_BUTTON);
}
bool cross_input_left()
{
    return arduboy.pressed(LEFT_BUTTON);
}

bool cross_input_right()
{
    return arduboy.pressed(RIGHT_BUTTON);
}
bool cross_input_a()
{
    return arduboy.pressed(A_BUTTON);
}
bool cross_input_b()
{
    return arduboy.pressed(B_BUTTON);
}

void cross_print(int x, int y, int size, char *string) {
    arduboy.setCursor(x,y);
    arduboy.setTextSize(size);
    arduboy.print(string);
}

void cross_drawPixel(int x, int y, bool colour)
{
    arduboy.drawPixel(x, y, colour);
}

void cross_drawHLine(int x, int y, int length, bool colour)
{
    arduboy.drawFastHLine(x,y,length,colour);
}

void cross_drawVLine(int x, int y, int length, bool colour)
{
    arduboy.drawFastVLine(x,y,length,colour);
}

void cross_drawBitmapTile(int x, int y, int width, int height, int colour, int mirror, float zoom, unsigned char *tile)
{
    ardbitmap.drawCompressedResized(x, y, tile, colour, 0, mirror, zoom);
}

bool cross_getPixel(int x, int y) {
    return arduboy.getPixel(x,y);
}

void cross_playSound(bool makeSound, uint16_t hertz, uint8_t duration) {
#ifndef ESP32
  if (makeSound) sound.tone(hertz, duration); // play a 1000Hz tone for 500ms
#endif
};

void cross_setup()
{
    arduboy.begin();
    arduboy.setFrameRate(FRAME_RATE);
    arduboy.initRandomSeed();
    arduboy.audio.on();
}

void cross_clear_screen() {
        arduboy.clear();
}

bool cross_loop_start()
{
    if (!(arduboy.nextFrame()))
        return false;

    frameTime = currentTime;
    currentTime = millis();
    frameMs = currentTime - frameTime;
    // if (frameMs == 0)
    //     frameMs = 1;
    
    arduboy.setCursor(0, 0);
    //arduboy.pollButtons();

    return true;
}

void cross_loop_end()
{
    arduboy.display();
}

unsigned long getFrameMs()
{
    return frameMs;
}

unsigned long getCurrentMs()
{
    return currentTime;
}

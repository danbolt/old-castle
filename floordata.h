#ifndef FLOOR_DATA_H
#define FLOOR_DATA_H

#include "xorshift.h"

#include "RoomData.h"

#define NO_PREVIOUS_FLOOR -1

#define NUMBER_OF_FLOORS 16
extern xorshift32_state grandSeed; // The seed of the entire playthrough
extern xorshift32_state roomSeeds[NUMBER_OF_FLOORS]; // The individual seeds for each floor

#define EXIT_COUNT 5
extern s8 exitMap[NUMBER_OF_FLOORS][EXIT_COUNT];

extern s8 roomsCleared[NUMBER_OF_FLOORS][MAX_NUMBER_OF_ROOMS_PER_FLOOR];

#define NUMBER_OF_SPECIAL_KEYS 16
extern u8 playerHasSpecialKey[NUMBER_OF_SPECIAL_KEYS];
typedef enum {
  SpecialKey_Red = 0,
  SpecialKey_Blue = 1,
  SpecialKey_Green = 2,
  SpecialKey_Purple = 3,
  SpecialKey_White = 4,
  SpecialKey_Black = 5,
  SpecialKey_Grey = 6,
  SpecialKey_Pink = 7,
  SpecialKey_Turquoise = 8,
  SpecialKey_Orange = 9,
  SpecialKey_Brown = 10,
  SpecialKey_Gold = 11,
  SpecialKey_Maroon = 12,
  SpecialKey_Teal = 13,
  SpecialKey_Magenta = 14,
  SpecialKey_NavyBlue = 15,
} SpecialKeyType;

void initializeSeeds();

void initalizeConnections();

void unClearAllRooms();
int hasRoomBeenCleared(int floor, int roomIndex);
void clearRoom(int floor, int roomIndex);

void initalizeSpecialKeysState();
int hasSpecialKey(SpecialKeyType specialKey);
void giveSpecialKey(SpecialKeyType specialKey);

#endif
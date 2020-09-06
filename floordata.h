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

void initializeSeeds();

void initalizeConnections();

void unClearAllRooms();
int hasRoomBeenCleared(int floor, int roomIndex);
void clearRoom(int floor, int roomIndex);

#endif
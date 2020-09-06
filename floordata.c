

#include "floordata.h"

xorshift32_state grandSeed;
xorshift32_state roomSeeds[NUMBER_OF_FLOORS];

s8 exitMap[NUMBER_OF_FLOORS][EXIT_COUNT];

s8 roomsCleared[NUMBER_OF_FLOORS][MAX_NUMBER_OF_ROOMS_PER_FLOOR];

void initializeSeeds() {
	int i;

	// TODO: randomize this per tick + input
	grandSeed.a = 82675341;

	for (i = 0; i < NUMBER_OF_FLOORS; i++) {
		roomSeeds[i].a = xorshift32(&grandSeed);
	}
}

void initalizeConnections() {
  int i, j;

  // Clear out all the paths
	for (i = 0; i < NUMBER_OF_FLOORS; i++) {
		for (j = 0; j < EXIT_COUNT; j++) {
			exitMap[i][j] = -1;
		}
	}

	// The Foyer is more or less the "hub room"
	exitMap[0][0] = 1;
	exitMap[0][1] = 4;
	exitMap[0][2] = 7;
	exitMap[0][3] = 10;

	// Basement 1
	exitMap[1][0] = 0;
	exitMap[1][1] = 2;
	// Basement 2
	exitMap[2][0] = 1;
	exitMap[2][1] = 3;
	// Basement 3 "cavern"
	exitMap[3][0] = 2;

	// Garden 1
	exitMap[4][0] = 0;
	exitMap[4][1] = 5;
	// Garden 2
	exitMap[5][0] = 4;
	exitMap[5][1] = 6;
	// Garden 3 "greenhouse"
	exitMap[6][0] = 5;

	// Ballroom 1
	exitMap[7][0] = 0;
	exitMap[7][1] = 8;
	// Ballroom 2
	exitMap[8][0] = 7;
	exitMap[8][1] = 9;
	// Ballroom 3 "chambers"
	exitMap[9][0] = 8;

	// The final room
	exitMap[10][0] = 0;
	exitMap[10][1] = 11;

	// A long corridor
	exitMap[11][0] = 10;
	exitMap[11][1] = 12;

	// The room with the ending
	exitMap[12][0] = 11;
}

void unClearAllRooms() {
	int i;
	int j;

	for (i = 0; i < NUMBER_OF_FLOORS; i++) {
		for (j = 0; j < MAX_NUMBER_OF_ROOMS_PER_FLOOR; j++) {
			roomsCleared[i][j] = 0;
		}
	}
}

int hasRoomBeenCleared(int floor, int roomIndex) {
	return roomsCleared[floor][roomIndex];
}

void clearRoom(int floor, int roomIndex) {
	roomsCleared[floor][roomIndex] = 1;
}

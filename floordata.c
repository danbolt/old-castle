

#include "floordata.h"

xorshift32_state grandSeed;
xorshift32_state roomSeeds[NUMBER_OF_FLOORS];

s8 exitMap[NUMBER_OF_FLOORS][EXIT_COUNT];

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
	exitMap[0][0] = 1;
	exitMap[0][1] = 4;
	exitMap[0][2] = 7;
	exitMap[0][3] = 10;
}

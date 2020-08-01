#ifndef FLOOR_DATA_H
#define FLOOR_DATA_H

#include "xorshift.h"

#define NUMBER_OF_FLOORS 16
extern xorshift32_state grandSeed; // The seed of the entire playthrough
extern xorshift32_state roomSeeds[NUMBER_OF_FLOORS]; // The individual seeds for each floor

#define EXIT_COUNT 5
extern s8 exitMap[NUMBER_OF_FLOORS][EXIT_COUNT];

void initializeSeeds();

void initalizeConnections();

#endif
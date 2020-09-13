

#include "floordata.h"

xorshift32_state grandSeed;
xorshift32_state roomSeeds[NUMBER_OF_FLOORS];

s8 exitMap[NUMBER_OF_FLOORS][EXIT_COUNT];

s8 roomsCleared[NUMBER_OF_FLOORS][MAX_NUMBER_OF_ROOMS_PER_FLOOR];

u8 playerHasSpecialKey[NUMBER_OF_SPECIAL_KEYS];

KeyColor colorMappng[] = {
	{ 212, 0, 0 },
	{ 0, 0, 212 },
	{ 0, 212, 0 },
	{ 152, 66, 245 },
	{ 255, 255, 255 },
	{ 50, 50, 50 },
	{ 255, 255, 0 },
	{ 250, 145, 175 },
	{ 81, 109, 219 },
	{ 255, 98, 0 },
	{ 66, 37, 6 },
	{ 176, 136, 42 },
	{ 125, 19, 0 },
	{ 41, 242, 162 },
	{ 255, 0, 255 },
	{ 0, 9, 112 }
};

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

void initalizeSpecialKeysState() {
	int i;

	for (i = 0; i < NUMBER_OF_SPECIAL_KEYS; i++) {
		playerHasSpecialKey[i] = 0;
	}
}

int hasSpecialKey(SpecialKeyType specialKey) {
	int casted = (int)(specialKey);

	if ((casted < 0) || (casted >= NUMBER_OF_SPECIAL_KEYS)) {
		return 0;
	}

	return playerHasSpecialKey[casted];
}

void giveSpecialKey(SpecialKeyType specialKey) {
	int casted = (int)(specialKey);

	if ((casted < 0) || (casted >= NUMBER_OF_SPECIAL_KEYS)) {
		return;
	}

	playerHasSpecialKey[casted] = 1;
}

KeyColor* getKeyColor(SpecialKeyType specialKey) {
	int casted = (int)(specialKey);

	if ((casted < 0) || (casted >= NUMBER_OF_SPECIAL_KEYS)) {
		return NULL;
	}

	return &(colorMappng[casted]);
}

const char* getKeyAdjective(SpecialKeyType specialKey) {
	switch (specialKey) {
		case SpecialKey_Red: return "Gauche"; 
		case SpecialKey_Blue: return "Milquetoast"; 
		case SpecialKey_Green: return "Uneasy"; 
		case SpecialKey_Purple: return "Brainless"; 
		case SpecialKey_White: return "Dull"; 
		case SpecialKey_Black: return "Wisful"; 
		case SpecialKey_Yellow: return "Daring"; 
		case SpecialKey_Pink: return "Unlovable"; 
		case SpecialKey_Turquoise: return "Infantile"; 
		case SpecialKey_Orange: return "Giving"; 
		case SpecialKey_Brown: return "Dejected"; 
		case SpecialKey_Gold: return "Exhausted"; 
		case SpecialKey_Maroon: return "Ardent"; 
		case SpecialKey_Teal: return "Careful"; 
		case SpecialKey_Magenta: return "Unsatisfying"; 
		case SpecialKey_NavyBlue: return "Normal"; 
		default: return "BADPREFIX";
	}
}

const char* getKeyName(SpecialKeyType specialKey) {
	switch (specialKey) {
		case SpecialKey_Red: return "Blood"; 
		case SpecialKey_Blue: return "Rainfall"; 
		case SpecialKey_Green: return "Respite"; 
		case SpecialKey_Purple: return "Grandeur"; 
		case SpecialKey_White: return "Blindness"; 
		case SpecialKey_Black: return "Angrogyny"; 
		case SpecialKey_Yellow: return "Stares"; 
		case SpecialKey_Pink: return "Honesty"; 
		case SpecialKey_Turquoise: return "Nostalgia"; 
		case SpecialKey_Orange: return "Naivety"; 
		case SpecialKey_Brown: return "Earnesty"; 
		case SpecialKey_Gold: return "Arrogance"; 
		case SpecialKey_Maroon: return "Stupidity"; 
		case SpecialKey_Teal: return "Heartlessness"; 
		case SpecialKey_Magenta: return "Emptyness"; 
		case SpecialKey_NavyBlue: return "Navy Blue"; 
		default: return "BADNAME";
	}
}



#include "RoomData.h"

#include "EntityData.h"
#include "game_math.h"
#include "graphic.h"

#define WARP_EPSILON 0.001f

static u8 MapInfo[MAP_SIZE * MAP_SIZE];
#define IS_TILE_BLOCKED(x, y) MapInfo[x + (y * MAP_SIZE)]

#define DARKEN_VERT 0x20

#define FLOOR_COLOR_1 0x62
#define FLOOR_COLOR_1_VAR 0x13
#define FLOOR_COLOR_2 0x7f
#define FLOOR_COLOR_2_VAR 0x0f

#define WALL_COLOR_R 0x4f
#define WALL_COLOR_G 0x4c
#define WALL_COLOR_B 0x2f

static xorshift32_state roomGeneratorState;

static Mtx tilesWarp[(RENDER_DISTANCE_IN_TILES + 2) * 2];

void fillInRooms(GeneratedRoom* rooms, int roomCount) {
	int i;
	int x;
	int y;

	int staircasesPlaced = 0;
	for (i = 0; i < roomCount; i++) {
		GeneratedRoom* room = &(rooms[i]);

		for (x = room->x; x < (room->x + room->width); x++) {
			for (y = room->y; y < (room->y + room->height); y++) {
				MapInfo[(y * MAP_SIZE) + x] = FLOOR_TILE;
			}
		}

		if (room->type == StaircaseRoom) {
			int midX = (room->x + (room->width / 2));
			int midY = (room->y + (room->height / 2));

			MapInfo[(midY * MAP_SIZE) + midX] = STAIRCASE_A + room->stairsDirectionIndex;
			staircasesPlaced++;
		}
	}
}

void fillInHighWalls() {
	int i;

	for (i = 0; i < (MAP_SIZE * MAP_SIZE); i++) {
		if (MapInfo[i] == EMPTY_HIGH_WALL_TILE) {
		    short x = i % MAP_SIZE;
		    short y = i / MAP_SIZE;

		    if ((x < (MAP_SIZE - 1)) && (FLOOR_TILE == IS_TILE_BLOCKED((x + 1), y))) {
		    	MapInfo[i] = HIGH_WALL_TILE;
		    	continue;
		    }
		    if ((x > 0) && (FLOOR_TILE == IS_TILE_BLOCKED((x - 1), y))) {
		    	MapInfo[i] = HIGH_WALL_TILE;
		    	continue;
		    }
		    if ((y < (MAP_SIZE - 1)) && (FLOOR_TILE == IS_TILE_BLOCKED(x, (y + 1)))) {
		    	MapInfo[i] = HIGH_WALL_TILE;
		    	continue;
		    }
		    if ((y > 0) && (FLOOR_TILE == IS_TILE_BLOCKED(x, (y - 1)))) {
		    	MapInfo[i] = HIGH_WALL_TILE;
		    	continue;
		    }
		}
	}
}

// Creates the "room layout" consisting of two main rooms and optional side corridors
int generateFloorInStyleA(GeneratedRoom* rooms) {
	int i;

	int mainCorridorLength = (xorshift32(&roomGeneratorState) % 25) + 10;

	rooms[0].x = (MAP_SIZE / 2) - 8;
	rooms[0].y = MAP_SIZE - 10;
	rooms[0].width = 16;
	rooms[0].height = 8;
	rooms[0].type = StartingRoom;

	rooms[1].x = (MAP_SIZE / 2) - 2;
	rooms[1].y = MAP_SIZE - 10 - (u8)mainCorridorLength;
	rooms[1].width = 4;
	rooms[1].height = (u8)mainCorridorLength;
	rooms[1].type = HallwayRoom;

	rooms[2].x = (MAP_SIZE / 2) - 10;
	rooms[2].y = MAP_SIZE - 10 - (u8)mainCorridorLength - BOSS_A_ROOM_HEIGHT;
	rooms[2].width = BOSS_A_ROOM_WIDTH;
	rooms[2].height = BOSS_A_ROOM_HEIGHT;
	rooms[2].type = BossARoom;

	rooms[3].x = MAP_SIZE / 4;
	rooms[3].y = MAP_SIZE - 10 - ((u8)mainCorridorLength / 2);
	rooms[3].width = MAP_SIZE / 2;
	rooms[3].height = 4;
	rooms[3].type = HallwayRoom;

	// Side room A
	rooms[4].x = 8;
	rooms[4].y = MAP_SIZE - 10 - ((u8)mainCorridorLength / 2) - 7;
	rooms[4].width = (MAP_SIZE / 4) - 8;
	rooms[4].height = 14;
	rooms[4].type = RestRoom;

	// Side room B
	rooms[5].x = (MAP_SIZE / 4) + (MAP_SIZE / 2);
	rooms[5].y = MAP_SIZE - 10 - (mainCorridorLength / 2) - 7;
	rooms[5].width = 14;
	rooms[5].height = 14;
	rooms[5].type = RestRoom;

	// A's subrooms
	rooms[6].x = rooms[4].x;
	rooms[6].y = rooms[4].y - 25;
	rooms[6].width = (MAP_SIZE / 4) - 8;
	rooms[6].height = 9 + (xorshift32(&roomGeneratorState) % 7);
	rooms[6].type = StaircaseRoom;
  rooms[6].stairsDirectionIndex = 0;

	rooms[7].x = rooms[4].x + 4 + (xorshift32(&roomGeneratorState) % 6);
	rooms[7].y = (rooms[6].y + rooms[6].height);
	rooms[7].width = 3;
	rooms[7].height = rooms[4].y - (rooms[6].y + rooms[6].height);
	rooms[7].type = HallwayRoom;

	rooms[8].x = rooms[4].x + 4;
	rooms[8].y = rooms[4].y + rooms[4].height + 10;
	rooms[8].width = (MAP_SIZE / 4) - 8;
	rooms[8].height = 7 + (xorshift32(&roomGeneratorState) % 5);
	rooms[8].type = StaircaseRoom;
  rooms[8].stairsDirectionIndex = 1;

	rooms[9].x = rooms[4].x + 4 + (xorshift32(&roomGeneratorState) % 6);
	rooms[9].y = (rooms[4].y + rooms[4].height);
	rooms[9].width = 3;
	rooms[9].height = rooms[4].y - (rooms[6].y + rooms[6].height);
	rooms[9].type = HallwayRoom;

	// B's subrooms
	rooms[10].x = rooms[5].x;
	rooms[10].y = rooms[5].y - 25;
	rooms[10].width = (MAP_SIZE / 4) - 8;
	rooms[10].height = 9 + (xorshift32(&roomGeneratorState) % 7);
	rooms[10].type = StaircaseRoom;
  rooms[10].stairsDirectionIndex = 2;

	rooms[11].x = rooms[5].x + 4 + (xorshift32(&roomGeneratorState) % 6);
	rooms[11].y = (rooms[10].y + rooms[10].height);
	rooms[11].width = 3;
	rooms[11].height = rooms[5].y - (rooms[10].y + rooms[10].height);
	rooms[11].type = HallwayRoom;

	rooms[12].x = rooms[5].x + 4;
	rooms[12].y = rooms[5].y + rooms[5].height + 10;
	rooms[12].width = (MAP_SIZE / 4) - 8;
	rooms[12].height = 7 + (xorshift32(&roomGeneratorState) % 5);
	rooms[12].type = StaircaseRoom;
  rooms[12].stairsDirectionIndex = 3;

	rooms[13].x = rooms[5].x + 4 + (xorshift32(&roomGeneratorState) % 6);
	rooms[13].y = (rooms[5].y + rooms[5].height);
	rooms[13].width = 3;
	rooms[13].height = rooms[5].y - (rooms[10].y + rooms[10].height);
	rooms[13].type = HallwayRoom;

	fillInRooms(rooms, 14);

	fillInHighWalls();

  return 14;
}

int generateBasementStyleFloor(GeneratedRoom* rooms) {
	int i;

	rooms[0].x = 1;
	rooms[0].y = 1;
	rooms[0].width = 10;
	rooms[0].height = 10;
	rooms[0].type = StaircaseRoom;
  rooms[0].stairsDirectionIndex = 0;

  rooms[1].x = 15;
  rooms[1].y = 1;
  rooms[1].width = 10;
  rooms[1].height = 10;
  rooms[1].type = StaircaseRoom;
  rooms[1].stairsDirectionIndex = 1;

  rooms[2].x = 11;
  rooms[2].y = 1;
  rooms[2].width = 4;
  rooms[2].height = 2;
  rooms[2].type = HallwayRoom;

	fillInRooms(rooms, 3);

	fillInHighWalls();

  return 3;
}

//u32 vertBuffUsage[MAX_NUMBER_OF_ROOMS_PER_FLOOR];

void createGenericDisplayData(GeneratedRoom* rooms, int numberOfGeneratedRooms) {
  int i;
  int j;
  int x;
  int y;
  for (i = 0; i < numberOfGeneratedRooms; i++) {
    GeneratedRoom* room = &(rooms[i]);
    Gfx* commandList = room->commands;
    Vtx* vertexList = room->verts;
    Vtx* lastBuffer = vertexList;

    // Fill in the floor tiles
    for (x = 0; x < rooms[i].width; x++) {
      for (y = 0; y < rooms[i].height; y++) {
        (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,         (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x00, 0xff, 0, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x00, 0, 0xff, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0xff, 0xff,    0xff };

        if ((vertexList - lastBuffer) >= 64u) {
          gSPVertex(commandList++, lastBuffer, 64, 0);

          for (j = 0; j < 64; j += 4) {
            gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
          }

          lastBuffer = vertexList;
        }
      }
    }

    if (lastBuffer != vertexList) {
      gSPVertex(commandList++, lastBuffer, (vertexList - lastBuffer), 0);
      for (j = 0; j < (vertexList - lastBuffer); j += 4) {
        gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
      }

      lastBuffer = vertexList;
    }

    // Fill in the wall tiles
    for (x = 0; x < rooms[i].width; x++) {
      if (!(isTileBlocked(room->x + x, room->y - 1))) {
        continue;
      }

      (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };
      (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };
      (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };
      (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };

      if ((vertexList - lastBuffer) >= 64u) {
        gSPVertex(commandList++, lastBuffer, 64, 0);

        for (j = 0; j < 64; j += 4) {
          gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
        }

        lastBuffer = vertexList;
      }
    }
    if (lastBuffer != vertexList) {
      gSPVertex(commandList++, lastBuffer, (vertexList - lastBuffer), 0);
      for (j = 0; j < (vertexList - lastBuffer); j += 4) {
        gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
      }

      lastBuffer = vertexList;
    }

    for (y = 0; y < rooms[i].height; y++) {
      if (isTileBlocked(room->x - 1, room->y + y)) {
        (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };

        if ((vertexList - lastBuffer) >= 64u) {
          gSPVertex(commandList++, lastBuffer, 64, 0);

          for (j = 0; j < 64; j += 4) {
            gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
          }

          lastBuffer = vertexList;
        }

        if (isTileBlocked(room->x + room->width + 1, room->y + y)) {
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0xff, 0, 0, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }
        }
      }
    }
    if (lastBuffer != vertexList) {
      gSPVertex(commandList++, lastBuffer, (vertexList - lastBuffer), 0);
      for (j = 0; j < (vertexList - lastBuffer); j += 4) {
        gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
      }

      lastBuffer = vertexList;
    }

    // vertBuffUsage[i] = vertexList - room->verts;

    gSPEndDisplayList(commandList++);
  }
}

int initMap(GeneratedRoom* rooms, xorshift32_state* seed, int floorNumber) {
  int i;
  int numberOfGeneratedRooms = 0;

  roomGeneratorState = *seed;

  // "Fill in" impassible tiles everywhere to start
  for (i = 0; i < (MAP_SIZE * MAP_SIZE); i++) {
  	MapInfo[i] = EMPTY_HIGH_WALL_TILE;
  }

  for (i = 0; i < MAX_NUMBER_OF_ROOMS_PER_FLOOR; i++) {
    rooms[i].stairsDirectionIndex = NO_STAIRS_DIRECTION;
  }

  //TODO: Turn the floor numbers from magic numbers into #define constants
  if (floorNumber == 0) {
  	numberOfGeneratedRooms = generateFloorInStyleA(rooms);
  } else {
  	numberOfGeneratedRooms = generateBasementStyleFloor(rooms);
  }

  // TODO: customize this for each area of the manor
  createGenericDisplayData(rooms, numberOfGeneratedRooms);

  return numberOfGeneratedRooms;
}

void initEnemiesForMap(GeneratedRoom* rooms) {
  int i;

  for (i = 0; i < MAX_NUMBER_OF_ROOMS_PER_FLOOR; i++) {
    if (rooms[i].type == EnemyRoom) {
      generateAimEmitterEntity((rooms[i].x + (rooms[i].width / 4)) * TILE_SIZE, (rooms[i].y  + (rooms[i].height / 4)) * TILE_SIZE);
      generateAimEmitterEntity((rooms[i].x + (rooms[i].width / 4 * 3)) * TILE_SIZE, (rooms[i].y  + (rooms[i].height / 4)) * TILE_SIZE);
      generateAimEmitterEntity((rooms[i].x + (rooms[i].width / 4 * 3)) * TILE_SIZE, (rooms[i].y  + (rooms[i].height / 4 * 3)) * TILE_SIZE);
      generateAimEmitterEntity((rooms[i].x + (rooms[i].width / 4)) * TILE_SIZE, (rooms[i].y  + (rooms[i].height / 4 * 3)) * TILE_SIZE);
      

      // MapInfo[((rooms[i].y + 4) * MAP_SIZE) + (rooms[i].x + 3)] = LOW_WALL_TILE;
      // MapInfo[((rooms[i].y + 4) * MAP_SIZE) + (rooms[i].x + rooms[i].width - 4)] = LOW_WALL_TILE;
      // MapInfo[((rooms[i].y + rooms[i].height - 5) * MAP_SIZE) + (rooms[i].x + 3)] = LOW_WALL_TILE;
      // MapInfo[((rooms[i].y + rooms[i].height - 5) * MAP_SIZE) + (rooms[i].x + rooms[i].width - 4)] = LOW_WALL_TILE;
    }

    if (rooms[i].type == BossARoom) {
      generateBossA((rooms[i].x + (rooms[i].width / 2)) * TILE_SIZE, (rooms[i].y  + (rooms[i].height / 2)) * TILE_SIZE);
    }
  }
}

int isTileBlocked(int x, int y) {
	return IS_TILE_BLOCKED(x, y);
}



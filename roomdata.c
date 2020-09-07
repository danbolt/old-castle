

#include "RoomData.h"

#include "EntityData.h"
#include "game_math.h"
#include "graphic.h"

#define WARP_EPSILON 0.001f

static u8 MapInfo[MAP_SIZE * MAP_SIZE];
#define IS_TILE_BLOCKED(x, y) MapInfo[x + (y * MAP_SIZE)]

#define DARKEN_VERT 0x14
#define WALL_COLOR_R 0x4f
#define WALL_COLOR_G 0x4c
#define WALL_COLOR_B 0x2f

#define STAIRS_R 0xff
#define STAIRS_G 0xe6
#define STAIRS_B 0xcc

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
	rooms[3].y = MAP_SIZE - 16 - ((u8)mainCorridorLength / 2);
	rooms[3].width = MAP_SIZE / 2;
	rooms[3].height = 4;
	rooms[3].type = HallwayRoom;

	// Side room A
	rooms[4].x = 8;
	rooms[4].y = MAP_SIZE - 10 - ((u8)mainCorridorLength / 2) - 7;
	rooms[4].width = (MAP_SIZE / 4) - 8;
	rooms[4].height = 10;
	rooms[4].type = RestRoom;

	// Side room B
	rooms[5].x = (MAP_SIZE / 4) + (MAP_SIZE / 2);
	rooms[5].y = MAP_SIZE - 10 - (mainCorridorLength / 2) - 7;
	rooms[5].width = 14;
	rooms[5].height = 10;
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

u32 vertBuffUsage[MAX_NUMBER_OF_ROOMS_PER_FLOOR];

void darkenFloorTiles(GeneratedRoom* room, Vtx* vertexList, int x, int y) {
  if (isTileBlocked(x + room->x, y + room->y)) {
    (*(vertexList - 4)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 4)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 4)).v.cn[2] -= DARKEN_VERT;
    (*(vertexList - 3)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 3)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 3)).v.cn[2] -= DARKEN_VERT;
    (*(vertexList - 2)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 2)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 2)).v.cn[2] -= DARKEN_VERT;
    (*(vertexList - 1)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 1)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 1)).v.cn[2] -= DARKEN_VERT;
  }

  if (isTileBlocked(x + room->x - 1, y + room->y)) {
    (*(vertexList - 4)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 4)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 4)).v.cn[2] -= DARKEN_VERT;
    (*(vertexList - 1)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 1)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 1)).v.cn[2] -= DARKEN_VERT;
  }

  if (isTileBlocked(x + room->x + 1, y + room->y)) {
    (*(vertexList - 3)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 3)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 3)).v.cn[2] -= DARKEN_VERT;
    (*(vertexList - 2)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 2)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 2)).v.cn[2] -= DARKEN_VERT;
  }
  if (isTileBlocked(x + room->x, y + room->y - 1)) {
    (*(vertexList - 3)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 3)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 3)).v.cn[2] -= DARKEN_VERT;
    (*(vertexList - 4)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 4)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 4)).v.cn[2] -= DARKEN_VERT;
  }
  if (isTileBlocked(x + room->x, y + room->y + 1)) {
    (*(vertexList - 1)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 1)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 1)).v.cn[2] -= DARKEN_VERT;
    (*(vertexList - 2)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 2)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 2)).v.cn[2] -= DARKEN_VERT;
  }

  if (isTileBlocked(x + room->x - 1, y + room->y - 1)) {
    (*(vertexList - 4)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 4)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 4)).v.cn[2] -= DARKEN_VERT;
  }
  if (isTileBlocked(x + room->x + 1, y + room->y - 1)) {
    (*(vertexList - 3)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 3)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 3)).v.cn[2] -= DARKEN_VERT;
  }
  if (isTileBlocked(x + room->x + 1, y + room->y + 1)) {
    (*(vertexList - 2)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 2)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 2)).v.cn[2] -= DARKEN_VERT;
  }
  if (isTileBlocked(x + room->x - 1, y + room->y + 1)) {
    (*(vertexList - 1)).v.cn[0] -= DARKEN_VERT;
    (*(vertexList - 1)).v.cn[1] -= DARKEN_VERT;
    (*(vertexList - 1)).v.cn[2] -= DARKEN_VERT;
  }
}

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
        const u8 floorToneA = (x % 2 == 0) ? 0x44 : 0x62;
        const u8 floorToneB = (x % 2 == 0) ? 0x62 : 0x44;

        (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,         (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, floorToneA, floorToneA - DARKEN_VERT, floorToneA - DARKEN_VERT, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, floorToneA, floorToneA - DARKEN_VERT, floorToneA - DARKEN_VERT, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, floorToneB, floorToneB - DARKEN_VERT, floorToneB - DARKEN_VERT, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, floorToneB, floorToneB - DARKEN_VERT, floorToneB - DARKEN_VERT, 0xff };

        darkenFloorTiles(room, vertexList, x, y);

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

    // Fill in the top wall tiles
    for (x = 0; x < rooms[i].width; x++) {
      if (!(isTileBlocked(room->x + x, room->y - 1))) {
        continue;
      }

      (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
      (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
      (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };
      (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };

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

    // Fill in the side wall tiles
    for (y = 0; y < rooms[i].height; y++) {
      if (isTileBlocked(room->x - 1, room->y + y)) {
        (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };

        if ((vertexList - lastBuffer) >= 64u) {
          gSPVertex(commandList++, lastBuffer, 64, 0);

          for (j = 0; j < 64; j += 4) {
            gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
          }

          lastBuffer = vertexList;
        }

        if (isTileBlocked(room->x + room->width + 1, room->y + y)) {
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };

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

    vertBuffUsage[i] = vertexList - room->verts;

    gSPEndDisplayList(commandList++);
  }
}

#define FOYER_HALL_FLOOR_A 0x33
#define FOYER_HALL_FLOOR_B 0x44

void createFoyerDisplayData(GeneratedRoom* rooms, int numberOfGeneratedRooms) {
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
        if ((isTileBlocked(x + room->x, y + room->y) >= STAIRCASE_A) && (isTileBlocked(x + room->x, y + room->y) <= STAIRCASE_E)) {
          int stairX = (((room->x + x) * TILE_SIZE + 1)) * ROOM_VERT_DATA_SCALE;
          int stairY = (((room->y + y) * TILE_SIZE + 1)) * ROOM_VERT_DATA_SCALE;
          for (j = 0; j < 8; j++) {
            const short stairHeight = (j * 65) - 90;
            const float percent = j / 8.f;
            const float percentNext = (j + 0.8) / 8.f;
            const float xOffset1 = (short)(cosf(percent * M_PI * 2) * 150);
            const float xOffset2 = (short)(cosf(percentNext * M_PI * 2) * 150);
            const float yOffset1 = (short)(sinf(percent * M_PI * 2) * 150);
            const float yOffset2 = (short)(sinf(percentNext * M_PI * 2) * 150);

            (*(vertexList++)) = (Vtx){ stairX, stairY, stairHeight, 0, 0, 0, STAIRS_R, STAIRS_G, STAIRS_B, 0xff };
            (*(vertexList++)) = (Vtx){ stairX + xOffset1, stairY + yOffset1, stairHeight, 0, 0, 0, STAIRS_R, STAIRS_G, STAIRS_B, 0xff };
            (*(vertexList++)) = (Vtx){ stairX + xOffset2, stairY + yOffset2, stairHeight, 0, 0, 0, 0x11, 0x11, 0x11, 0xff };
            (*(vertexList++)) = (Vtx){ stairX + xOffset2, stairY + yOffset2, stairHeight + 65, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }
          }

          continue;
        }

        if (room->type == HallwayRoom) {
          u8 tone1 = FOYER_HALL_FLOOR_A;
          u8 tone2 = FOYER_HALL_FLOOR_A;
          u8 tone3 = FOYER_HALL_FLOOR_A;
          u8 tone4 = FOYER_HALL_FLOOR_A;

          if (((x + room->x) % 4 == 0) && ((y + room->y) % 4 == 0)) {
            tone1 = FOYER_HALL_FLOOR_B;
            tone2 = FOYER_HALL_FLOOR_B;
            tone3 = FOYER_HALL_FLOOR_B;
            tone4 = FOYER_HALL_FLOOR_B;
          } else if (((x + room->x) % 4 == 1) && ((y + room->y) % 4 == 0)) {
            tone1 = FOYER_HALL_FLOOR_B;
            tone4 = FOYER_HALL_FLOOR_B;
          } else if (((x + room->x) % 4 == 3) && ((y + room->y) % 4 == 0)) {
            tone2 = FOYER_HALL_FLOOR_B;
            tone3 = FOYER_HALL_FLOOR_B;
          }

          if (((x + room->x) % 4 == 0) && ((y + room->y) % 4 == 3)) {
            tone3 = FOYER_HALL_FLOOR_B;
            tone4 = FOYER_HALL_FLOOR_B;
          } else if (((x + room->x) % 4 == 0) && ((y + room->y) % 4 == 1)) {
            tone1 = FOYER_HALL_FLOOR_B;
            tone2 = FOYER_HALL_FLOOR_B;
          }

          if (((x + room->x) % 4 == 1) && ((y + room->y) % 4 == 1)) {
            tone1 = FOYER_HALL_FLOOR_B;
          } else if (((x + room->x) % 4 == 3) && ((y + room->y) % 4 == 1)) {
            tone2 = FOYER_HALL_FLOOR_B;
          } else if (((x + room->x) % 4 == 1) && ((y + room->y) % 4 == 3)) {
            tone4 = FOYER_HALL_FLOOR_B;
          } else if (((x + room->x) % 4 == 3) && ((y + room->y) % 4 == 3)) {
            tone3 = FOYER_HALL_FLOOR_B;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,         (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, tone1, tone1, tone1, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, tone2, tone2, tone2, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, tone3, tone3, tone3, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, tone4, tone4, tone4, 0xff };
        } else {
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,         (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x66, 0x55, 0x55, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x66, 0x55, 0x55, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x66, 0x55, 0x55, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x66, 0x55, 0x55, 0xff };
        }

        darkenFloorTiles(room, vertexList, x, y);

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

    // Fill in the top wall tiles
    for (x = 0; x < rooms[i].width; x++) {
      if (!(isTileBlocked(room->x + x, room->y - 1))) {
        continue;
      }

      (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
      (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
      (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };
      (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };

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

    // Fill in the side wall tiles
    for (y = 0; y < rooms[i].height; y++) {
      if (isTileBlocked(room->x - 1, room->y + y)) {
        (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };

        if ((vertexList - lastBuffer) >= 64u) {
          gSPVertex(commandList++, lastBuffer, 64, 0);

          for (j = 0; j < 64; j += 4) {
            gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
          }

          lastBuffer = vertexList;
        }
      }

      if (isTileBlocked(room->x + room->width, room->y + y)) {
        (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };

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

    vertBuffUsage[i] = vertexList - room->verts;

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
    createFoyerDisplayData(rooms, numberOfGeneratedRooms);
  } else {
  	numberOfGeneratedRooms = generateBasementStyleFloor(rooms);
    createGenericDisplayData(rooms, numberOfGeneratedRooms);
  }

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



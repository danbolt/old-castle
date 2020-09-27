

#include "RoomData.h"

#include "EntityData.h"
#include "game_math.h"
#include "graphic.h"
#include "FloorData.h"
#include "main.h"

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

#define HALLWAY_WALLS_R 0x99
#define HALLWAY_WALLS_G 0x1f
#define HALLWAY_WALLS_B 0x00

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

    if (room->type == StartingRoom) {
      int x1 = (room->x + 3);
      int x2 = ((room->x + room->width) - 3);
      int midY = (room->y + (room->height / 3));
      MapInfo[(midY * MAP_SIZE) + x1] = HIGH_WALL_TILE;
      MapInfo[(midY * MAP_SIZE) + x2] = HIGH_WALL_TILE;

      MapInfo[((midY + 3) * MAP_SIZE) + (room->x + (room->width / 2) - 4)] = LOW_WALL_TILE;
      MapInfo[((midY + 3) * MAP_SIZE) + (room->x + (room->width / 2) + 3)] = LOW_WALL_TILE;
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
	rooms[2].type = EnemyRoom;

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
	rooms[4].type = EnemyRoom;

	// Side room B
	rooms[5].x = (MAP_SIZE / 4) + (MAP_SIZE / 2);
	rooms[5].y = MAP_SIZE - 10 - (mainCorridorLength / 2) - 7;
	rooms[5].width = 14;
	rooms[5].height = 10;
	rooms[5].type = EnemyRoom;

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

int generateBasementStyleFloor(GeneratedRoom* rooms, int floorNumber) {
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
  if (floorNumber == 4) {
    rooms[2].type = LockRoom;
    rooms[2].lockIndex = SpecialKey_Blue;
  } else if (floorNumber == 7) {
    rooms[2].type = LockRoom;
    rooms[2].lockIndex = SpecialKey_Green;
  } else if (floorNumber == 10) {
    rooms[2].type = LockRoom;
    rooms[2].lockIndex = SpecialKey_Purple;
  }

	fillInRooms(rooms, 3);

	fillInHighWalls();

  return 3;
}

int generateBossStyleFloor(GeneratedRoom* rooms, int floorNumber) {
  int i;

  rooms[0].x = MAP_SIZE / 2;
  rooms[0].y = BOSS_A_ROOM_HEIGHT + 5 + 1;
  rooms[0].width = 10;
  rooms[0].height = 10;
  rooms[0].type = StaircaseRoom;
  rooms[0].stairsDirectionIndex = 0;

  rooms[1].x = MAP_SIZE / 2;
  rooms[1].y = 1;
  rooms[1].width = BOSS_A_ROOM_WIDTH;
  rooms[1].height = BOSS_A_ROOM_HEIGHT;
  rooms[1].type = BossARoom;

  rooms[2].x = (MAP_SIZE / 2) + 2;
  rooms[2].y = BOSS_A_ROOM_HEIGHT + 1;
  rooms[2].width = 4;
  rooms[2].height = 5;
  rooms[2].type = HallwayRoom;

  fillInRooms(rooms, 3);

  fillInHighWalls();

  return 3;
}

u32 vertBuffUsage[MAX_NUMBER_OF_ROOMS_PER_FLOOR];

void darkenFloorTiles(GeneratedRoom* room, Vtx* vertexList, int x, int y) {
  if (isTileBlocked(x + room->x, y + room->y)) {
    (*(vertexList - 4)).v.cn[0] = MAX(0, ((*(vertexList - 4)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 4)).v.cn[1] = MAX(0, ((*(vertexList - 4)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 4)).v.cn[2] = MAX(0, ((*(vertexList - 4)).v.cn[2] - DARKEN_VERT));
    (*(vertexList - 3)).v.cn[0] = MAX(0, ((*(vertexList - 3)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 3)).v.cn[1] = MAX(0, ((*(vertexList - 3)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 3)).v.cn[2] = MAX(0, ((*(vertexList - 3)).v.cn[2] - DARKEN_VERT));
    (*(vertexList - 2)).v.cn[0] = MAX(0, ((*(vertexList - 2)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 2)).v.cn[1] = MAX(0, ((*(vertexList - 2)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 2)).v.cn[2] = MAX(0, ((*(vertexList - 2)).v.cn[2] - DARKEN_VERT));
    (*(vertexList - 1)).v.cn[0] = MAX(0, ((*(vertexList - 1)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 1)).v.cn[1] = MAX(0, ((*(vertexList - 1)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 1)).v.cn[2] = MAX(0, ((*(vertexList - 1)).v.cn[2] - DARKEN_VERT));
  }

  if (isTileBlocked(x + room->x - 1, y + room->y)) {
    (*(vertexList - 4)).v.cn[0] = MAX(0, ((*(vertexList - 4)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 4)).v.cn[1] = MAX(0, ((*(vertexList - 4)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 4)).v.cn[2] = MAX(0, ((*(vertexList - 4)).v.cn[2] - DARKEN_VERT));
    (*(vertexList - 1)).v.cn[0] = MAX(0, ((*(vertexList - 1)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 1)).v.cn[1] = MAX(0, ((*(vertexList - 1)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 1)).v.cn[2] = MAX(0, ((*(vertexList - 1)).v.cn[2] - DARKEN_VERT));
  }

  if (isTileBlocked(x + room->x + 1, y + room->y)) {
    (*(vertexList - 3)).v.cn[0] = MAX(0, ((*(vertexList - 3)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 3)).v.cn[1] = MAX(0, ((*(vertexList - 3)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 3)).v.cn[2] = MAX(0, ((*(vertexList - 3)).v.cn[2] - DARKEN_VERT));
    (*(vertexList - 2)).v.cn[0] = MAX(0, ((*(vertexList - 2)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 2)).v.cn[1] = MAX(0, ((*(vertexList - 2)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 2)).v.cn[2] = MAX(0, ((*(vertexList - 2)).v.cn[2] - DARKEN_VERT));
  }
  if (isTileBlocked(x + room->x, y + room->y - 1)) {
    (*(vertexList - 3)).v.cn[0] = MAX(0, ((*(vertexList - 3)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 3)).v.cn[1] = MAX(0, ((*(vertexList - 3)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 3)).v.cn[2] = MAX(0, ((*(vertexList - 3)).v.cn[2] - DARKEN_VERT));
    (*(vertexList - 4)).v.cn[0] = MAX(0, ((*(vertexList - 4)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 4)).v.cn[1] = MAX(0, ((*(vertexList - 4)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 4)).v.cn[2] = MAX(0, ((*(vertexList - 4)).v.cn[2] - DARKEN_VERT));
  }
  if (isTileBlocked(x + room->x, y + room->y + 1)) {
    (*(vertexList - 1)).v.cn[0] = MAX(0, ((*(vertexList - 1)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 1)).v.cn[1] = MAX(0, ((*(vertexList - 1)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 1)).v.cn[2] = MAX(0, ((*(vertexList - 1)).v.cn[2] - DARKEN_VERT));
    (*(vertexList - 2)).v.cn[0] = MAX(0, ((*(vertexList - 2)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 2)).v.cn[1] = MAX(0, ((*(vertexList - 2)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 2)).v.cn[2] = MAX(0, ((*(vertexList - 2)).v.cn[2] - DARKEN_VERT));
  }

  if (isTileBlocked(x + room->x - 1, y + room->y - 1)) {
    (*(vertexList - 4)).v.cn[0] = MAX(0, ((*(vertexList - 4)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 4)).v.cn[1] = MAX(0, ((*(vertexList - 4)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 4)).v.cn[2] = MAX(0, ((*(vertexList - 4)).v.cn[2] - DARKEN_VERT));
  }
  if (isTileBlocked(x + room->x + 1, y + room->y - 1)) {
    (*(vertexList - 3)).v.cn[0] = MAX(0, ((*(vertexList - 3)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 3)).v.cn[1] = MAX(0, ((*(vertexList - 3)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 3)).v.cn[2] = MAX(0, ((*(vertexList - 3)).v.cn[2] - DARKEN_VERT));
  }
  if (isTileBlocked(x + room->x + 1, y + room->y + 1)) {
    (*(vertexList - 2)).v.cn[0] = MAX(0, ((*(vertexList - 2)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 2)).v.cn[1] = MAX(0, ((*(vertexList - 2)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 2)).v.cn[2] = MAX(0, ((*(vertexList - 2)).v.cn[2] - DARKEN_VERT));
  }
  if (isTileBlocked(x + room->x - 1, y + room->y + 1)) {
    (*(vertexList - 1)).v.cn[0] = MAX(0, ((*(vertexList - 1)).v.cn[0] - DARKEN_VERT));
    (*(vertexList - 1)).v.cn[1] = MAX(0, ((*(vertexList - 1)).v.cn[1] - DARKEN_VERT));
    (*(vertexList - 1)).v.cn[2] = MAX(0, ((*(vertexList - 1)).v.cn[2] - DARKEN_VERT));
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

        
        if (isTileBlocked(x + room->x, y + room->y) == LOW_WALL_TILE) {
          // table top
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          // legs
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT - 50, 0, 0, 0, 0x2E - DARKEN_VERT, 0x1C - DARKEN_VERT, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT - 50, 0, 0, 0, 0x2E - DARKEN_VERT, 0x1C - DARKEN_VERT, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE - 40, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE - 20, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE + 40, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE + 20, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE - 40, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE - 20, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE + 40, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE + 20, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

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

#define ENTRANCE_FLOOR_A_R (DARKEN_VERT + (DARKEN_VERT / 2))
#define ENTRANCE_FLOOR_A_G (DARKEN_VERT + (DARKEN_VERT / 2))
#define ENTRANCE_FLOOR_A_B 0x3A
#define ENTRANCE_FLOOR_B_R (DARKEN_VERT + (DARKEN_VERT / 2))
#define ENTRANCE_FLOOR_B_G (DARKEN_VERT + (DARKEN_VERT / 2))
#define ENTRANCE_FLOOR_B_B (DARKEN_VERT + (DARKEN_VERT / 2))

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

        // Create a special segment of display list for a staircase
        // Sadly, I'm unsure of how to rotate it with a static display list :(
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

        if (isTileBlocked(x + room->x, y + room->y) == LOW_WALL_TILE) {
          // table top
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          // legs
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT - 50, 0, 0, 0, 0x2E - DARKEN_VERT, 0x1C - DARKEN_VERT, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, LOW_WALL_HEIGHT - 50, 0, 0, 0, 0x2E - DARKEN_VERT, 0x1C - DARKEN_VERT, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE - 40, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE - 20, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE + 40, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE + 20, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE - 40, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1)     * ROOM_VERT_DATA_SCALE * TILE_SIZE - 20, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE + 40, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, LOW_WALL_HEIGHT - 30, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE + 20, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }
        }

        if (isTileBlocked(x + room->x, y + room->y) == HIGH_WALL_TILE) {
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x34 + DARKEN_VERT, 0x20 + DARKEN_VERT, 0x0f, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x34 + DARKEN_VERT, 0x20 + DARKEN_VERT, 0x0f, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x34 + DARKEN_VERT, 0x20 + DARKEN_VERT, 0x0f, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x34 + DARKEN_VERT, 0x20 + DARKEN_VERT, 0x0f, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          continue;
        }

        if ((isTileBlocked(x + room->x, y + room->y) == FLOOR_TILE) && isTileBlocked(x + room->x, y + room->y - 1) == HIGH_WALL_TILE) {
          if (room->type == StartingRoom) {
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 50, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 2 * ROOM_VERT_DATA_SCALE + 50, 0, 0, 0, 0x20, 0x09, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE     - 50, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 2 * ROOM_VERT_DATA_SCALE + 50, 0, 0, 0, 0x20, 0x09, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE     - 50, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 0 * ROOM_VERT_DATA_SCALE - 20, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 50, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 0 * ROOM_VERT_DATA_SCALE - 20, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 3 * ROOM_VERT_DATA_SCALE + 50, 0, 0, 0, 0x40, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 3 * ROOM_VERT_DATA_SCALE + 50, 0, 0, 0, 0x40, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 3 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x20, 0x10, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 3 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x20, 0x10, 0x00, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x34, 0x20, 0x0f, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x34, 0x20, 0x0f, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x21, 0x12, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x21, 0x12, 0x00, 0xff };
          }

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }
        }

        if ((isTileBlocked(x + room->x, y + room->y) == FLOOR_TILE) && isTileBlocked(x + room->x - 1, y + room->y) == HIGH_WALL_TILE) {
          if (room->type == StartingRoom) {
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 50, 2 * ROOM_VERT_DATA_SCALE + 50, 0, 0, 0, 0x20, 0x09, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE     + 50, 2 * ROOM_VERT_DATA_SCALE + 50, 0, 0, 0, 0x20, 0x09, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE     + 50, 0 * ROOM_VERT_DATA_SCALE - 20, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 50, 0 * ROOM_VERT_DATA_SCALE - 20, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 3 * ROOM_VERT_DATA_SCALE + 50, 0, 0, 0, 0x40, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 3 * ROOM_VERT_DATA_SCALE + 50, 0, 0, 0, 0x40, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 3 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x20, 0x10, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 3 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x20, 0x10, 0x00, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x34, 0x20, 0x0f, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x34, 0x20, 0x0f, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x21, 0x12, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x21, 0x12, 0x00, 0xff };
          }

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }
        }

        if ((isTileBlocked(x + room->x, y + room->y) == FLOOR_TILE) && (isTileBlocked(x + room->x + 1, y + room->y) == HIGH_WALL_TILE)) {
          if (room->type == StartingRoom) {
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE     + 50, 2 * ROOM_VERT_DATA_SCALE + 50, 0, 0, 0, 0x20, 0x09, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 50, 2 * ROOM_VERT_DATA_SCALE + 50, 0, 0, 0, 0x20, 0x09, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 50, 0 * ROOM_VERT_DATA_SCALE - 20, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE     + 50, 0 * ROOM_VERT_DATA_SCALE - 20, 0, 0, 0, 0x2E, 0x1C, 0x00, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 3 * ROOM_VERT_DATA_SCALE + 50, 0, 0, 0, 0x40, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 3 * ROOM_VERT_DATA_SCALE + 50, 0, 0, 0, 0x40, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 3 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x20, 0x10, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 3 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x20, 0x10, 0x00, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x34, 0x20, 0x0f, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x34, 0x20, 0x0f, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x21, 0x12, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x21, 0x12, 0x00, 0xff };    

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }      
          }
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
        } else if (room->type == StartingRoom) {
          if ((y >= (room->height - 2)) && ((x == ((room -> width / 2) + 0)) || (x == ((room -> width / 2) - 1)))) {
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,         (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_B_R - 0, ENTRANCE_FLOOR_B_G - 0, ENTRANCE_FLOOR_B_B - 0, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_B_R + DARKEN_VERT, ENTRANCE_FLOOR_B_G + DARKEN_VERT, ENTRANCE_FLOOR_B_B + DARKEN_VERT, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_B_R - 0, ENTRANCE_FLOOR_B_G - 0, ENTRANCE_FLOOR_B_B - 0, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_B_R + DARKEN_VERT, ENTRANCE_FLOOR_B_G + DARKEN_VERT, ENTRANCE_FLOOR_B_B + DARKEN_VERT, 0xff };
          } else if ((y >= (room->height - 3)) && ((x == ((room -> width / 2) + 0)) || (x == ((room -> width / 2) - 1)) || (x == ((room -> width / 2) - 2)) || (x == ((room -> width / 2) + 1)))) {
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,         (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_B_R - 0, ENTRANCE_FLOOR_B_G - 0, ENTRANCE_FLOOR_B_B - 0, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_B_R - DARKEN_VERT, ENTRANCE_FLOOR_B_G - DARKEN_VERT, ENTRANCE_FLOOR_B_B + DARKEN_VERT, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_B_R - 0, ENTRANCE_FLOOR_B_G - 0, ENTRANCE_FLOOR_B_B - 0, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_B_R - DARKEN_VERT, ENTRANCE_FLOOR_B_G - DARKEN_VERT, ENTRANCE_FLOOR_B_B + DARKEN_VERT, 0xff };
          } else {
            const int tileVal = ((x % 2) + (y % 2));
            if (tileVal == 0) {
              (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,         (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_A_R, ENTRANCE_FLOOR_A_G, ENTRANCE_FLOOR_A_B, 0xff };
              (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_A_R, ENTRANCE_FLOOR_A_G, ENTRANCE_FLOOR_A_B, 0xff };
              (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_B_R, ENTRANCE_FLOOR_B_G, ENTRANCE_FLOOR_B_B, 0xff };
              (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_A_R, ENTRANCE_FLOOR_A_G, ENTRANCE_FLOOR_A_B, 0xff };
            } else if (tileVal == 1) {
              (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,         (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_A_R, ENTRANCE_FLOOR_A_G, ENTRANCE_FLOOR_A_B, 0xff };
              (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_B_R, ENTRANCE_FLOOR_B_G, ENTRANCE_FLOOR_B_B, 0xff };
              (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_A_R, ENTRANCE_FLOOR_A_G, ENTRANCE_FLOOR_A_B, 0xff };
              (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_B_R, ENTRANCE_FLOOR_B_G, ENTRANCE_FLOOR_B_B, 0xff };
            } else if (tileVal == 2) {
              (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,         (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_B_R, ENTRANCE_FLOOR_B_G, ENTRANCE_FLOOR_B_B, 0xff };
              (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_A_R, ENTRANCE_FLOOR_A_G, ENTRANCE_FLOOR_A_B, 0xff };
              (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_A_R, ENTRANCE_FLOOR_A_G, ENTRANCE_FLOOR_A_B, 0xff };
              (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, ENTRANCE_FLOOR_A_R, ENTRANCE_FLOOR_A_G, ENTRANCE_FLOOR_A_B, 0xff };
            }
          }
        } else if (room->type == BossARoom) {
          // TODO: move this to the boss' floor layout later
          u8 tone1 = FOYER_HALL_FLOOR_A + (0x30);
          u8 tone2 = FOYER_HALL_FLOOR_A + (0x30);
          u8 tone3 = FOYER_HALL_FLOOR_A + (0x30);
          u8 tone4 = FOYER_HALL_FLOOR_A + (0x30);
          const u8 right = ((x + y + 1) % 2 == 0);
          const u8 left = ((x + y) % 2 == 0);
          const u8 top = ((y + 1) % 2 == 0);
          const u8 bottom = ((y) % 2 == 0);
          tone2 += right ? 0 : -DARKEN_VERT;
          tone3 += right ? 0 : -DARKEN_VERT;
          tone1 += left ? 0 : -DARKEN_VERT;
          tone4 += left ? 0 : -DARKEN_VERT;
          tone3 += top ? 0 : -DARKEN_VERT;
          tone4 += top ? 0 : -DARKEN_VERT;
          tone1 += bottom ? 0 : -DARKEN_VERT;
          tone2 += bottom ? 0 : -DARKEN_VERT;

          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,         (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, tone1, tone1, tone1, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, tone2, tone2, tone2 - 0x10, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, tone3, tone3, tone3 - 0x10, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE,     (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, tone4, tone4, tone4 - 0x10, 0xff };
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

      if (room->type == HallwayRoom) {
        (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, HALLWAY_WALLS_R - ((x % 2 != 0) ? (DARKEN_VERT * 2) : 0), HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, HALLWAY_WALLS_R - ((x % 2 == 0) ? (DARKEN_VERT * 2) : 0), HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x10, 0x10, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x10, 0x10, 0xff };
      } else if (room->type == BossARoom) {
        (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + (x % 2 == 0 ? 40 : -20),  2 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x20, 0x1C, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + (x % 2 != 0 ? 40 : -20),  2 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x20, 0x1C, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE,      -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x04, 0x01, 0x00, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE,      -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x04, 0x01, 0x00, 0xff };

        if ((vertexList - lastBuffer) >= 64u) {
          gSPVertex(commandList++, lastBuffer, 64, 0);

          for (j = 0; j < 64; j += 4) {
            gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
          }

          lastBuffer = vertexList;
        }

        (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE + (x % 2 == 0 ? (guRandom() % 100 + 40) : 0), 0, 0, 0, 0x30, 0x10, 0x10, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE + (x % 2 != 0 ? (guRandom() % 100 + 40) : 0), 0, 0, 0, 0x30, 0x10, 0x10, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + (x % 2 != 0 ? 40 : -20),      2 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x20, 0x1C, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + (x % 2 == 0 ? 40 : -20),      2 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x20, 0x1C, 0xff };
      } else if (room->type == StartingRoom) {} else {
        (*(vertexList++)) = (Vtx){ (room->x + x)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };
        (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };
      }

      if ((vertexList - lastBuffer) >= 64u) {
        gSPVertex(commandList++, lastBuffer, 64, 0);

        for (j = 0; j < 64; j += 4) {
          gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
        }

        lastBuffer = vertexList;
      }

      if (room->type == HallwayRoom) {
        if (x % 4 == 0) {
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 45, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x80, 0x80, 0x7F, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x80, 0x80, 0x7F, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 55, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2D, 0x2D, 0x2D, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 45, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2D, 0x2D, 0x2D, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x80, 0x80, 0x7F, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 45, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x80, 0x80, 0x7F, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 45, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2D, 0x2D, 0x2D, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 55, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2D, 0x2D, 0x2D, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }
        }

        if (x % 8 == 6) {
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 119, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5,  180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5,  180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 55, 180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 100, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 55,  180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5,  180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 119, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 100, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 50, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 18,  180, 0, 0, 0, 0xff, 0xff, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5,  210, 0, 0, 0, 0xff, 0x7f, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 50, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 18, 180, 0, 0, 0, 0xff, 0xff, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 45, 150, 0, 0, 0, 0xff, 0x00, 0x01, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 141, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5,  252, 0, 0, 0, HALLWAY_WALLS_R, HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5,  412, 0, 0, 0, HALLWAY_WALLS_R, HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 141, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 252, 0, 0, 0, HALLWAY_WALLS_R, HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, 130, 0, 0, 0, HALLWAY_WALLS_R + DARKEN_VERT, HALLWAY_WALLS_G + DARKEN_VERT, HALLWAY_WALLS_B + DARKEN_VERT, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 83,  -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_A - DARKEN_VERT, FOYER_HALL_FLOOR_A - DARKEN_VERT, FOYER_HALL_FLOOR_A - DARKEN_VERT, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 181, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE,  -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_B - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 181, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 181, (room->y + room->height) * ROOM_VERT_DATA_SCALE * TILE_SIZE,  -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, (room->y + room->height) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 83,  -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_A - DARKEN_VERT, FOYER_HALL_FLOOR_A - DARKEN_VERT, FOYER_HALL_FLOOR_A - DARKEN_VERT, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 181, (room->y + room->height) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + x) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, (room->y + room->height) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_B - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT, 0xff };

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

    // Fill in the side wall tiles
    for (y = 0; y < rooms[i].height; y++) {
      if (isTileBlocked(room->x - 1, room->y + y)) {
        if (room->type == HallwayRoom) {
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, HALLWAY_WALLS_R - ((x % 2 != 0) ? (DARKEN_VERT * 2) : 0), HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y)     * ROOM_VERT_DATA_SCALE * TILE_SIZE,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, HALLWAY_WALLS_R - ((x % 2 == 0) ? (DARKEN_VERT * 2) : 0), HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x10, 0x10, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x10, 0x10, 0xff };
        } else if (room->type == BossARoom) {
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + (y % 2 != 0 ? 40 : -20), (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,  2 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x20, 0x1C, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + (y % 2 == 0 ? 40 : -20), (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE,  2 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x20, 0x1C, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,      -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x04, 0x01, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE,      -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x04, 0x01, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE + (y % 2 == 0 ? (guRandom() % 100 + 40) : 0), 0, 0, 0, 0x30, 0x10, 0x10, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y +1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE + (y % 2 != 0 ? (guRandom() % 100 + 40) : 0), 0, 0, 0, 0x30, 0x10, 0x10, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + (y % 2 == 0 ? 40 : -20), (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE,      2 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x20, 0x1C, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + (y % 2 != 0 ? 40 : -20), (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,      2 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x20, 0x1C, 0xff };
        } else if (room->type == StartingRoom) {} else {
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };
        }

        if ((vertexList - lastBuffer) >= 64u) {
          gSPVertex(commandList++, lastBuffer, 64, 0);

          for (j = 0; j < 64; j += 4) {
            gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
          }

          lastBuffer = vertexList;
        }

        if (room->type == HallwayRoom) {
          if (y % 4 == 0) {
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x80, 0x80, 0x7F, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 45,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x80, 0x80, 0x7F, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 45, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2D, 0x2D, 0x2D, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 55, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2D, 0x2D, 0x2D, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 45,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x80, 0x80, 0x7F, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x80, 0x80, 0x7F, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 55, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2D, 0x2D, 0x2D, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 45, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2D, 0x2D, 0x2D, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }
          }

          if (y % 8 == 6) {
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 119,  180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 55, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, 180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, 100, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 55, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 119, 180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, 100, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  210, 0, 0, 0, 0xff, 0x7f, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 18, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 50,  180, 0, 0, 0, 0xff, 0xff, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 45, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, 150, 0, 0, 0, 0xff, 0x00, 0x01, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 18, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 50, 180, 0, 0, 0, 0xff, 0xff, 0x00, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  412, 0, 0, 0, HALLWAY_WALLS_R, HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 141,  252, 0, 0, 0, HALLWAY_WALLS_R, HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, 130, 0, 0, 0, HALLWAY_WALLS_R + DARKEN_VERT, HALLWAY_WALLS_G + DARKEN_VERT, HALLWAY_WALLS_B + DARKEN_VERT, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 141, 252, 0, 0, 0, HALLWAY_WALLS_R, HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 181,  -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 83, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_A - DARKEN_VERT, FOYER_HALL_FLOOR_A - DARKEN_VERT, FOYER_HALL_FLOOR_A - DARKEN_VERT, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 181, -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_B + DARKEN_VERT, FOYER_HALL_FLOOR_B, FOYER_HALL_FLOOR_B, 0xff };

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

      if (isTileBlocked(room->x + room->width, room->y + y)) {
        if (room->type == HallwayRoom) {
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y)         * ROOM_VERT_DATA_SCALE * TILE_SIZE,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, HALLWAY_WALLS_R - ((x % 2 != 0) ? (DARKEN_VERT * 2) : 0), HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, HALLWAY_WALLS_R - ((x % 2 == 0) ? (DARKEN_VERT * 2) : 0), HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x10, 0x10, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y)         * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x10, 0x10, 0xff };
        } else if (room->type == BossARoom) {
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE + (y % 2 == 0 ? 40 : -20), (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE,  2 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x20, 0x1C, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE + (y % 2 != 0 ? 40 : -20), (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,  2 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x20, 0x1C, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y) * ROOM_VERT_DATA_SCALE * TILE_SIZE,      -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x04, 0x01, 0x00, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,      -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x04, 0x01, 0x00, 0xff };

          if ((vertexList - lastBuffer) >= 64u) {
            gSPVertex(commandList++, lastBuffer, 64, 0);

            for (j = 0; j < 64; j += 4) {
              gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
            }

            lastBuffer = vertexList;
          }

          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE + (y % 2 == 0 ? (guRandom() % 100 + 40) : 0), 0, 0, 0, 0x30, 0x10, 0x10, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y +1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, 5 * ROOM_VERT_DATA_SCALE + (y % 2 != 0 ? (guRandom() % 100 + 40) : 0), 0, 0, 0, 0x30, 0x10, 0x10, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE + (y % 2 != 0 ? 40 : -20), (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,      2 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x20, 0x1C, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE + (y % 2 == 0 ? 40 : -20), (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE,      2 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x30, 0x20, 0x1C, 0xff };
        } else if (room->type == StartingRoom) {} else {
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y)     * ROOM_VERT_DATA_SCALE * TILE_SIZE,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y + 1) * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };
          (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y)     * ROOM_VERT_DATA_SCALE * TILE_SIZE, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, WALL_COLOR_R - DARKEN_VERT, WALL_COLOR_G - DARKEN_VERT, WALL_COLOR_B - DARKEN_VERT, 0xff };
        }

        if ((vertexList - lastBuffer) >= 64u) {
          gSPVertex(commandList++, lastBuffer, 64, 0);

          for (j = 0; j < 64; j += 4) {
            gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
          }

          lastBuffer = vertexList;
        }

        if (room->type == HallwayRoom) {
          if (y % 4 == 0) {
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 45,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x80, 0x80, 0x7F, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x80, 0x80, 0x7F, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 55, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2D, 0x2D, 0x2D, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 45, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2D, 0x2D, 0x2D, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x80, 0x80, 0x7F, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 45,  5 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x80, 0x80, 0x7F, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 45, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2D, 0x2D, 0x2D, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 55, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, -1 * ROOM_VERT_DATA_SCALE, 0, 0, 0, 0x2D, 0x2D, 0x2D, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }
          }

          if (y % 8 == 6) {
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 119,  180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 55, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0, 180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, 100, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 55, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 119, 180, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, 100, 0, 0, 0, 0x00, 0x00, 0x00, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 18, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 50,  180, 0, 0, 0, 0xff, 0xff, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  210, 0, 0, 0, 0xff, 0x7f, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 18, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 50, 180, 0, 0, 0, 0xff, 0xff, 0x00, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 45, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, 150, 0, 0, 0, 0xff, 0x00, 0x01, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }

            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 141,  252, 0, 0, 0, HALLWAY_WALLS_R, HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  412, 0, 0, 0, HALLWAY_WALLS_R, HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 141, 252, 0, 0, 0, HALLWAY_WALLS_R, HALLWAY_WALLS_G, HALLWAY_WALLS_B, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 5, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, 130, 0, 0, 0, HALLWAY_WALLS_R + DARKEN_VERT, HALLWAY_WALLS_G + DARKEN_VERT, HALLWAY_WALLS_B + DARKEN_VERT, 0xff };

            if ((vertexList - lastBuffer) >= 64u) {
              gSPVertex(commandList++, lastBuffer, 64, 0);

              for (j = 0; j < 64; j += 4) {
                gSP2Triangles(commandList++, j + 0, j + 1, j + 2, 0, j + 0, j + 2, j + 3, 0);
              }

              lastBuffer = vertexList;
            }


            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 83, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 0,  -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_A - DARKEN_VERT, FOYER_HALL_FLOOR_A - DARKEN_VERT, FOYER_HALL_FLOOR_A - DARKEN_VERT, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 181,  -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, 0xff };
            
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE + 181, -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, FOYER_HALL_FLOOR_B - DARKEN_VERT - DARKEN_VERT, 0xff };
            (*(vertexList++)) = (Vtx){ (room->x + room->width) * ROOM_VERT_DATA_SCALE * TILE_SIZE, (room->y + y) * ROOM_VERT_DATA_SCALE * TILE_SIZE - 0, -1 * ROOM_VERT_DATA_SCALE + 5, 0, 0, 0, FOYER_HALL_FLOOR_B + DARKEN_VERT, FOYER_HALL_FLOOR_B, FOYER_HALL_FLOOR_B, 0xff };

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
    rooms[i].lockIndex = -1;
    rooms[i].numberOfEnemies = 0;
  }

  //TODO: Turn the floor numbers from magic numbers into #define constants
  if (floorNumber == 0) {
  	numberOfGeneratedRooms = generateFloorInStyleA(rooms);
    createFoyerDisplayData(rooms, numberOfGeneratedRooms);
  } else if ((floorNumber == 3) || (floorNumber == 6) || (floorNumber == 9)) {
    numberOfGeneratedRooms = generateBossStyleFloor(rooms, floorNumber);
    createGenericDisplayData(rooms, numberOfGeneratedRooms);
  } else {
  	numberOfGeneratedRooms = generateBasementStyleFloor(rooms, floorNumber);
    createGenericDisplayData(rooms, numberOfGeneratedRooms);
  }

  return numberOfGeneratedRooms;
}

void initEnemiesForMap(GeneratedRoom* rooms) {
  int i;

  for (i = 0; i < MAX_NUMBER_OF_ROOMS_PER_FLOOR; i++) {
    if (rooms[i].type == EnemyRoom) {
      if (hasRoomBeenCleared(currentFloor, i)) {
        continue;
      }

      rooms[i].enemies[rooms[i].numberOfEnemies++] = generateAimEmitterEntity((rooms[i].x + (rooms[i].width / 4)) * TILE_SIZE, (rooms[i].y  + (rooms[i].height / 4)) * TILE_SIZE);
      rooms[i].enemies[rooms[i].numberOfEnemies++] = generateAimEmitterEntity((rooms[i].x + (rooms[i].width / 4 * 3)) * TILE_SIZE, (rooms[i].y  + (rooms[i].height / 4)) * TILE_SIZE);
      rooms[i].enemies[rooms[i].numberOfEnemies++] = generateAimEmitterEntity((rooms[i].x + (rooms[i].width / 4 * 3)) * TILE_SIZE, (rooms[i].y  + (rooms[i].height / 4 * 3)) * TILE_SIZE);
      rooms[i].enemies[rooms[i].numberOfEnemies++] = generateAimEmitterEntity((rooms[i].x + (rooms[i].width / 4)) * TILE_SIZE, (rooms[i].y  + (rooms[i].height / 4 * 3)) * TILE_SIZE);
    }

    if (rooms[i].type == BossARoom) {
      generateBossA((rooms[i].x + (rooms[i].width / 2)) * TILE_SIZE, (rooms[i].y  + (rooms[i].height / 2)) * TILE_SIZE);
    }
  }
}

int isTileBlocked(int x, int y) {
	return IS_TILE_BLOCKED(x, y);
}



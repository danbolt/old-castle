

#include "RoomData.h"

#include "EntityData.h"
#include "game_math.h"
#include "graphic.h"

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

#define VERTS_PER_TILE 8
static Vtx map_geom[MAP_SIZE * MAP_SIZE * VERTS_PER_TILE];


void fillInRooms(GeneratedRoom* rooms, int roomCount) {
	int i;
	int x;
	int y;

	for (i = 0; i < roomCount; i++) {
		GeneratedRoom* room = &(rooms[i]);

		for (x = room->x; x < (room->x + room->width); x++) {
			for (y = room->y; y < (room->y + room->height); y++) {
				MapInfo[(y * MAP_SIZE) + x] = FLOOR_TILE;
			}
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

	int mainCorridorLength = (guRandom() % 25) + 25;

	rooms[0].x = (MAP_SIZE / 2) - 4;
	rooms[0].y = MAP_SIZE - 10;
	rooms[0].width = 8;
	rooms[0].height = 8;
	rooms[0].type = StartingRoom;

	rooms[1].x = (MAP_SIZE / 2) - 2;
	rooms[1].y = MAP_SIZE - 10 - (u8)mainCorridorLength;
	rooms[1].width = 4;
	rooms[1].height = (u8)mainCorridorLength;
	rooms[1].type = HallwayRoom;

	rooms[2].x = (MAP_SIZE / 2) - 12;
	rooms[2].y = MAP_SIZE - 10 - (u8)mainCorridorLength - 33;
	rooms[2].width = 24;
	rooms[2].height = 33;
	rooms[2].type = EnemyRoom;

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
	rooms[4].type = EnemyRoom;

	// Side room B
	rooms[5].x = (MAP_SIZE / 4) + (MAP_SIZE / 2);
	rooms[5].y = MAP_SIZE - 10 - (mainCorridorLength / 2) - 7;
	rooms[5].width = 14;
	rooms[5].height = 14;
	rooms[5].type = EnemyRoom;

	fillInRooms(rooms, 6);

	fillInHighWalls();
}

void initMap(GeneratedRoom* rooms) {
  int i;

  // "Fill in" impassible tiles everywhere to start
  for (i = 0; i < (MAP_SIZE * MAP_SIZE); i++) {
  	MapInfo[i] = EMPTY_HIGH_WALL_TILE;
  }

  generateFloorInStyleA(rooms);
}

void initEnemiesForMap(GeneratedRoom* rooms) {
  int i;

  for (i = 0; i < MAX_NUMBER_OF_ROOMS_PER_FLOOR; i++) {
    if (rooms[i].type == EnemyRoom) {
      int emittersToPlace = (guRandom() % 7) + 1;
      int iEmit;
      for (iEmit = 0; iEmit < emittersToPlace; iEmit++) {
        int xEnemyPos = 4 + (guRandom() % (rooms[i].width - 8));
        int yEnemyPos = 4 + (guRandom() % (rooms[i].height - 8));
        generateAimEmitterEntity((rooms[i].x + xEnemyPos) * TILE_SIZE, (rooms[i].y + yEnemyPos) * TILE_SIZE);
      }
      

      MapInfo[((rooms[i].y + 4) * MAP_SIZE) + (rooms[i].x + 4)] = LOW_WALL_TILE;
      MapInfo[((rooms[i].y + 4) * MAP_SIZE) + (rooms[i].x + rooms[i].width - 5)] = LOW_WALL_TILE;
      MapInfo[((rooms[i].y + rooms[i].height - 5) * MAP_SIZE) + (rooms[i].x + 4)] = LOW_WALL_TILE;
      MapInfo[((rooms[i].y + rooms[i].height - 5) * MAP_SIZE) + (rooms[i].x + rooms[i].width - 5)] = LOW_WALL_TILE;
    }
  }
}

void updateMapFromInfo() {
  int i;

  for (i = 0; i < MAP_SIZE * MAP_SIZE; i++) {
    short x = i % MAP_SIZE;
    short y = i / MAP_SIZE;

    u8 tileType = IS_TILE_BLOCKED(x, y);

    if (tileType == FLOOR_TILE) {
      int var = guRandom() % FLOOR_COLOR_2_VAR;
      int var2 = guRandom() % FLOOR_COLOR_1_VAR;

      map_geom[(i * VERTS_PER_TILE) + 0].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 0].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 0].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 0].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 0].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 0].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 0].v.cn[0] = FLOOR_COLOR_1 + var2;
      map_geom[(i * VERTS_PER_TILE) + 0].v.cn[1] = FLOOR_COLOR_1 + var2;
      map_geom[(i * VERTS_PER_TILE) + 0].v.cn[2] = FLOOR_COLOR_1 + var2;
      map_geom[(i * VERTS_PER_TILE) + 0].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 1].v.ob[0] = (x * TILE_SIZE) + 1;
      map_geom[(i * VERTS_PER_TILE) + 1].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 1].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 1].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 1].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 1].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 1].v.cn[0] = FLOOR_COLOR_2 + var;
      map_geom[(i * VERTS_PER_TILE) + 1].v.cn[1] = FLOOR_COLOR_2 + var;
      map_geom[(i * VERTS_PER_TILE) + 1].v.cn[2] = FLOOR_COLOR_1 + var;
      map_geom[(i * VERTS_PER_TILE) + 1].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 2].v.ob[0] = (x * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 2].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 2].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 2].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 2].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 2].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 2].v.cn[0] = FLOOR_COLOR_1 + var2;
      map_geom[(i * VERTS_PER_TILE) + 2].v.cn[1] = FLOOR_COLOR_1 + var2;
      map_geom[(i * VERTS_PER_TILE) + 2].v.cn[2] = FLOOR_COLOR_1 + var2;
      map_geom[(i * VERTS_PER_TILE) + 2].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 3].v.ob[0] = (x * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 3].v.ob[1] = (y * TILE_SIZE) + 1;
      map_geom[(i * VERTS_PER_TILE) + 3].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 3].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 3].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 3].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 3].v.cn[0] = FLOOR_COLOR_2 + var;
      map_geom[(i * VERTS_PER_TILE) + 3].v.cn[1] = FLOOR_COLOR_2 + var;
      map_geom[(i * VERTS_PER_TILE) + 3].v.cn[2] = FLOOR_COLOR_1 + var;
      map_geom[(i * VERTS_PER_TILE) + 3].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 4].v.ob[0] = (x * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 4].v.ob[1] = (y * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 4].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 4].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 4].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 4].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[0] = FLOOR_COLOR_1 + var2;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[1] = FLOOR_COLOR_1 + var2;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[2] = FLOOR_COLOR_1 + var2;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 5].v.ob[0] = (x * TILE_SIZE) + 1;
      map_geom[(i * VERTS_PER_TILE) + 5].v.ob[1] = (y * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 5].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 5].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 5].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 5].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[0] = FLOOR_COLOR_2 + var;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[1] = FLOOR_COLOR_2 + var;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[2] = FLOOR_COLOR_1 + var;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 6].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 6].v.ob[1] = (y * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 6].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 6].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 6].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 6].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[0] = FLOOR_COLOR_1 + var2;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[1] = FLOOR_COLOR_1 + var2;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[2] = FLOOR_COLOR_1 + var2;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 7].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 7].v.ob[1] = (y * TILE_SIZE) + 1;
      map_geom[(i * VERTS_PER_TILE) + 7].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 7].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 7].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 7].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[0] = FLOOR_COLOR_2 + var;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[1] = FLOOR_COLOR_2 + var;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[2] = FLOOR_COLOR_1 + var;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[3] = 0xff;

      if ((y > 0) && IS_TILE_BLOCKED(x, (y - 1))) {
        map_geom[(i * VERTS_PER_TILE) + 0].v.cn[0] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 0].v.cn[1] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 0].v.cn[2] -= DARKEN_VERT;

        map_geom[(i * VERTS_PER_TILE) + 1].v.cn[0] -= DARKEN_VERT * 2;
        map_geom[(i * VERTS_PER_TILE) + 1].v.cn[1] -= DARKEN_VERT * 2;
        map_geom[(i * VERTS_PER_TILE) + 1].v.cn[2] -= DARKEN_VERT * 2;

        map_geom[(i * VERTS_PER_TILE) + 2].v.cn[0] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2].v.cn[1] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2].v.cn[2] -= DARKEN_VERT;
      }
      if ((y < (MAP_SIZE - 1)) && IS_TILE_BLOCKED(x, (y + 1))) {
        map_geom[(i * VERTS_PER_TILE) + 6].v.cn[0] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 6].v.cn[1] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 6].v.cn[2] -= DARKEN_VERT;

        map_geom[(i * VERTS_PER_TILE) + 5].v.cn[0] -= DARKEN_VERT * 2;
        map_geom[(i * VERTS_PER_TILE) + 5].v.cn[1] -= DARKEN_VERT * 2;
        map_geom[(i * VERTS_PER_TILE) + 5].v.cn[2] -= DARKEN_VERT * 2;

        map_geom[(i * VERTS_PER_TILE) + 4].v.cn[0] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 4].v.cn[1] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 4].v.cn[2] -= DARKEN_VERT;
      }
      if ((x > 0) && IS_TILE_BLOCKED((x - 1), y)) {
        map_geom[(i * VERTS_PER_TILE) + 0].v.cn[0] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 0].v.cn[1] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 0].v.cn[2] -= DARKEN_VERT;

        map_geom[(i * VERTS_PER_TILE) + 7].v.cn[0] -= DARKEN_VERT * 2;
        map_geom[(i * VERTS_PER_TILE) + 7].v.cn[1] -= DARKEN_VERT * 2;
        map_geom[(i * VERTS_PER_TILE) + 7].v.cn[2] -= DARKEN_VERT * 2;

        map_geom[(i * VERTS_PER_TILE) + 6].v.cn[0] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 6].v.cn[1] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 6].v.cn[2] -= DARKEN_VERT;
      }
      if ((x < (MAP_SIZE - 1)) && IS_TILE_BLOCKED((x + 1), y)) {
        map_geom[(i * VERTS_PER_TILE) + 2].v.cn[0] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2].v.cn[1] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2].v.cn[2] -= DARKEN_VERT;

        map_geom[(i * VERTS_PER_TILE) + 3].v.cn[0] -= DARKEN_VERT * 2;
        map_geom[(i * VERTS_PER_TILE) + 3].v.cn[1] -= DARKEN_VERT * 2;
        map_geom[(i * VERTS_PER_TILE) + 3].v.cn[2] -= DARKEN_VERT * 2;

        map_geom[(i * VERTS_PER_TILE) + 4].v.cn[0] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 4].v.cn[1] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 4].v.cn[2] -= DARKEN_VERT;
      }

      if ((x > 0) && (y > 0) && IS_TILE_BLOCKED((x - 1), (y - 1))) {
        map_geom[(i * VERTS_PER_TILE) + 0].v.cn[0] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 0].v.cn[1] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 0].v.cn[2] -= DARKEN_VERT;
      }
      if ((x > 0) && (y < (MAP_SIZE - 1)) && IS_TILE_BLOCKED((x - 1), (y + 1))) {
        map_geom[(i * VERTS_PER_TILE) + 6].v.cn[0] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 6].v.cn[1] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 6].v.cn[2] -= DARKEN_VERT;
      }
      if ((x < (MAP_SIZE - 1)) && (y < (MAP_SIZE - 1)) && IS_TILE_BLOCKED((x + 1), (y + 1))) {
        map_geom[(i * VERTS_PER_TILE) + 4].v.cn[0] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 4].v.cn[1] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 4].v.cn[2] -= DARKEN_VERT;
      }
      if ((x < (MAP_SIZE - 1)) && (y > 0) && IS_TILE_BLOCKED((x + 1), (y - 1))) {
        map_geom[(i * VERTS_PER_TILE) + 2].v.cn[0] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2].v.cn[1] -= DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2].v.cn[2] -= DARKEN_VERT;
      }

    } else if (tileType == LOW_WALL_TILE) {
      map_geom[(i * VERTS_PER_TILE) + 0].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 0].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 0].v.ob[2] = 1;
      map_geom[(i * VERTS_PER_TILE) + 0].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 0].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 0].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 0].v.cn[0] = 0x5f;
      map_geom[(i * VERTS_PER_TILE) + 0].v.cn[1] = 0x54;
      map_geom[(i * VERTS_PER_TILE) + 0].v.cn[2] = 0x32;
      map_geom[(i * VERTS_PER_TILE) + 0].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 1].v.ob[0] = (x * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 1].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 1].v.ob[2] = 1;
      map_geom[(i * VERTS_PER_TILE) + 1].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 1].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 1].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 1].v.cn[0] = 0x5f;
      map_geom[(i * VERTS_PER_TILE) + 1].v.cn[1] = 0x54;
      map_geom[(i * VERTS_PER_TILE) + 1].v.cn[2] = 0x32;
      map_geom[(i * VERTS_PER_TILE) + 1].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 2].v.ob[0] = (x * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 2].v.ob[1] = (y * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 2].v.ob[2] = 1;
      map_geom[(i * VERTS_PER_TILE) + 2].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 2].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 2].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 2].v.cn[0] = 0x5f;
      map_geom[(i * VERTS_PER_TILE) + 2].v.cn[1] = 0x54;
      map_geom[(i * VERTS_PER_TILE) + 2].v.cn[2] = 0x32;
      map_geom[(i * VERTS_PER_TILE) + 2].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 3].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 3].v.ob[1] = (y * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 3].v.ob[2] = 1;
      map_geom[(i * VERTS_PER_TILE) + 3].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 3].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 3].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 3].v.cn[0] = 0x5f;
      map_geom[(i * VERTS_PER_TILE) + 3].v.cn[1] = 0x54;
      map_geom[(i * VERTS_PER_TILE) + 3].v.cn[2] = 0x32;
      map_geom[(i * VERTS_PER_TILE) + 3].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 4].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 4].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 4].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 4].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 4].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 4].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[0] = 0x5f - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[1] = 0x54 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[2] = 0x32 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[3] = 0xff - DARKEN_VERT;

      map_geom[(i * VERTS_PER_TILE) + 5].v.ob[0] = (x * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 5].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 5].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 5].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 5].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 5].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[0] = 0x5f - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[1] = 0x54 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[2] = 0x32 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[3] = 0xff - DARKEN_VERT;

      map_geom[(i * VERTS_PER_TILE) + 6].v.ob[0] = (x * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 6].v.ob[1] = (y * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 6].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 6].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 6].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 6].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[0] = 0x5f - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[1] = 0x54 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[2] = 0x32 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[3] = 0xff - DARKEN_VERT;

      map_geom[(i * VERTS_PER_TILE) + 7].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 7].v.ob[1] = (y * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 7].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 7].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 7].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 7].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[0] = 0x5f - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[1] = 0x54 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[2] = 0x32 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[3] = 0xff - DARKEN_VERT;
    } else if (tileType == HIGH_WALL_TILE) {
      map_geom[(i * VERTS_PER_TILE) + 0].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 0].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 0].v.ob[2] = 3;
      map_geom[(i * VERTS_PER_TILE) + 0].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 0].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 0].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 0].v.cn[0] = 0x5f;
      map_geom[(i * VERTS_PER_TILE) + 0].v.cn[1] = 0x54;
      map_geom[(i * VERTS_PER_TILE) + 0].v.cn[2] = 0x32;
      map_geom[(i * VERTS_PER_TILE) + 0].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 1].v.ob[0] = (x * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 1].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 1].v.ob[2] = 3;
      map_geom[(i * VERTS_PER_TILE) + 1].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 1].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 1].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 1].v.cn[0] = 0x5f;
      map_geom[(i * VERTS_PER_TILE) + 1].v.cn[1] = 0x54;
      map_geom[(i * VERTS_PER_TILE) + 1].v.cn[2] = 0x32;
      map_geom[(i * VERTS_PER_TILE) + 1].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 2].v.ob[0] = (x * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 2].v.ob[1] = (y * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 2].v.ob[2] = 3;
      map_geom[(i * VERTS_PER_TILE) + 2].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 2].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 2].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 2].v.cn[0] = 0x5f;
      map_geom[(i * VERTS_PER_TILE) + 2].v.cn[1] = 0x54;
      map_geom[(i * VERTS_PER_TILE) + 2].v.cn[2] = 0x32;
      map_geom[(i * VERTS_PER_TILE) + 2].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 3].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 3].v.ob[1] = (y * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 3].v.ob[2] = 3;
      map_geom[(i * VERTS_PER_TILE) + 3].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 3].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 3].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 3].v.cn[0] = 0x5f;
      map_geom[(i * VERTS_PER_TILE) + 3].v.cn[1] = 0x54;
      map_geom[(i * VERTS_PER_TILE) + 3].v.cn[2] = 0x32;
      map_geom[(i * VERTS_PER_TILE) + 3].v.cn[3] = 0xff;

      map_geom[(i * VERTS_PER_TILE) + 4].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 4].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 4].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 4].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 4].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 4].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[0] = 0x5f - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[1] = 0x54 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[2] = 0x32 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[3] = 0xff - DARKEN_VERT;

      map_geom[(i * VERTS_PER_TILE) + 5].v.ob[0] = (x * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 5].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 5].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 5].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 5].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 5].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[0] = 0x5f - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[1] = 0x54 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[2] = 0x32 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[3] = 0xff - DARKEN_VERT;

      map_geom[(i * VERTS_PER_TILE) + 6].v.ob[0] = (x * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 6].v.ob[1] = (y * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 6].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 6].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 6].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 6].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[0] = 0x5f - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[1] = 0x54 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[2] = 0x32 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[3] = 0xff - DARKEN_VERT;

      map_geom[(i * VERTS_PER_TILE) + 7].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 7].v.ob[1] = (y * TILE_SIZE) + 2;
      map_geom[(i * VERTS_PER_TILE) + 7].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 7].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 7].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 7].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[0] = 0x5f - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[1] = 0x54 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[2] = 0x32 - DARKEN_VERT;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[3] = 0xff - DARKEN_VERT;
    }
  }
}

void renderMapTiles(float camera_x, float camera_y, float camera_rotation) {
	int i;
	int j;

	for (i = MAX(0, (int)(((camera_x + cosf(camera_rotation + M_PI_2) * 4.f) / TILE_SIZE) - RENDER_DISTANCE_IN_TILES)); i < MIN(MAP_SIZE, (int)(((camera_x + cosf(camera_rotation + M_PI_2) * 4.f) / TILE_SIZE) + RENDER_DISTANCE_IN_TILES)); i++) {
		for (j = MAX(0, (int)(((camera_y + sinf(camera_rotation + M_PI_2) * 4.f) / TILE_SIZE) - RENDER_DISTANCE_IN_TILES)); j < MIN(MAP_SIZE, (int)(((camera_y + sinf(camera_rotation + M_PI_2) * 4.f) / TILE_SIZE) + RENDER_DISTANCE_IN_TILES)); j++) {
		  int type = IS_TILE_BLOCKED(i, j);

		  if (type == EMPTY_HIGH_WALL_TILE) {
		    continue;
		  }

		  gSPVertex(glistp++,&(map_geom[((j * MAP_SIZE) + i) * VERTS_PER_TILE]), 8, 0);

		  if (type == FLOOR_TILE) {
		    gSP2Triangles(glistp++,7,0,1,0,1,2,3,0);
		    gSP2Triangles(glistp++,3,4,5,0,5,6,7,0);
		    gSP2Triangles(glistp++,7,1,5,0,5,1,3,0);
		  } else if (type == LOW_WALL_TILE) {
		    gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
		    gSP2Triangles(glistp++,0,4,1,0,4,5,1,0);
		    gSP2Triangles(glistp++,3,2,7,0,2,6,7,0);
		    gSP2Triangles(glistp++,2,1,6,0,6,1,5,0);
		    gSP2Triangles(glistp++,0,3,7,0,0,7,4,0);
		  } else if (type == HIGH_WALL_TILE) {
		    gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
		    gSP2Triangles(glistp++,0,4,1,0,4,5,1,0);
		    gSP2Triangles(glistp++,3,2,7,0,2,6,7,0);
		    gSP2Triangles(glistp++,2,1,6,0,6,1,5,0);
		    gSP2Triangles(glistp++,0,3,7,0,0,7,4,0);
		  }
		}
	}
}


int isTileBlocked(int x, int y) {
	return IS_TILE_BLOCKED(x, y);
}



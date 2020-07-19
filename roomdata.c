

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

void initMap(GeneratedRoom* rooms) {
  int i;
  int j;
  u8 upIndices[MAP_SIZE / ROOM_SIZE];
  int prev = 0;

  // "Connect" the rooms 
  for (i = 0; i < NUMBER_OF_ROOMS_PER_FLOOR; i++) {

    rooms[i].type = i == 0 ? StartingRoom : EnemyRoom;

    rooms[i].east = 1;
    rooms[i].north = 1;
  } 

  // "Dig out" the rooms
  for (i = 0; i < MAP_SIZE; i++) {
    for (j = 0; j < MAP_SIZE; j++) {
      int roomX = i % ROOM_SIZE;
      int roomY = j % ROOM_SIZE;
      int roomIndex = ((i / ROOM_SIZE) + ((j / ROOM_SIZE) * (MAP_SIZE / ROOM_SIZE)));

      MapInfo[j * MAP_SIZE + i] = EMPTY_HIGH_WALL_TILE;

      // initalize room data here
      if (roomX == 0 && roomY == 0) {
        int w = 14 + (guRandom() % (ROOM_SIZE - 14));
        int h = 14 + (guRandom() % (ROOM_SIZE - 14));

        rooms[roomIndex].rawX = i;
        rooms[roomIndex].rawY = j;
        rooms[roomIndex].x = (ROOM_SIZE - w) / 2;
        rooms[roomIndex].y = (ROOM_SIZE - h) / 2;
        rooms[roomIndex].width = w;
        rooms[roomIndex].height = h;
      }

      if (rooms[roomIndex].type == NoRoom) {
        continue;
      }

      if ((rooms[roomIndex].x == roomX) && (rooms[roomIndex].y == roomY)) {
        rooms[roomIndex].rawX = i;
        rooms[roomIndex].rawY = j;
      }

      // If we're outside the room, don't worry about it
      if ((roomX < rooms[roomIndex].x) || (roomX >= (rooms[roomIndex].x + rooms[roomIndex].width)) || (roomY < rooms[roomIndex].y) || (roomY >= (rooms[roomIndex].y + rooms[roomIndex].height))) {
        continue;
      }

      if ((roomX == rooms[roomIndex].x) || (roomX == (rooms[roomIndex].x + rooms[roomIndex].width - 1)) || (roomY == rooms[roomIndex].y) || (roomY == (rooms[roomIndex].y + rooms[roomIndex].height - 1))) {
        MapInfo[j * MAP_SIZE + i] = HIGH_WALL_TILE;
      } else if ((roomX > rooms[roomIndex].x) && (roomX < (rooms[roomIndex].x + rooms[roomIndex].width - 1)) && (roomY > rooms[roomIndex].y) && (roomY < (rooms[roomIndex].y + rooms[roomIndex].height - 1))) {
        MapInfo[j * MAP_SIZE + i] = FLOOR_TILE;
      }
    }
  }

  // "Dig out" adjacent rooms with pathways in a fun manner
  for (i = 0; i < NUMBER_OF_ROOMS_PER_FLOOR; i++) {
    GeneratedRoom* room = &rooms[i];
    GeneratedRoom* east = (i % (MAP_SIZE / ROOM_SIZE) != ((MAP_SIZE / ROOM_SIZE) - 1)) ? &rooms[i + 1] : NULL;
    GeneratedRoom* north = (i / (MAP_SIZE / ROOM_SIZE) != ((MAP_SIZE / ROOM_SIZE) - 1)) ? &rooms[i + (MAP_SIZE / ROOM_SIZE)] : NULL;

    // Connect an east bridge
    if (room->east && east != NULL && (east->type != NoRoom)) {
      int i;
      int midY = room->rawY + (room->height / 2) + (guRandom() % 4);

      for (i = (room->rawX + room->width - 1); i < (east->rawX + 1); i++) {
        MapInfo[((midY + 1) * MAP_SIZE) + i] = HIGH_WALL_TILE;
        MapInfo[((midY) * MAP_SIZE) + i] = FLOOR_TILE;
        MapInfo[((midY - 1) * MAP_SIZE) + i] = FLOOR_TILE;
        MapInfo[((midY - 2) * MAP_SIZE) + i] = FLOOR_TILE;
        MapInfo[((midY - 3) * MAP_SIZE) + i] = HIGH_WALL_TILE;
      }
    }

    if (room->north && north != NULL && (north->type != NoRoom)) {
      int i;
      int midX = room->rawX + (room->width / 2) + (guRandom() % 4);

      for (i = (room->rawY + room->height - 1); i < (north->rawY + 1); i++) {
        MapInfo[(i * MAP_SIZE) + (midX + 1)] = HIGH_WALL_TILE;
        MapInfo[(i * MAP_SIZE) + (midX - 0)] = FLOOR_TILE;
        MapInfo[(i * MAP_SIZE) + (midX - 1)] = FLOOR_TILE;
        MapInfo[(i * MAP_SIZE) + (midX - 2)] = FLOOR_TILE;
        MapInfo[(i * MAP_SIZE) + (midX - 3)] = FLOOR_TILE;
        MapInfo[(i * MAP_SIZE) + (midX - 4)] = HIGH_WALL_TILE;
      }
    }
  }
}

void initEnemiesForMap(GeneratedRoom* rooms) {
  int i;

  for (i = 0; i < NUMBER_OF_ROOMS_PER_FLOOR; i++) {
    if (rooms[i].type == EnemyRoom) {
      int emittersToPlace = (guRandom() % 7) + 1;
      int iEmit;
      for (iEmit = 0; iEmit < emittersToPlace; iEmit++) {
        int xEnemyPos = 4 + (guRandom() % (rooms[i].width - 8));
        int yEnemyPos = 4 + (guRandom() % (rooms[i].height - 8));
        generateAimEmitterEntity((rooms[i].rawX + xEnemyPos) * TILE_SIZE, (rooms[i].rawY + yEnemyPos) * TILE_SIZE);
      }
      

      MapInfo[((rooms[i].rawY + 4) * MAP_SIZE) + (rooms[i].rawX + 4)] = LOW_WALL_TILE;
      MapInfo[((rooms[i].rawY + 4) * MAP_SIZE) + (rooms[i].rawX + rooms[i].width - 5)] = LOW_WALL_TILE;
      MapInfo[((rooms[i].rawY + rooms[i].height - 5) * MAP_SIZE) + (rooms[i].rawX + 4)] = LOW_WALL_TILE;
      MapInfo[((rooms[i].rawY + rooms[i].height - 5) * MAP_SIZE) + (rooms[i].rawX + rooms[i].width - 5)] = LOW_WALL_TILE;
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
      int offset = 0;

      if ((offset < 8) && (y > 0) && (((HIGH_WALL_TILE != IS_TILE_BLOCKED(x, (y - 1))) && (EMPTY_HIGH_WALL_TILE != IS_TILE_BLOCKED(x, (y - 1)))))) {
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.ob[0] = (x * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.ob[1] = (y * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.ob[2] = 3;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[0] = WALL_COLOR_R;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[1] = WALL_COLOR_G;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[2] = WALL_COLOR_B;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[3] = 0xff;

        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.ob[0] = (x * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.ob[1] = (y * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.ob[2] = -1;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[0] = WALL_COLOR_R - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[1] = WALL_COLOR_G - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[2] = WALL_COLOR_B - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[3] = 0xff;

        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.ob[0] = (x * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.ob[1] = (y * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.ob[2] = -1;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[0] = WALL_COLOR_R - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[1] = WALL_COLOR_G - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[2] = WALL_COLOR_B - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[3] = 0xff;

        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.ob[0] = (x * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.ob[1] = (y * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.ob[2] = 3;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[0] = WALL_COLOR_R;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[1] = WALL_COLOR_G;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[2] = WALL_COLOR_B;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[3] = 0xff;

        offset += 4;
      }

      if ((offset < 8) && (y < (MAP_SIZE - 1)) && (((HIGH_WALL_TILE != IS_TILE_BLOCKED(x, (y + 1))) && (EMPTY_HIGH_WALL_TILE != IS_TILE_BLOCKED(x, (y + 1)))))) {
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.ob[0] = (x * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.ob[1] = (y * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.ob[2] = 3;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[0] = WALL_COLOR_R;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[1] = WALL_COLOR_G;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[2] = WALL_COLOR_B;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[3] = 0xff;

        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.ob[0] = (x * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.ob[1] = (y * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.ob[2] = -1;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[0] = WALL_COLOR_R - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[1] = WALL_COLOR_G - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[2] = WALL_COLOR_B - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[3] = 0xff;

        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.ob[0] = (x * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.ob[1] = (y * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.ob[2] = -1;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[0] = WALL_COLOR_R - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[1] = WALL_COLOR_G - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[2] = WALL_COLOR_B - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[3] = 0xff;

        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.ob[0] = (x * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.ob[1] = (y * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.ob[2] = 3;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[0] = WALL_COLOR_R;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[1] = WALL_COLOR_G;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[2] = WALL_COLOR_B;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[3] = 0xff;

        offset += 4;
      }

      if ((offset < 8) && (x < (MAP_SIZE - 1)) && (((HIGH_WALL_TILE != IS_TILE_BLOCKED((x + 1), (y))) && (EMPTY_HIGH_WALL_TILE != IS_TILE_BLOCKED((x + 1), (y)))))) {
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.ob[0] = (x * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.ob[1] = (y * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.ob[2] = 3;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[0] = WALL_COLOR_R;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[1] = WALL_COLOR_G;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[2] = WALL_COLOR_B;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[3] = 0xff;

        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.ob[0] = (x * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.ob[1] = (y * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.ob[2] = -1;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[0] = WALL_COLOR_R - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[1] = WALL_COLOR_G - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[2] = WALL_COLOR_B - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[3] = 0xff;

        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.ob[0] = (x * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.ob[1] = (y * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.ob[2] = -1;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[0] = WALL_COLOR_R - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[1] = WALL_COLOR_G - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[2] = WALL_COLOR_B - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[3] = 0xff;

        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.ob[0] = (x * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.ob[1] = (y * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.ob[2] = 3;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[0] = WALL_COLOR_R;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[1] = WALL_COLOR_G;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[2] = WALL_COLOR_B;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[3] = 0xff;

        offset += 4;
      }

      if ((offset < 8) && (x > 0) && (((HIGH_WALL_TILE != IS_TILE_BLOCKED((x - 1), (y))) && (EMPTY_HIGH_WALL_TILE != IS_TILE_BLOCKED((x - 1), (y)))))) {
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.ob[0] = (x * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.ob[1] = (y * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.ob[2] = 3;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[0] = WALL_COLOR_R;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[1] = WALL_COLOR_G;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[2] = WALL_COLOR_B;
        map_geom[(i * VERTS_PER_TILE) + 0 + offset].v.cn[3] = 0xff;

        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.ob[0] = (x * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.ob[1] = (y * TILE_SIZE) + 2;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.ob[2] = -1;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[0] = WALL_COLOR_R - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[1] = WALL_COLOR_G - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[2] = WALL_COLOR_B - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 1 + offset].v.cn[3] = 0xff;

        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.ob[0] = (x * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.ob[1] = (y * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.ob[2] = -1;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[0] = WALL_COLOR_R - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[1] = WALL_COLOR_G - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[2] = WALL_COLOR_B - DARKEN_VERT;
        map_geom[(i * VERTS_PER_TILE) + 2 + offset].v.cn[3] = 0xff;

        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.ob[0] = (x * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.ob[1] = (y * TILE_SIZE);
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.ob[2] = 3;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.flag = 0;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.tc[0] = 0;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.tc[1] = 0;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[0] = WALL_COLOR_R;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[1] = WALL_COLOR_G;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[2] = WALL_COLOR_B;
        map_geom[(i * VERTS_PER_TILE) + 3 + offset].v.cn[3] = 0xff;

        offset += 4;
      }
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
		    gSP2Triangles(glistp++,4,5,6,0,4,6,7,0);
		  }
		}
	}
}


int isTileBlocked(int x, int y) {
	return IS_TILE_BLOCKED(x, y);
}



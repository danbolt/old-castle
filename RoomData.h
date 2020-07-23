
// This corresponds to data about the generated rooms

#ifndef ROOM_DATA_H
#define ROOM_DATA_H

#include <nusys.h>

#define MAP_SIZE 125
#define ROOM_SIZE 25
#define TILE_SIZE 2
#define INV_TILE_SIZE (1.0f / TILE_SIZE)

#define FLOOR_TILE 0
#define LOW_WALL_TILE 1
#define HIGH_WALL_TILE 2
#define EMPTY_HIGH_WALL_TILE 3

#define RENDER_DISTANCE_IN_TILES 12
#define RENDER_DISTANCE (RENDER_DISTANCE_IN_TILES * TILE_SIZE)
#define RENDER_DISTANCE_SQ (RENDER_DISTANCE * RENDER_DISTANCE)

#define MAX_NUMBER_OF_ROOMS_PER_FLOOR ((MAP_SIZE / ROOM_SIZE) * (MAP_SIZE / ROOM_SIZE))

typedef enum {
  StartingRoom, // The room where the player spawns
  StaircaseRoom, // A room with a starcase to another floor
  EnemyRoom, // A room full of enemies
  RestRoom, // A room without enemies to take a break
  NoRoom, // The void
  HallwayRoom, // A room that's more of a hallawy to another room
} RoomType;

typedef struct {
  u8 x;
  u8 y;
  u8 width;
  u8 height;

  RoomType type;
} GeneratedRoom;

void initMap(GeneratedRoom* rooms);

void initEnemiesForMap(GeneratedRoom* rooms);

void updateMapFromInfo();

void renderMapTiles(float camera_x, float camera_y, float camera_rotation);

int isTileBlocked(int x, int y);

#endif
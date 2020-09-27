
// This corresponds to data about the generated rooms

#ifndef ROOM_DATA_H
#define ROOM_DATA_H

#include <nusys.h>

#include "xorshift.h"

#define MAP_SIZE 125
#define ROOM_SIZE 25
#define TILE_SIZE 2
#define INV_TILE_SIZE (1.0f / TILE_SIZE)

#define FLOOR_TILE 0
#define LOW_WALL_TILE 1
#define HIGH_WALL_TILE 2
#define EMPTY_HIGH_WALL_TILE 3
#define STAIRCASE_A 4
#define STAIRCASE_B 5
#define STAIRCASE_C 6
#define STAIRCASE_D 7
#define STAIRCASE_E 8

#define LOW_WALL_HEIGHT 60

#define RENDER_DISTANCE_IN_TILES 12
#define RENDER_DISTANCE (RENDER_DISTANCE_IN_TILES * TILE_SIZE)
#define RENDER_DISTANCE_SQ (RENDER_DISTANCE * RENDER_DISTANCE)

#define MAX_NUMBER_OF_ROOMS_PER_FLOOR ((MAP_SIZE / ROOM_SIZE) * (MAP_SIZE / ROOM_SIZE))

#define ROOM_VERT_BUFFER_SIZE 2048
#define ROOM_COMMAND_BUFFER_SIZE 512

#define ROOM_MACRO_SCALE 0.01f
#define ROOM_VERT_DATA_SCALE 100

#define MAX_ENEMIES_PER_ROOM 16

typedef enum {
  StartingRoom, // The room where the player spawns
  StaircaseRoom, // A room with a starcase to another floor
  EnemyRoom, // A room full of enemies
  RestRoom, // A room without enemies and consisting of some dialogue
  NoRoom, // The void
  HallwayRoom, // A room that's more of a hallawy to another room
  BossARoom, // The room for boss A
  LockRoom
} RoomType;

#define NO_STAIRS_DIRECTION -1
typedef struct {
  u8 x;
  u8 y;
  u8 width;
  u8 height;
  s8 stairsDirectionIndex; // used if this is a staircaseRoom

  RoomType type;

  Vtx verts[ROOM_VERT_BUFFER_SIZE];
  Gfx commands[ROOM_COMMAND_BUFFER_SIZE];

  s8 lockIndex;

  u8 enemies[MAX_ENEMIES_PER_ROOM];
  u8 numberOfEnemies;
} GeneratedRoom;

int initMap(GeneratedRoom* rooms, xorshift32_state* seed, int floorNumber);

void initEnemiesForMap(GeneratedRoom* rooms);

int isTileBlocked(int x, int y);

#endif
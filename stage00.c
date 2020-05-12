#include <assert.h>
#include <nusys.h>
#include "main.h"
#include "graphic.h"
#include <gu.h>
#include "os_time.h"

#include "sample_tex.h"

#define PLAYER_MOVE_SPEED 0.134753f
#define DEFAULT_TARGET_DISTANCE 9.9f
#define JUMP_SPEED_PER_TICK 0.03115f
#define LAND_SPEED_PER_TICK 0.035f
#define DEATH_FALL_PER_TICK 0.18f

#define FLOOR_TILE 0
#define LOW_WALL_TILE 1
#define HIGH_WALL_TILE 2
#define EMPTY_HIGH_WALL_TILE 3

#define PLAYER_HIT_RADIUS 1
#define PLAYER_HIT_RADIUS_SQ (PLAYER_HIT_RADIUS * PLAYER_HIT_RADIUS)

#define EMITTER_DEAD 0
#define EMITTER_ALIVE 1

typedef enum {
  Move,
  Jumping,
  Landed,
  Dead
} PlayerState;

static float player_x;
static float player_y;
static float player_jump_x;
static float player_jump_y;
static float player_facing_x;
static float player_facing_y;
static float target_distance;
static PlayerState player_state;
static float player_t;
static int HIT_WALL_WHILE_JUMPING;

static float camera_rotation;
static float player_rotation;

static float camera_x; /* The display position-X */
static float camera_y; /* The display position-Y */

static int newTileX;
static int newTileY;

static OSTime time;
static OSTime delta;

#define BULLET_COUNT 200
#define EMITTER_COUNT 40

#define CAMERA_MOVE_SPEED 0.01726f
#define CAMERA_TURN_SPEED 0.03826f
#define CAMERA_DISTANCE 13.23f
#define CAMERA_HEIGHT 41.0f
#define CAMERA_LERP 0.13f

#define MAP_SIZE 125
#define ROOM_SIZE 25
#define TILE_SIZE 2
#define INV_TILE_SIZE (1.0f / TILE_SIZE)

#define RENDER_DISTANCE_IN_TILES 12
#define RENDER_DISTANCE (RENDER_DISTANCE_IN_TILES * TILE_SIZE)
#define RENDER_DISTANCE_SQ (RENDER_DISTANCE * RENDER_DISTANCE)

typedef struct {
  float x;
  float y;
} Position;

typedef struct {
  float x;
  float y;
} Velocity;

typedef struct {
  Mtx mat;
} EntityTransform;

typedef struct {
  s8 emitterIndex;

  u8 shotsToFire;
  u8 spreadSpeadInDegrees;
  float period;
  float t;
} AimEmitterData;

// TODO: time to break into different translation units for sanity

typedef enum {
  StartingRoom, // The room where the player spawns
  StaircaseRoom, // A room with a starcase to another floor
  EnemyRoom, // A room full of enemies
  RestRoom, // A room without enemies to take a break
} RoomType;

typedef struct {
  u8 x;
  u8 y;
  u8 width;
  u8 height;

  RoomType type;
} GeneratedRoom;

#define NUMBER_OF_ROOMS_PER_FLOOR ((MAP_SIZE / ROOM_SIZE) * (MAP_SIZE / ROOM_SIZE))

static Position BulletPositions[BULLET_COUNT];
static Velocity BulletVelocities[BULLET_COUNT];
static u8 BulletStates[BULLET_COUNT];
static u8 BulletRadiiSquared[BULLET_COUNT];
static EntityTransform BulletMatricies[BULLET_COUNT];
static int NextBulletIndex;

#define AIM_EMITTER_COUNT 32
static AimEmitterData AimEmitters[AIM_EMITTER_COUNT];

#define EMITTER_RADIUS 4.f
#define EMITTER_RADIUS_SQ (EMITTER_RADIUS * EMITTER_RADIUS)

static Position EmitterPositions[EMITTER_COUNT];
static Velocity EmitterVelocities[EMITTER_COUNT];
static EntityTransform EmitterMatricies[EMITTER_COUNT];
static u8 EmitterStates[EMITTER_COUNT];
static u32 EmitterTicks[EMITTER_COUNT];

static int MapInfo[MAP_SIZE * MAP_SIZE]; // 0 for empty, 1 for filled

#define VERTS_PER_TILE 8
static Vtx map_geom[MAP_SIZE * MAP_SIZE * VERTS_PER_TILE];

#define IS_TILE_BLOCKED(x, y) MapInfo[x + (y * MAP_SIZE)]

static Vtx bullet_test_geom[] =  {
        {         1,  1, 1, 0, 0, 0, 0, 0, 0, 0xff },
        {        -1,  1, 1, 0, 0, 0, 0, 0, 0, 0xff },
        {        -1, -1, 1, 0, 0, 0, 0, 0, 0, 0xff },
        {         1, -1, 1, 0, 0, 0, 0, 0, 0, 0xff },
        {        -1,  -1,  1, 0, 0, 0, 0,    0, 0,    0xff },
        {        -1,   1,  1, 0, 0, 0, 0,    0, 0, 0xff },
        {        -1,   1, -1, 0, 0, 0, 0,    0, 0, 0xff },
        {        -1,  -1, -1, 0, 0, 0, 0, 0, 0,    0xff },
        {        1,  1, 1, 0, 0, 0, 0, 0, 0, 0xff     },
        {        1,  -1, 1, 0, 0, 0, 0, 0, 0, 0xff  },
        {        1, -1, -1, 0, 0, 0, 0, 0, 0, 0xff },
        {        1, 1, -1, 0, 0, 0, 0, 0, 0, 0xff },
        {        -1,  1, 1, 0, 0, 0, 0, 0, 0, 0xff },
        {         1,  1, 1, 0, 0, 0, 0, 0, 0, 0xff },
        {         1, -1, 1, 0, 0, 0, 0, 0, 0, 0xff },
        {        -1, -1, 1, 0, 0, 0, 0, 0, 0, 0xff },
        {        -1,  1, 1, 0, 0, 0, 0, 0, 0, 0xff },
        {         1,  1, 1, 0, 0, 0, 0, 0, 0, 0xff },
        {         1,  1, -1, 0, 0, 0, 0, 0, 0, 0xff },
        {        -1,  1, -1, 0, 0, 0, 0, 0, 0, 0xff },
        {         1,  -1, 1, 0, 0, 0, 0, 0, 0, 0xff },
        {        -1,  -1, 1, 0, 0, 0, 0, 0, 0, 0xff },
        {        -1,  -1, -1, 0, 0, 0, 0, 0, 0, 0xff },
        {         1,  -1, -1, 0, 0, 0, 0, 0, 0, 0xff },
};

static Vtx emitter_test_geom[] =  {
        {         1,  1, 1, 0, 0, 0, 120, 120, 120, 0xff },
        {        -1,  1, 1, 0, 0, 0, 120, 120, 120, 0xff },
        {        -1, -1, 1, 0, 0, 0, 120, 120, 120, 0xff },
        {         1, -1, 1, 0, 0, 0, 120, 120, 120, 0xff },
        {        -1,  -1,  1, 0, 0, 0, 120,    120, 120,    0xff },
        {        -1,   1,  1, 0, 0, 0, 120,    120, 120, 0xff },
        {        -1,   1, -1, 0, 0, 0, 120,    120, 120, 0xff },
        {        -1,  -1, -1, 0, 0, 0, 120, 120, 120,    0xff },
        {        1,  1, 1, 0, 0, 0, 120, 120, 120, 0xff     },
        {        1,  -1, 1, 0, 0, 0, 120, 120, 120, 0xff  },
        {        1, -1, -1, 0, 0, 0, 120, 120, 120, 0xff },
        {        1, 1, -1, 0, 0, 0, 120, 120, 120, 0xff },
        {        -1,  1, 1, 0, 0, 0, 120, 120, 120, 0xff },
        {         1,  1, 1, 0, 0, 0, 120, 120, 120, 0xff },
        {         1, -1, 1, 0, 0, 0, 120, 120, 120, 0xff },
        {        -1, -1, 1, 0, 0, 0, 120, 120, 120, 0xff },
        {        -1,  1, 1, 0, 0, 0, 120, 120, 120, 0xff },
        {         1,  1, 1, 0, 0, 0, 120, 120, 120, 0xff },
        {         1,  1, -1, 0, 0, 0, 120, 120, 120, 0xff },
        {        -1,  1, -1, 0, 0, 0, 120, 120, 120, 0xff },
        {         1,  -1, 1, 0, 0, 0, 120, 120, 120, 0xff },
        {        -1,  -1, 1, 0, 0, 0, 120, 120, 120, 0xff },
        {        -1,  -1, -1, 0, 0, 0, 120, 120, 120, 0xff },
        {         1,  -1, -1, 0, 0, 0, 120, 120, 120, 0xff },
};

static Vtx jump_target_geom[] =  {
  {   1,  1, 0, 0, 0, 0, 0xff, 0, 0, 0xff },
  {   0,  1, 0, 0, 0, 0, 0xff, 0, 0, 0xff },
  {   0,  0, 0, 0, 0, 0, 0xff, 0, 0, 0xff },
  {   1,  0, 0, 0, 0, 0, 0xff, 0, 0, 0xff },

  {   0,  0, 0, 0, 0, 0, 0, 0, 0xff, 0xff },
  {  -1,  0, 0, 0, 0, 0, 0, 0, 0xff, 0xff },
  {  -1, -1, 0, 0, 0, 0, 0, 0, 0xff, 0xff },
  {   0, -1, 0, 0, 0, 0, 0, 0, 0xff, 0xff },
};

static Vtx player_cloak[] = {
{ -98, 112, 33, 0, 0, 0, 30, 30, 92, 255 },
{ -63, 145, -135, 0, 0, 0, 13, 13, 38, 255 },
{ 34, 98, 40, 0, 0, 0, 30, 30, 92, 255 },
{ -71, 88, 95, 0, 0, 0, 30, 30, 92, 255 },
{ 20, 88, 95, 0, 0, 0, 52, 52, 158, 255 },
{ -54, 78, 115, 0, 0, 0, 30, 30, 92, 255 },
{ -98, -110, 33, 0, 0, 0, 30, 30, 92, 255 },
{ -63, -143, -135, 0, 0, 0, 13, 13, 38, 255 },
{ 34, -96, 40, 0, 0, 0, 30, 30, 92, 255 },
{ -140, 1, -69, 0, 0, 0, 13, 13, 38, 255 },
{ 47, 1, 74, 0, 0, 0, 30, 30, 92, 255 },
{ -47, 1, -13, 0, 0, 0, 13, 13, 38, 255 },
{ -107, 1, 48, 0, 0, 0, 30, 30, 92, 255 },
{ -71, -86, 95, 0, 0, 0, 30, 30, 92, 255 },
{ 20, -86, 95, 0, 0, 0, 52, 52, 158, 255 },
{ 33, 1, 95, 0, 0, 0, 30, 30, 92, 255 },
{ -81, 1, 101, 0, 0, 0, 52, 52, 158, 255 },
{ -83, 1, 131, 0, 0, 0, 52, 52, 158, 255 },
{ -54, -76, 115, 0, 0, 0, 30, 30, 92, 255 },
};

static Vtx player_face[] = {
{ 14, 47, 88, 0, 0, 0, 201, 149, 113, 255 },
{ 20, 56, 125, 0, 0, 0, 153, 112, 84, 255 },
{ 26, 48, 162, 0, 0, 0, 153, 112, 84, 255 },
{ -4, 57, 78, 0, 0, 0, 67, 172, 237, 255 },
{ -21, 69, 137, 0, 0, 0, 52, 125, 201, 255 },
{ 12, 60, 191, 0, 0, 0, 19, 170, 255, 255 },
{ 14, -44, 88, 0, 0, 0, 201, 149, 113, 255 },
{ 20, -53, 125, 0, 0, 0, 153, 112, 84, 255 },
{ 26, -45, 162, 0, 0, 0, 153, 112, 84, 255 },
{ -16, 1, 103, 0, 0, 0, 255, 255, 255, 255 },
{ 56, 1, 172, 0, 0, 0, 153, 112, 84, 255 },
{ 47, 1, 79, 0, 0, 0, 201, 149, 113, 255 },
{ 59, 1, 124, 0, 0, 0, 201, 149, 113, 255 },
{ -4, -55, 78, 0, 0, 0, 67, 172, 237, 255 },
{ -21, -67, 137, 0, 0, 0, 52, 125, 201, 255 },
{ 12, -57, 191, 0, 0, 0, 19, 170, 255, 255 },
{ 52, 1, 204, 0, 0, 0, 19, 170, 255, 255 },
{ 34, 52, 165, 0, 0, 0, 0, 0, 0, 255 },
{ 49, 23, 165, 0, 0, 0, 0, 0, 0, 255 },
{ 43, 43, 131, 0, 0, 0, 51, 153, 27, 255 },
{ 58, 13, 131, 0, 0, 0, 255, 248, 253, 255 },
{ 38, -52, 164, 0, 0, 0, 0, 0, 0, 255 },
{ 52, -22, 164, 0, 0, 0, 0, 0, 0, 255 },
{ 40, -39, 131, 0, 0, 0, 51, 153, 27, 255 },
{ 54, -9, 131, 0, 0, 0, 255, 248, 253, 255 },
{ -73, 43, 94, 0, 0, 0, 46, 35, 153, 255 },
{ -60, 45, 179, 0, 0, 0, 67, 172, 237, 255 },
{ -73, -41, 94, 0, 0, 0, 46, 35, 153, 255 },
{ -60, -43, 179, 0, 0, 0, 67, 172, 237, 255 },
{ -30, 1, 218, 0, 0, 0, 67, 172, 237, 255 },
{ -101, 1, 121, 0, 0, 0, 46, 35, 153, 255 },
};

static Vtx player_legs[] = {
{ -22, -64, -23, 0, 0, 0, 170, 94, 176, 255 },
{ -22, 64, -23, 0, 0, 0, 170, 94, 176, 255 },
{ 22, -64, -23, 0, 0, 0, 157, 37, 176, 255 },
{ 16, -44, 58, 0, 0, 0, 170, 94, 176, 255 },
{ 22, 64, -23, 0, 0, 0, 141, 32, 158, 255 },
{ 16, 44, 58, 0, 0, 0, 170, 94, 176, 255 },
{ -22, -24, -23, 0, 0, 0, 170, 94, 176, 255 },
{ 22, -24, -23, 0, 0, 0, 157, 37, 176, 255 },
{ 16, -17, 58, 0, 0, 0, 170, 94, 176, 255 },
{ 22, 19, -23, 0, 0, 0, 141, 32, 158, 255 },
{ -22, 19, -23, 0, 0, 0, 170, 94, 176, 255 },
{ 16, 13, 58, 0, 0, 0, 170, 94, 176, 255 },
{ -15, -33, -134, 0, 0, 0, 86, 48, 89, 255 },
{ -15, -57, -134, 0, 0, 0, 86, 48, 89, 255 },
{ 38, 65, -134, 0, 0, 0, 86, 48, 89, 255 },
{ -15, 56, -134, 0, 0, 0, 86, 48, 89, 255 },
{ 38, 20, -134, 0, 0, 0, 86, 48, 89, 255 },
{ 38, -65, -134, 0, 0, 0, 86, 48, 89, 255 },
{ 38, -25, -134, 0, 0, 0, 86, 48, 89, 255 },
{ -15, 29, -134, 0, 0, 0, 86, 48, 89, 255 },
{ 19, 20, -88, 0, 0, 0, 141, 32, 158, 255 },
{ 19, 64, -88, 0, 0, 0, 141, 32, 158, 255 },
{ -19, 57, -88, 0, 0, 0, 86, 48, 89, 255 },
{ -19, 17, -88, 0, 0, 0, 86, 48, 89, 255 },
{ 19, -64, -88, 0, 0, 0, 141, 32, 158, 255 },
{ 19, -24, -88, 0, 0, 0, 141, 32, 158, 255 },
{ -19, -57, -88, 0, 0, 0, 86, 48, 89, 255 },
{ -19, -22, -88, 0, 0, 0, 86, 48, 89, 255 },
};

static Vtx player_sword[] = {
{ 83, -22, -12, 0, 0, 0, 0, 186, 167, 255 },
{ 83, -22, 12, 0, 0, 0, 40, 186, 168, 255 },
{ 118, -25, -11, 0, 0, 0, 0, 186, 167, 255 },
{ 118, -25, 11, 0, 0, 0, 40, 186, 168, 255 },
{ 134, -61, 14, 0, 0, 0, 40, 186, 168, 255 },
{ 134, -61, -14, 0, 0, 0, 40, 186, 168, 255 },
{ 429, -15, 1, 0, 0, 0, 182, 186, 186, 255 },
{ 83, 22, -12, 0, 0, 0, 0, 186, 167, 255 },
{ 83, 22, 12, 0, 0, 0, 40, 186, 168, 255 },
{ 118, 25, -11, 0, 0, 0, 0, 186, 167, 255 },
{ 118, 25, 11, 0, 0, 0, 40, 186, 168, 255 },
{ 83, 0, -12, 0, 0, 0, 0, 186, 167, 255 },
{ 83, 0, 12, 0, 0, 0, 40, 186, 168, 255 },
{ 118, 0, -11, 0, 0, 0, 0, 186, 167, 255 },
{ 118, 0, 11, 0, 0, 0, 40, 186, 168, 255 },
{ 134, 0, 21, 0, 0, 0, 40, 186, 168, 255 },
{ 134, 61, 14, 0, 0, 0, 40, 186, 168, 255 },
{ 134, 61, -14, 0, 0, 0, 40, 186, 168, 255 },
{ 134, 0, -21, 0, 0, 0, 0, 186, 167, 255 },
{ 429, 0, 2, 0, 0, 0, 182, 186, 186, 255 },
{ 429, 15, 1, 0, 0, 0, 182, 186, 186, 255 },
{ 365, 0, -4, 0, 0, 0, 40, 186, 168, 255 },
};

float fabs_d(float x) {
  if (x < 0.f) {
    return -x;
  }
  return x;
}

float lerp(float v0, float v1, float t) {
  return (1 - t) * v0 + t * v1;
}

float cubic(float t) {
  return t * t * t;
}

/* copy-paste from the nusnake example */
#define TOL ((float)1.0E-7)    /* Fix precision to 10^-7 because of the float type  */
#define M_PI_2    1.57079632679489661923
#define M_PI_4    0.78539816339744830962
#define M_RTOD    (180.0/3.14159265358979323846)

#define NRECTAB 100
static float reciprocal_table_f[NRECTAB] = {
    0,1.0/1.0,1.0/2.0,1.0/3.0,1.0/4.0,1.0/5.0,1.0/6.0,1.0/7.0,1.0/8.0,1.0/9.0,1.0/10.0,
    1.0/11.0,1.0/12.0,1.0/13.0,1.0/14.0,1.0/15.0,1.0/16.0,1.0/17.0,1.0/18.0,1.0/19.0,1.0/20.0,
    1.0/21.0,1.0/22.0,1.0/23.0,1.0/24.0,1.0/25.0,1.0/26.0,1.0/27.0,1.0/28.0,1.0/29.0,1.0/30.0,
    1.0/31.0,1.0/32.0,1.0/33.0,1.0/34.0,1.0/35.0,1.0/36.0,1.0/37.0,1.0/38.0,1.0/39.0,1.0/40.0,
    1.0/41.0,1.0/42.0,1.0/43.0,1.0/44.0,1.0/45.0,1.0/46.0,1.0/47.0,1.0/48.0,1.0/49.0,1.0/50.0,
    1.0/51.0,1.0/52.0,1.0/53.0,1.0/54.0,1.0/55.0,1.0/56.0,1.0/57.0,1.0/58.0,1.0/59.0,1.0/60.0,
    1.0/61.0,1.0/62.0,1.0/63.0,1.0/64.0,1.0/65.0,1.0/66.0,1.0/67.0,1.0/68.0,1.0/69.0,1.0/70.0,
    1.0/71.0,1.0/72.0,1.0/73.0,1.0/74.0,1.0/75.0,1.0/76.0,1.0/77.0,1.0/78.0,1.0/79.0,1.0/80.0,
    1.0/81.0,1.0/82.0,1.0/83.0,1.0/84.0,1.0/85.0,1.0/86.0,1.0/87.0,1.0/88.0,1.0/89.0,1.0/90.0,
    1.0/91.0,1.0/92.0,1.0/93.0,1.0/94.0,1.0/95.0,1.0/96.0,1.0/97.0,1.0/89.0,1.0/99.0
};

void updateMapFromInfo() {
  int i;

  for (i = 0; i < MAP_SIZE * MAP_SIZE; i++) {
    short x = i % MAP_SIZE;
    short y = i / MAP_SIZE;

    int roll = guRandom() % 0x0f;

    map_geom[(i * VERTS_PER_TILE) + 0].v.ob[0] = (x * TILE_SIZE);
    map_geom[(i * VERTS_PER_TILE) + 0].v.ob[1] = (y * TILE_SIZE);
    map_geom[(i * VERTS_PER_TILE) + 0].v.ob[2] = (MapInfo[i] == 0) ? -1 : (MapInfo[i] == LOW_WALL_TILE ? 1 : 10);
    map_geom[(i * VERTS_PER_TILE) + 0].v.flag = 0;
    map_geom[(i * VERTS_PER_TILE) + 0].v.tc[0] = 0;
    map_geom[(i * VERTS_PER_TILE) + 0].v.tc[1] = 0;
    map_geom[(i * VERTS_PER_TILE) + 0].v.cn[0] = (MapInfo[i] == 0) ? (0x60 + roll) : (0x71 + 0x1a);
    map_geom[(i * VERTS_PER_TILE) + 0].v.cn[1] = (MapInfo[i] == 0) ? (0x60 + roll) : (0x44 + 0x00);
    map_geom[(i * VERTS_PER_TILE) + 0].v.cn[2] = (MapInfo[i] == 0) ? (0x3d + roll) : (0x12 + 0x00);
    map_geom[(i * VERTS_PER_TILE) + 0].v.cn[3] = (MapInfo[i] == 0) ? (0xff + roll) : (0x01 + 0x00);

    map_geom[(i * VERTS_PER_TILE) + 1].v.ob[0] = (x * TILE_SIZE) + TILE_SIZE;
    map_geom[(i * VERTS_PER_TILE) + 1].v.ob[1] = (y * TILE_SIZE);
    map_geom[(i * VERTS_PER_TILE) + 1].v.ob[2] = (MapInfo[i] == 0) ? -1 : (MapInfo[i] == LOW_WALL_TILE ? 1 : 10);
    map_geom[(i * VERTS_PER_TILE) + 1].v.flag = 0;
    map_geom[(i * VERTS_PER_TILE) + 1].v.tc[0] = 0;
    map_geom[(i * VERTS_PER_TILE) + 1].v.tc[1] = 0;
    map_geom[(i * VERTS_PER_TILE) + 1].v.cn[0] = (MapInfo[i] == 0) ? (0x60 + roll) : (0x71 + 0x1a);
    map_geom[(i * VERTS_PER_TILE) + 1].v.cn[1] = (MapInfo[i] == 0) ? (0x60 + roll) : (0x44 + 0x00);
    map_geom[(i * VERTS_PER_TILE) + 1].v.cn[2] = (MapInfo[i] == 0) ? (0x3d + roll) : (0x12 + 0x00);
    map_geom[(i * VERTS_PER_TILE) + 1].v.cn[3] = (MapInfo[i] == 0) ? (0xff + roll) : (0x01 + 0x00);

    map_geom[(i * VERTS_PER_TILE) + 2].v.ob[0] = (x * TILE_SIZE) + TILE_SIZE;
    map_geom[(i * VERTS_PER_TILE) + 2].v.ob[1] = (y * TILE_SIZE) + TILE_SIZE;
    map_geom[(i * VERTS_PER_TILE) + 2].v.ob[2] = (MapInfo[i] == 0) ? -1 : (MapInfo[i] == LOW_WALL_TILE ? 1 : 10);
    map_geom[(i * VERTS_PER_TILE) + 2].v.flag = 0;
    map_geom[(i * VERTS_PER_TILE) + 2].v.tc[0] = 0;
    map_geom[(i * VERTS_PER_TILE) + 2].v.tc[1] = 0;
    map_geom[(i * VERTS_PER_TILE) + 2].v.cn[0] = (MapInfo[i] == 0) ? (0x60 + roll) : (0x71 + 0x40);
    map_geom[(i * VERTS_PER_TILE) + 2].v.cn[1] = (MapInfo[i] == 0) ? (0x60 + roll) : (0x44 + 0x00);
    map_geom[(i * VERTS_PER_TILE) + 2].v.cn[2] = (MapInfo[i] == 0) ? (0x3d + roll) : (0x12 + 0x40);
    map_geom[(i * VERTS_PER_TILE) + 2].v.cn[3] = (MapInfo[i] == 0) ? (0xff + roll) : (0x01 + 0x40);

    map_geom[(i * VERTS_PER_TILE) + 3].v.ob[0] = (x * TILE_SIZE);
    map_geom[(i * VERTS_PER_TILE) + 3].v.ob[1] = (y * TILE_SIZE) + TILE_SIZE;
    map_geom[(i * VERTS_PER_TILE) + 3].v.ob[2] = (MapInfo[i] == 0) ? -1 : (MapInfo[i] == LOW_WALL_TILE ? 1 : 10);
    map_geom[(i * VERTS_PER_TILE) + 3].v.flag = 0;
    map_geom[(i * VERTS_PER_TILE) + 3].v.tc[0] = 0;
    map_geom[(i * VERTS_PER_TILE) + 3].v.tc[1] = 0;
    map_geom[(i * VERTS_PER_TILE) + 3].v.cn[0] = (MapInfo[i] == 0) ? (0x60 + roll) : (0x71 + 0x1a);
    map_geom[(i * VERTS_PER_TILE) + 3].v.cn[1] = (MapInfo[i] == 0) ? (0x60 + roll) : (0x44 + 0x00);
    map_geom[(i * VERTS_PER_TILE) + 3].v.cn[2] = (MapInfo[i] == 0) ? (0x3d + roll) : (0x12 + 0x00);
    map_geom[(i * VERTS_PER_TILE) + 3].v.cn[3] = (MapInfo[i] == 0) ? (0xff + roll) : (0x01 + 0x00);

    if (IS_TILE_BLOCKED(x, y)) {
      map_geom[(i * VERTS_PER_TILE) + 4].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 4].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 4].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 4].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 4].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 4].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[0] = 0x11;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[1] = 0x24;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[2] = 0x12;
      map_geom[(i * VERTS_PER_TILE) + 4].v.cn[3] = 0x01;

      map_geom[(i * VERTS_PER_TILE) + 5].v.ob[0] = (x * TILE_SIZE) + TILE_SIZE;
      map_geom[(i * VERTS_PER_TILE) + 5].v.ob[1] = (y * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 5].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 5].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 5].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 5].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[0] = 0x11;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[1] = 0x24;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[2] = 0x12;
      map_geom[(i * VERTS_PER_TILE) + 5].v.cn[3] = 0x01;

      map_geom[(i * VERTS_PER_TILE) + 6].v.ob[0] = (x * TILE_SIZE) + TILE_SIZE;
      map_geom[(i * VERTS_PER_TILE) + 6].v.ob[1] = (y * TILE_SIZE) + TILE_SIZE;
      map_geom[(i * VERTS_PER_TILE) + 6].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 6].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 6].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 6].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[0] = 0x11;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[1] = 0x24;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[2] = 0x12;
      map_geom[(i * VERTS_PER_TILE) + 6].v.cn[3] = 0x01;

      map_geom[(i * VERTS_PER_TILE) + 7].v.ob[0] = (x * TILE_SIZE);
      map_geom[(i * VERTS_PER_TILE) + 7].v.ob[1] = (y * TILE_SIZE) + TILE_SIZE;
      map_geom[(i * VERTS_PER_TILE) + 7].v.ob[2] = -1;
      map_geom[(i * VERTS_PER_TILE) + 7].v.flag = 0;
      map_geom[(i * VERTS_PER_TILE) + 7].v.tc[0] = 0;
      map_geom[(i * VERTS_PER_TILE) + 7].v.tc[1] = 0;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[0] = 0x11;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[1] = 0x24;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[2] = 0x12;
      map_geom[(i * VERTS_PER_TILE) + 7].v.cn[3] = 0x01;
    }
  }
}

int consumeNextBullet() {
  int i;
  int iRaw;
  int foundNewBullet = 0;
  int nextCandidate = 0;
  int result = -1;

  // Find the next bullet to shoot


  for (i = 0; i < BULLET_COUNT; i++) {
    if (BulletStates[i] != 0) {
      continue;
    }


    result = i;
    NextBulletIndex = i;
  }

  return result;
}

void initMap(GeneratedRoom* rooms) {
  int i;
  int j;

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

        rooms[roomIndex].x = (ROOM_SIZE - w) / 2;
        rooms[roomIndex].y = (ROOM_SIZE - h) / 2;
        rooms[roomIndex].width = w;
        rooms[roomIndex].height = h;
        rooms[roomIndex].type = EnemyRoom;
      }

      // If we're outside the room, don't worry about it
      if ((roomX < rooms[roomIndex].x) || (roomX >= (rooms[roomIndex].x + rooms[roomIndex].width)) || (roomY < rooms[roomIndex].y) || (roomY >= (rooms[roomIndex].y + rooms[roomIndex].height))) {
        continue;
      }

      if ((roomX == rooms[roomIndex].x) || (roomX == (rooms[roomIndex].x + rooms[roomIndex].width - 1)) || (roomY == rooms[roomIndex].y) || (roomY == (rooms[roomIndex].y + rooms[roomIndex].height - 1))) {
        MapInfo[j * MAP_SIZE + i] = HIGH_WALL_TILE;
      } else if ((roomX > rooms[roomIndex].x) && (roomX < (rooms[roomIndex].x + rooms[roomIndex].width - 1)) && (roomY > rooms[roomIndex].y) && (roomY < (rooms[roomIndex].y + rooms[roomIndex].height - 1))) {
        int roll = guRandom() % 30;

        if (roll == 0) {
          MapInfo[j * MAP_SIZE + i] = LOW_WALL_TILE;
        } else {
          MapInfo[j * MAP_SIZE + i] = FLOOR_TILE;
        }
      }
    }
  }
}

void initEnemiesForMap(GeneratedRoom* rooms) {
  int i;

  for (i = 0; i < AIM_EMITTER_COUNT; i++) {
    EmitterStates[i] = EMITTER_ALIVE;
    EmitterPositions[i].x = (MAP_SIZE * TILE_SIZE * 0.5f) + (sinf(((float)i) / 13 * M_PI * 2) * (MAP_SIZE * TILE_SIZE * 0.1525f));
    EmitterPositions[i].y = (MAP_SIZE * TILE_SIZE * 0.5f) + (cosf(((float)i) / 13 * M_PI * 2) * (MAP_SIZE * TILE_SIZE * 0.1525f));
    EmitterVelocities[i].x = 0.f;
    EmitterVelocities[i].y = 0.f;

    AimEmitters[i].emitterIndex = i;
  }
}

/* The initialization of stage 0 */
void initStage00(void)
{
  int i;
  GeneratedRoom rooms[NUMBER_OF_ROOMS_PER_FLOOR];

  target_distance = DEFAULT_TARGET_DISTANCE;
  player_state = Move;
  player_t = 0.f;
  HIT_WALL_WHILE_JUMPING = 0;

  camera_x = 0.0f;
  camera_y = 0.0f;

  time = 0;
  delta = 0;

  camera_rotation = 0.00001f;
  player_rotation = 0.f;

  for (i = 0; i < BULLET_COUNT; i++) {
    float r = i / (float)((guRandom() % 10 / 10.f) * M_PI * 2.f);

    BulletPositions[i].x = MAP_SIZE * TILE_SIZE * 0.15f;
    BulletPositions[i].y = MAP_SIZE * TILE_SIZE * 0.15f;

    BulletVelocities[i].x = cosf(guRandom() % 7) * 0.05f;
    BulletVelocities[i].y = sinf(guRandom() % 7) * 0.05f;

    BulletStates[i] = 0;
    BulletRadiiSquared[i] = 1 * 1;

    guMtxIdent(&(BulletMatricies[i].mat));   
  }
  NextBulletIndex = 0;

  for (i = 0; i < EMITTER_COUNT; i++) {
    EmitterStates[i] = EMITTER_DEAD;
    EmitterPositions[i].x = 0.f;
    EmitterPositions[i].y = 0.f;
    EmitterVelocities[i].x = 0.f;
    EmitterVelocities[i].y = 0.f;
    EmitterTicks[i] = 0;

    guMtxIdent(&(EmitterMatricies[i].mat));   
  }

  for (i = 0; i < AIM_EMITTER_COUNT; i++) { 
    AimEmitters[i].emitterIndex = -1;
    AimEmitters[i].period = 2.0f;
    AimEmitters[i].shotsToFire = 1;
    AimEmitters[i].spreadSpeadInDegrees = 90.f;
    AimEmitters[i].t = 0.f + (guRandom() % 5);
  }

  initMap(rooms);
  initEnemiesForMap(rooms);
  updateMapFromInfo();

  player_x = rooms[0].x + (rooms[0].width / 2);
  player_y = rooms[0].y + (rooms[0].height / 2);
}

void addBulletToDisplayList()
{
  int roll1 = guRandom() % 20;
  int roll2 = guRandom() % 20;
  int roll3 = guRandom() % 20;
  int roll4 = guRandom() % 20;

  gSP2Triangles(glistp++, roll1 + 0, roll1 + 1, roll1 + 2, 0, roll1 + 0, roll1 + 2, roll1 + 3, 0);
  gSP2Triangles(glistp++, roll2 + 0, roll2 + 1, roll2 + 2, 0, roll2 + 0, roll2 + 2, roll2 + 3, 0);
  gSP2Triangles(glistp++, roll3 + 0, roll3 + 1, roll3 + 2, 0, roll3 + 0, roll3 + 2, roll3 + 3, 0);
  gSP2Triangles(glistp++, roll4 + 0, roll4 + 1, roll4 + 2, 0, roll4 + 0, roll4 + 2, roll4 + 3, 0);
}

void addEmitterToDisplayList()
{
  gSPVertex(glistp++,&(emitter_test_geom[guRandom() % 21]), 4, 0);
  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
  gSPVertex(glistp++,&(emitter_test_geom[guRandom() % 21]), 4, 0);
  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
  gSPVertex(glistp++,&(emitter_test_geom[guRandom() % 21]), 4, 0);
  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
  gSPVertex(glistp++,&(emitter_test_geom[guRandom() % 21]), 4, 0);
  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
}

void addPlayerDisplayList()
{
  gSPVertex(glistp++,&(player_cloak[0]), 19, 0);
  gSP2Triangles(glistp++, 9, 1, 11, 0, 2, 1, 0, 0);
  gSP2Triangles(glistp++, 9, 0, 1, 0, 1, 10, 11, 0);
  gSP2Triangles(glistp++, 0, 16, 3, 0, 2, 3, 4, 0);
  gSP2Triangles(glistp++, 10, 4, 15, 0, 3, 17, 5, 0);
  gSP2Triangles(glistp++, 9, 11, 7, 0, 8, 6, 7, 0);
  gSP2Triangles(glistp++, 9, 6, 12, 0, 7, 10, 8, 0);
  gSP2Triangles(glistp++, 6, 16, 12, 0, 8, 13, 6, 0);
  gSP2Triangles(glistp++, 10, 14, 8, 0, 13, 17, 16, 0);
  gSP2Triangles(glistp++, 14, 18, 13, 0, 9, 12, 0, 0);
  gSP2Triangles(glistp++, 1, 2, 10, 0, 0, 12, 16, 0);
  gSP2Triangles(glistp++, 2, 0, 3, 0, 10, 2, 4, 0);
  gSP2Triangles(glistp++, 3, 16, 17, 0, 4, 3, 5, 0);
  gSP2Triangles(glistp++, 9, 7, 6, 0, 7, 11, 10, 0);
  gSP2Triangles(glistp++, 6, 13, 16, 0, 8, 14, 13, 0);
  gSP2Triangles(glistp++, 10, 15, 14, 0, 13, 18, 17, 0);
  gSP2Triangles(glistp++, 4, 17, 14, 0, 14, 15, 4, 0);
  gSP2Triangles(glistp++, 4, 5, 17, 0, 17, 18, 14, 0);

  gSPVertex(glistp++,&(player_face[0]), 31, 0);
  gSP2Triangles(glistp++, 2, 4, 5, 0, 10, 5, 16, 0);
  gSP2Triangles(glistp++, 9, 0, 11, 0, 2, 12, 1, 0);
  gSP2Triangles(glistp++, 0, 12, 11, 0, 1, 3, 4, 0);
  gSP2Triangles(glistp++, 8, 14, 7, 0, 10, 15, 8, 0);
  gSP2Triangles(glistp++, 8, 12, 10, 0, 6, 12, 7, 0);
  gSP2Triangles(glistp++, 7, 13, 6, 0, 2, 1, 4, 0);
  gSP2Triangles(glistp++, 10, 2, 5, 0, 2, 10, 12, 0);
  gSP2Triangles(glistp++, 0, 1, 12, 0, 1, 0, 3, 0);
  gSP2Triangles(glistp++, 8, 15, 14, 0, 10, 16, 15, 0);
  gSP2Triangles(glistp++, 9, 11, 6, 0, 8, 7, 12, 0);
  gSP2Triangles(glistp++, 6, 11, 12, 0, 7, 14, 13, 0);
  gSP2Triangles(glistp++, 18, 20, 19, 0, 17, 18, 19, 0);
  gSP2Triangles(glistp++, 22, 23, 24, 0, 21, 23, 22, 0);
  gSP2Triangles(glistp++, 14, 28, 27, 0, 5, 25, 26, 0);
  gSP2Triangles(glistp++, 13, 14, 27, 0, 4, 3, 25, 0);
  gSP2Triangles(glistp++, 15, 29, 28, 0, 27, 28, 30, 0);
  gSP2Triangles(glistp++, 28, 29, 30, 0, 26, 30, 29, 0);
  gSP2Triangles(glistp++, 25, 30, 26, 0, 14, 15, 28, 0);
  gSP2Triangles(glistp++, 5, 4, 25, 0, 29, 5, 26, 0);
  gSP2Triangles(glistp++, 15, 16, 29, 0, 16, 5, 29, 0);

  gSPVertex(glistp++,&(player_legs[0]), 28, 0);
  gSP2Triangles(glistp++, 1, 5, 4, 0, 7, 3, 2, 0);
  gSP2Triangles(glistp++, 2, 3, 0, 0, 22, 14, 15, 0);
  gSP2Triangles(glistp++, 8, 0, 3, 0, 11, 6, 8, 0);
  gSP2Triangles(glistp++, 10, 7, 6, 0, 9, 8, 7, 0);
  gSP2Triangles(glistp++, 4, 11, 9, 0, 21, 16, 14, 0);
  gSP2Triangles(glistp++, 5, 10, 11, 0, 12, 17, 13, 0);
  gSP2Triangles(glistp++, 15, 16, 19, 0, 25, 17, 18, 0);
  gSP2Triangles(glistp++, 20, 19, 16, 0, 24, 13, 17, 0);
  gSP2Triangles(glistp++, 27, 18, 12, 0, 26, 12, 13, 0);
  gSP2Triangles(glistp++, 23, 15, 19, 0, 10, 22, 23, 0);
  gSP2Triangles(glistp++, 9, 23, 20, 0, 4, 20, 21, 0);
  gSP2Triangles(glistp++, 1, 21, 22, 0, 0, 27, 26, 0);
  gSP2Triangles(glistp++, 6, 25, 27, 0, 2, 26, 24, 0);
  gSP2Triangles(glistp++, 7, 24, 25, 0, 7, 8, 3, 0);
  gSP2Triangles(glistp++, 22, 21, 14, 0, 8, 6, 0, 0);
  gSP2Triangles(glistp++, 11, 10, 6, 0, 10, 9, 7, 0);
  gSP2Triangles(glistp++, 9, 11, 8, 0, 4, 5, 11, 0);
  gSP2Triangles(glistp++, 21, 20, 16, 0, 5, 1, 10, 0);
  gSP2Triangles(glistp++, 12, 18, 17, 0, 15, 14, 16, 0);
  gSP2Triangles(glistp++, 25, 24, 17, 0, 20, 23, 19, 0);
  gSP2Triangles(glistp++, 24, 26, 13, 0, 27, 25, 18, 0);
  gSP2Triangles(glistp++, 26, 27, 12, 0, 23, 22, 15, 0);
  gSP2Triangles(glistp++, 10, 1, 22, 0, 9, 10, 23, 0);
  gSP2Triangles(glistp++, 4, 9, 20, 0, 1, 4, 21, 0);
  gSP2Triangles(glistp++, 0, 6, 27, 0, 6, 7, 25, 0);
  gSP2Triangles(glistp++, 2, 0, 26, 0, 7, 2, 24, 0);
}

void addSwordDisplayList()
{
  gSPVertex(glistp++,&(player_sword[0]), 22, 0);
  gSP2Triangles(glistp++, 14, 4, 15, 0, 2, 1, 0, 0);
  gSP2Triangles(glistp++, 11, 2, 0, 0, 14, 1, 3, 0);
  gSP2Triangles(glistp++, 0, 12, 11, 0, 15, 6, 19, 0);
  gSP2Triangles(glistp++, 2, 18, 5, 0, 3, 5, 4, 0);
  gSP2Triangles(glistp++, 21, 19, 6, 0, 4, 5, 6, 0);
  gSP2Triangles(glistp++, 5, 21, 6, 0, 14, 3, 4, 0);
  gSP2Triangles(glistp++, 2, 3, 1, 0, 11, 13, 2, 0);
  gSP2Triangles(glistp++, 14, 12, 1, 0, 0, 1, 12, 0);
  gSP2Triangles(glistp++, 15, 4, 6, 0, 2, 13, 18, 0);
  gSP2Triangles(glistp++, 3, 2, 5, 0, 5, 18, 21, 0);
  gSP2Triangles(glistp++, 14, 15, 16, 0, 9, 7, 8, 0);
  gSP2Triangles(glistp++, 11, 7, 9, 0, 14, 10, 8, 0);
  gSP2Triangles(glistp++, 7, 11, 12, 0, 15, 19, 20, 0);
  gSP2Triangles(glistp++, 9, 17, 18, 0, 10, 16, 17, 0);
  gSP2Triangles(glistp++, 21, 20, 19, 0, 16, 20, 17, 0);
  gSP2Triangles(glistp++, 17, 20, 21, 0, 14, 16, 10, 0);
  gSP2Triangles(glistp++, 9, 8, 10, 0, 11, 9, 13, 0);
  gSP2Triangles(glistp++, 14, 8, 12, 0, 7, 12, 8, 0);
  gSP2Triangles(glistp++, 15, 20, 16, 0, 9, 18, 13, 0);
  gSP2Triangles(glistp++, 10, 17, 9, 0, 17, 21, 18, 0);
}

/* Make the display list and activate the task */
void makeDL00(void)
{
  Dynamic* dynamicp;
  char conbuf[20]; 
  int i;
  int j;
  Mtx playerTranslation;
  Mtx playerRotation;
  Mtx playerJumpRotation;
  Mtx playerScale;
  Mtx targetTranslation;
  Mtx targetScale;
  Mtx swordTranslation;
  Mtx swordScale;
  Mtx swordRotationX;
  Mtx swordRotationZ;
  int running = (fabs_d(player_facing_x) > 0.1f) || (fabs_d(player_facing_y) > 0.1f);

  /* Perspective normal value; I don't know what this does yet. */
  u16 perspNorm;

  /* Specify the display list buffer */
  dynamicp = &gfx_dynamic[gfx_gtask_no];
  glistp = &gfx_glist[gfx_gtask_no][0];

  /* Initialize RCP */
  gfxRCPInit();

  /* Clear the frame and Z-buffer */
  gfxClearCfb();

  /* projection,modeling matrix set */
  guPerspective(&dynamicp->projection, &perspNorm, 35.0f, (float)SCREEN_WD/(float)SCREEN_HT, 10.0f, 100.0f, 1.0f);
  guLookAt(&dynamicp->viewing, camera_x + (cosf(camera_rotation - (M_PI * 0.5f) ) * CAMERA_DISTANCE), camera_y + (sinf(camera_rotation - (M_PI * 0.5f) ) * CAMERA_DISTANCE), CAMERA_HEIGHT, camera_x, camera_y, 0.0f, 0.0f, 0.0f, 1.0f);

  gSPPerspNormalize(glistp++, perspNorm);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->projection)), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->viewing)), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++, G_CYC_1CYCLE);
  gDPSetRenderMode(glistp++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
  gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
  gSPSetGeometryMode(glistp++, G_ZBUFFER | G_CULL_BACK | G_SHADE | G_SHADING_SMOOTH);

  // Render Player
  guTranslate(&(playerTranslation), player_x, player_y, ((player_state == Jumping) ? (sinf(player_t * M_PI) * 6.2f) : 0.f));
  guRotate(&(playerRotation), player_rotation / M_PI * 180.f, 0.0f, 0.0f, 1.0f);
  if (player_state == Jumping) {
    guRotate(&(playerJumpRotation), sinf(player_t * M_PI * 2.f) * 75.2f, 0.0f, 1.0f, 0.0f);
  } else if (player_state == Move) {
    if (running) {
      guRotate(&(playerJumpRotation), sinf(time / 100000.f) * 5.f, 0.0f, 1.0f, 0.0f);
    } else {
      guRotate(&(playerJumpRotation), 5.f, 0.0f, 1.0f, 0.0f);
    }
  } else if (player_state == Landed) {
    guRotate(&(playerJumpRotation), 51.7f, 0.0f, 1.0f, 0.0f);
  } else if (player_state == Dead) {
    guRotate(&(playerJumpRotation), lerp(0.f, -90.f, (player_t)), 0.0f, 1.0f, 0.0f);
  }
  
  guScale(&(playerScale), 0.01, 0.01, 0.01);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(playerTranslation)), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(playerRotation)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(playerJumpRotation)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(playerScale)), G_MTX_PUSH | G_MTX_MODELVIEW);

  addPlayerDisplayList();

  // Determine sword display list
  guMtxIdent(&swordTranslation);
  if (player_state == Move) {
    guScale(&(swordScale), 0.9f, 0.9f, 0.8f);
    if (running) {
      guRotate(&(swordRotationX), 5.f + (6.3f * sinf(time / 150000.f)), 0.0f, 1.0f, 0.0f);
      guRotate(&(swordRotationZ), 120.f + (6.3f * sinf(time / 180000.f)), 0.0f, 0.0f, 1.0f);
    } else {
      guRotate(&(swordRotationX), 5.f + (4.f * sinf(time / 400000.f)), 0.0f, 1.0f, 0.0f);
      guRotate(&(swordRotationZ), 135.f, 0.0f, 0.0f, 1.0f);
    }
  } else if (player_state == Jumping) {
    float cubedScale = cubic(player_t);
    guTranslate(&swordTranslation, 0.f, 0.f, cubedScale * 10.f);
    if (cubedScale < 0.3f) {
      float scale = lerp(1.0f, 0.7f, cubedScale);
      guScale(&(swordScale), scale, scale, scale);
      guRotate(&(swordRotationX), lerp(5.f, -180.f, cubedScale / 0.3f), 0.0f, 1.0f, 0.0f);
      guRotate(&(swordRotationZ), lerp(135.f, 90.f, cubedScale / 0.3f), 0.0f, 0.0f, 1.0f);
    } else {
      float scale = lerp(0.7f, 2.1f, cubedScale);
      guScale(&(swordScale), scale, scale, scale);
      guRotate(&(swordRotationX), lerp(-90.f, 90.f, ((cubedScale - 0.3f) / 0.7f)), 0.0f, 1.0f, 0.0f);
      guRotate(&(swordRotationZ), lerp(110.f, 0.f, ((cubedScale - 0.3f) / 0.7f)), 0.0f, 0.0f, 1.0f);
    }
  } else if (player_state == Landed) {
    float scale = lerp(2.0f, 1.0f, player_t);
    guTranslate(&swordTranslation, 0.f, 0.f, (1 - scale) * 5.f);
    guScale(&(swordScale), scale, scale, scale);
    guRotate(&(swordRotationX), lerp(90.f, 5.f, player_t), 0.0f, 1.0f, 0.0f);
    guRotate(&(swordRotationZ), lerp(0.f, 120.f, player_t), 0.0f, 0.0f, 1.0f);
  } else if (player_state == Dead) {
    guScale(&(swordScale), 0.5f, 0.5f, 0.5f);
    guRotate(&(swordRotationX), 5.f + (4.f * sinf(time / 400000.f)), 0.0f, 1.0f, 0.0f);
    guRotate(&(swordRotationZ), 135.f, 0.0f, 0.0f, 1.0f);
  }
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(swordTranslation)), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(swordRotationZ)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(swordRotationX)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(swordScale)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  addSwordDisplayList();
  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

  guTranslate(&(targetTranslation), target_distance, 0.0f, 0.f);
  guScale(&(targetScale), (sinf(player_x) * 0.6f + 0.5f) + 0.4f, (cosf(player_y) * 0.6f + 0.5f) + 0.4f, 0.f);

  if (player_state == Move) { 
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(targetTranslation)), G_MTX_PUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(targetScale)), G_MTX_NOPUSH | G_MTX_MODELVIEW);

    gSPVertex(glistp++, &(jump_target_geom[0]), 4, 0);
    gSP2Triangles(glistp++, 0,1,2,0,0,2,3,0);
    gSPVertex(glistp++, &(jump_target_geom[4]), 4, 0);
    gSP2Triangles(glistp++, 0,1,2,0,0,2,3,0);

    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
  }

  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

  gDPPipeSync(glistp++);

  // Render Bullets
  for (i = 0; i < BULLET_COUNT; i++) {
    float dxSq;
    float dySq;
    if (BulletStates[i] == 0) {
      continue;
    }

    dxSq = player_x - BulletPositions[i].x;
    dxSq = dxSq * dxSq;
    if (dxSq > RENDER_DISTANCE_SQ) {
      continue;
    }

    dySq = player_y - BulletPositions[i].y;
    dySq = dySq * dySq;
    if (dySq > RENDER_DISTANCE_SQ) {
      continue;
    }

    if ((dySq + dxSq) >= RENDER_DISTANCE_SQ) {
      continue;
    }

    guTranslate(&(BulletMatricies[i].mat), BulletPositions[i].x, BulletPositions[i].y, 0.f);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(BulletMatricies[i])), G_MTX_PUSH | G_MTX_MODELVIEW);

    gSPVertex(glistp++,&(bullet_test_geom[0]), 23, 0);

    addBulletToDisplayList();

    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
  }

  // Render emitters
  for (i = 0; i < EMITTER_COUNT; i++) {
    float dxSq;
    float dySq;
    if (EmitterStates[i] == EMITTER_DEAD) {
      continue;
    }

    dxSq = player_x - EmitterPositions[i].x;
    dxSq = dxSq * dxSq;
    if (dxSq > RENDER_DISTANCE_SQ) {
      continue;
    }

    dySq = player_y - EmitterPositions[i].y;
    dySq = dySq * dySq;
    if (dySq > RENDER_DISTANCE_SQ) {
      continue;
    }

    if ((dySq + dxSq) >= RENDER_DISTANCE_SQ) {
      continue;
    }

    guTranslate(&(EmitterMatricies[i].mat), EmitterPositions[i].x, EmitterPositions[i].y, 0.f);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(EmitterMatricies[i])), G_MTX_PUSH | G_MTX_MODELVIEW);

    addEmitterToDisplayList();

    gDPPipeSync(glistp++);

    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
  }

  // Render map tiles
  for (i = MAX(0, (int)(((camera_x + cosf(camera_rotation + M_PI_2) * 4.f) / TILE_SIZE) - RENDER_DISTANCE_IN_TILES)); i < MIN(MAP_SIZE, (int)(((camera_x + cosf(camera_rotation + M_PI_2) * 4.f) / TILE_SIZE) + RENDER_DISTANCE_IN_TILES)); i++) {
    for (j = MAX(0, (int)(((camera_y + sinf(camera_rotation + M_PI_2) * 4.f) / TILE_SIZE) - RENDER_DISTANCE_IN_TILES)); j < MIN(MAP_SIZE, (int)(((camera_y + sinf(camera_rotation + M_PI_2) * 4.f) / TILE_SIZE) + RENDER_DISTANCE_IN_TILES)); j++) {
      if (IS_TILE_BLOCKED(i, j) == EMPTY_HIGH_WALL_TILE) {
        continue;
      }

      gSPVertex(glistp++,&(map_geom[((j * MAP_SIZE) + i) * VERTS_PER_TILE]), IS_TILE_BLOCKED(i, j) ? 8 : 4, 0);
      gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);

      if (IS_TILE_BLOCKED(i, j)) {
        gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
        gSP2Triangles(glistp++,2,1,5,0,2,5,6,0);
        gSP2Triangles(glistp++,3,2,6,0,3,6,7,0);
        gSP2Triangles(glistp++,0,3,7,0,0,7,4,0);
        gSP2Triangles(glistp++,1,0,4,0,1,4,5,0);
      }
    }
  }

  gDPPipeSync(glistp++);

  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);

  assert((glistp - gfx_glist[gfx_gtask_no]) < GFX_GLIST_LEN);

  /* Activate the task and 
     switch display buffers */
  nuGfxTaskStart(&gfx_glist[gfx_gtask_no][0],
		 (s32)(glistp - gfx_glist[gfx_gtask_no]) * sizeof (Gfx),
		 NU_GFX_UCODE_F3DLX_NON , NU_SC_NOSWAPBUFFER);

  nuDebTaskPerfBar0(1, 200, NU_SC_SWAPBUFFER);



  /*
  if(contPattern & 0x1)
  {

    nuDebConTextPos(0,1,3);
    sprintf(conbuf,"DL=%d / %d", (int)(glistp - gfx_glist[gfx_gtask_no]),  GFX_GLIST_LEN);
    nuDebConCPuts(0, conbuf);

    nuDebConTextPos(0,1,4);
    sprintf(conbuf,"time=%llu", time);
    nuDebConCPuts(0, conbuf);

    nuDebConTextPos(0,1,5);
    sprintf(conbuf,"delta=%llu", delta);
    nuDebConCPuts(0, conbuf);

    nuDebConTextPos(0,1,6);
    sprintf(conbuf,"NextBulletIndex=%d", NextBulletIndex);
    nuDebConCPuts(0, conbuf);


    nuDebConTextPos(0,1,21);
    sprintf(conbuf,"player_rotation=%5.1f", player_rotation);
    nuDebConCPuts(0, conbuf);

    nuDebConTextPos(0,1,22);
    sprintf(conbuf,"PlayerState=%d", player_state);
    nuDebConCPuts(0, conbuf);

    nuDebConTextPos(0,1,23);
    sprintf(conbuf,"plrX=%5.1f", player_x);
    nuDebConCPuts(0, conbuf);

    nuDebConTextPos(0,1,24);
    sprintf(conbuf,"plrY=%5.1f", player_y);
    nuDebConCPuts(0, conbuf);

    nuDebConTextPos(0,1,25);
    sprintf(conbuf,"camR=%5.1f", camera_rotation);
    nuDebConCPuts(0, conbuf);



    nuDebConTextPos(0,1,26);
    sprintf(conbuf,"camX=%5.1f", camera_x);
    nuDebConCPuts(0, conbuf);

    nuDebConTextPos(0,1,27);
    sprintf(conbuf,"camY=%5.1f", camera_y);
    nuDebConCPuts(0, conbuf);
  }
  else
  {
    nuDebConTextPos(0,9,24);
    nuDebConCPuts(0, "Controller 1 not connected; please restart");
  }

    
  /* Display characters on the frame buffer */
  //nuDebConDisp(NU_SC_SWAPBUFFER);

  /* Switch display list buffers */
  gfx_gtask_no ^= 1;
}

float
atan2bodyf(float y,float x)
{ 
    float arg,ys,old;
    float power, term, sum, z;
    int i;

    if ( y == 0.0f )
      return 0.0f;

    if ( x == 0.0f )
      return (y > 0.0f)? (float)M_PI_2 : -(float)M_PI_2;

    arg = y / x;

    if ( arg == 1.0f )
      return (y > 0.0f)? (float)M_PI_4 : -3.0f * (float)M_PI_4;
 
    if ( arg == -1.0f )
      return (x > 0.0f) ? -(float)M_PI_4 :  3.0f * (float)M_PI_4;

    if ( arg > 1.0f || arg < -1.0f) {
  sum = atan2bodyf(x, y);
  if( x > 0.0f )
    return (float)M_PI_2 - sum;
  else         
    return (y > 0.0f) ? (float)M_PI_2 - sum: -3.0f * (float)M_PI_2 -sum;
    }

    ys = arg * arg;
    old = 1.0f / (1.0f + ys);
    z = ys * old;
    sum = 1.0f;
    i = 4;
    power = z * 2.0f / 3.0f;
    while( i < NRECTAB-1) {
  term = power;
  sum += term;
  if ( term >= -TOL && term <= TOL) break;
  power *= ( z * (float)i * reciprocal_table_f[i+1] );
  i += 2;
    }
    sum *= arg * old;
    return ( x > 0.0f )? sum : ( y > 0.0f ) ? (float)M_PI + sum :-(float)M_PI + sum;
}

float
Atan2f(float y, float x)
{
    return atan2bodyf(y, x);
}

void updateGame00(void)
{  
  int i;
  float cosCamRot;
  float sinCamRot;
  float deltaX;
  float deltaY;
  float rotatedX;
  float rotatedY;
  float newX;
  float newY;
  float playerStickRot;
  int roll;
  OSTime newTime = OS_CYCLES_TO_USEC(osGetTime());
  float deltaSeconds;

  delta = (newTime - time);
  time = newTime;
  deltaSeconds = delta * 0.000001f;

  nuDebPerfMarkSet(0);

  /* Data reading of controller 1 */
  nuContDataGetEx(contdata,0);

  if (player_state == Move) {
    /* The reverse rotation by the A button */
    if(contdata[0].button & L_TRIG)
    {
        camera_rotation += CAMERA_TURN_SPEED;

        if (camera_rotation > (M_PI * 2.0f))
        {
          camera_rotation -= M_PI * 2.0f;
        } 
    }

    if(contdata[0].button & R_TRIG)
    {
        camera_rotation -= CAMERA_TURN_SPEED;

        if (camera_rotation < 0.f)
        {
          camera_rotation += M_PI * 2.0f;
        }
    }

    // Raw stick data (this should be adjusted for deadzones or emulator players on d-pads)
    cosCamRot = cosf(-camera_rotation);
    sinCamRot = sinf(-camera_rotation);
    deltaX = contdata->stick_x;
    deltaY = contdata->stick_y;
    player_facing_x = (deltaX * cosCamRot) + (deltaY * sinCamRot);
    player_facing_y = (-deltaX * sinCamRot) + (deltaY * cosCamRot);
    playerStickRot = Atan2f(player_facing_y, player_facing_x);

    // If we're pushing on the stick, update the player's rotation
    if ((fabs_d(contdata->stick_x) > 0.01f) || (fabs_d(contdata->stick_y) > 0.01f)) {
      if ((player_rotation < -(M_PI_2)) && (playerStickRot > (M_PI_2))) {
        player_rotation += M_PI * 2.f;
      } 
      if ((player_rotation > (M_PI_2)) && (playerStickRot < -(M_PI_2))) {
        player_rotation -= M_PI * 2.f;
      }

      player_rotation = lerp(player_rotation, playerStickRot, 0.342776562f);
    }

    newX = player_x + player_facing_x * PLAYER_MOVE_SPEED * deltaSeconds;
    newY = player_y + player_facing_y * PLAYER_MOVE_SPEED * deltaSeconds;

    if (contdata[0].trigger & A_BUTTON) {
      player_state = Jumping;
      player_t = 0.f;
      player_jump_x = player_x;
      player_jump_y = player_y;
      HIT_WALL_WHILE_JUMPING = 0;
    }

    for (i = 0; i < (sizeof(player_sword) / sizeof(Vtx)); i++) {
      player_sword[i].v.cn[0] = player_sword[(i + 1) % (sizeof(player_sword) / sizeof(Vtx))].v.cn[0];
    }
  } else if (player_state == Jumping) {
    if (HIT_WALL_WHILE_JUMPING == 0) {
      newX = player_jump_x + cosf(player_rotation) * player_t * target_distance;
      newY = player_jump_y + sinf(player_rotation) * player_t * target_distance;
    } else {
      newX = player_x;
      newY = player_y;
    }

    player_t += JUMP_SPEED_PER_TICK;
    if (player_t > 1.f) {
      player_state = Landed;
      player_t = 0.f;
    }

    roll = guRandom() % 2;
    for (i = 0; i < (sizeof(player_sword) / sizeof(Vtx)); i++) {
      player_sword[i].v.cn[0] = (roll == 0) ? 255 : 0;
      player_sword[i].v.cn[1] = (roll == 0) ? 200 : 0;
      player_sword[i].v.cn[2] = (roll == 0) ? 200 : 0;
    }

  } else if (player_state == Landed) {
    newX = player_x;
    newY = player_y;

    player_t += LAND_SPEED_PER_TICK;
    if (player_t > 1.0f) {
      player_state = Move;
    }
  } else if (player_state == Dead) {
    newX = player_x;
    newY = player_y;
    player_t = MIN(player_t + DEATH_FALL_PER_TICK, 1.f);
  }

  newTileX = (int)(newX * INV_TILE_SIZE);
  newTileY = (int)(newY * INV_TILE_SIZE);

  // step x
  if ((newTileX < (MAP_SIZE * TILE_SIZE)) && (newTileX >= 0) && (IS_TILE_BLOCKED(newTileX, (int)(player_y * INV_TILE_SIZE)))) {
    if (player_state == Jumping && (IS_TILE_BLOCKED(newTileX, (int)(player_y * INV_TILE_SIZE)) != LOW_WALL_TILE)) {
      HIT_WALL_WHILE_JUMPING = 1;
    }

    newX = player_x;
  }
  player_x = MIN(MAP_SIZE * TILE_SIZE, MAX(0, newX));

  // step y
  if ((newTileY < (MAP_SIZE * TILE_SIZE)) && (newTileY >= 0) && (IS_TILE_BLOCKED((int)(player_x * INV_TILE_SIZE), newTileY))) {
  
    if (player_state == Jumping && (IS_TILE_BLOCKED((int)(player_x * INV_TILE_SIZE), newTileY) != LOW_WALL_TILE)) {
      HIT_WALL_WHILE_JUMPING = 1;
    }

    newY = player_y;
  }
  player_y = MIN(MAP_SIZE * TILE_SIZE, MAX(0, newY));;

  // Lerp the camera
  camera_x = lerp(camera_x, player_x, CAMERA_LERP);
  camera_y = lerp(camera_y, player_y, CAMERA_LERP);
  
  // Update bullets
  for (i = 0; i < BULLET_COUNT; i++) {
    float dxSq = 9999.f;
    float dySq = 9999.f;

    if (BulletStates[i] == 0) {
      continue;
    }

    if ((BulletPositions[i].x < 0) || (BulletPositions[i].x > MAP_SIZE * TILE_SIZE)
        || (BulletPositions[i].y < 0) || (BulletPositions[i].y > MAP_SIZE * TILE_SIZE)) {
      BulletStates[i] = 0;
    }

    if (IS_TILE_BLOCKED((int)(BulletPositions[i].x * INV_TILE_SIZE), (int)(BulletPositions[i].y * INV_TILE_SIZE))) {
      BulletStates[i] = 0;
      continue;
    }

    BulletPositions[i].x += BulletVelocities[i].x * deltaSeconds;
    BulletPositions[i].y += BulletVelocities[i].y * deltaSeconds;

    // If the player's in the air or dead, there's no point in checking death
    if ((player_state == Jumping) || (player_state == Dead)) {
      continue;
    }

    dxSq = player_x - BulletPositions[i].x;
    dxSq = dxSq * dxSq;
    if (dxSq > BulletRadiiSquared[i]) {
      continue;
    }

    dySq = player_y - BulletPositions[i].y;
    dySq = dySq * dySq;
    if (dySq > BulletRadiiSquared[i]) {
      continue;
    }

    if ((dySq + dxSq) >= PLAYER_HIT_RADIUS_SQ) {
      continue;
    }

    // If we've reached this line, the bullet has hit the player
    player_state = Dead;
    player_t = 0;
  }

  // Update emitters position/velocity/life
  for (i = 0; i < EMITTER_COUNT; i++) {
    float dxSq = 9999.f;
    float dySq = 9999.f;

    if (EmitterStates[i] == EMITTER_DEAD) {
      continue;
    }

    EmitterPositions[i].x += EmitterVelocities[i].x * deltaSeconds;
    EmitterPositions[i].y += EmitterVelocities[i].y * deltaSeconds;

    // Kill an emitter if the player landed on it
    if (player_state != Landed) {
      continue;
    }

    dxSq = player_x - EmitterPositions[i].x;
    dxSq = dxSq * dxSq;
    if (dxSq > EMITTER_RADIUS_SQ) {
      continue;
    }

    dySq = player_y - EmitterPositions[i].y;
    dySq = dySq * dySq;
    if (dySq > EMITTER_RADIUS_SQ) {
      continue;
    }

    if ((dySq + dxSq) >= EMITTER_RADIUS_SQ) {
      continue;
    }

    EmitterStates[i] = EMITTER_DEAD;
  }

  // Update aim emitters
  for (i = 0; i < AIM_EMITTER_COUNT; i++) {
    int newBulletIndex;
    float theta;


    if (AimEmitters[i].emitterIndex == -1) {
      continue;
    }

    if ((EmitterStates[AimEmitters[i].emitterIndex]) == EMITTER_DEAD) {
      AimEmitters[i].emitterIndex = -1;
      continue;
    }

    AimEmitters[i].t += deltaSeconds;
    if (AimEmitters[i].t < AimEmitters[i].period) {
      continue;
    }

    // If we've made it here, fire
    AimEmitters[i].t = 0;

    // Skip if there are no available bullets
    newBulletIndex = consumeNextBullet();
    if (newBulletIndex == -1) {
      continue;
    }
    BulletStates[newBulletIndex] = 1;
    BulletPositions[newBulletIndex].x = EmitterPositions[AimEmitters[i].emitterIndex].x;
    BulletPositions[newBulletIndex].y = EmitterPositions[AimEmitters[i].emitterIndex].y;
    theta = Atan2f(player_y - BulletPositions[newBulletIndex].y, player_x - BulletPositions[newBulletIndex].x);
    BulletVelocities[newBulletIndex].x = 5.831332f * cosf(theta);
    BulletVelocities[newBulletIndex].y = 5.831332f * sinf(theta);

  }

  nuDebPerfMarkSet(1);
  
}

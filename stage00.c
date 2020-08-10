#include <assert.h>
#include <nusys.h>
#include "main.h"
#include "graphic.h"
#include <gu.h>
#include "os_time.h"

#include "PlayerData.h"
#include "EntityData.h"
#include "RoomData.h"
#include "game_math.h"
#include "letters.h"
#include "floordata.h"

#define CAMERA_MOVE_SPEED 0.01726f
#define CAMERA_TURN_SPEED 0.03826f
#define CAMERA_DISTANCE 13.23f
#define CAMERA_HEIGHT 50.0f
#define CAMERA_LERP 0.13f
#define CAMERA_FOV 30.0f

#define WARP_IN_TIME_IN_SECONDS 2.34222f

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
float player_sword_angle;
u8 player_bullets_collected;

static float camera_rotation;
static float player_rotation;

static float camera_x; /* The display position-X */
static float camera_y; /* The display position-Y */

static int newTileX;
static int newTileY;

static OSTime time;
static OSTime delta;

float deltaSeconds;

float test;

char testStringBuf[64];

int isWarping;
int isWarpingOut;
float warpDelta;

static Vtx jump_target_geom[] =  {
  {   0,  2, 0, 0, 0, 0, 0xff, 0, 0, 0xff },
  {   1,  1, 0, 0, 0, 0, 0xff, 0, 0, 0xff },
  {   2,  0, 0, 0, 0, 0, 0xff, 0, 0, 0xff },
  {   1,  -1, 0, 0, 0, 0, 0xff, 0, 0, 0xff },
  {   0,  -2, 0, 0, 0, 0, 0xff, 0, 0, 0xff },
  {   -1,  -1, 0, 0, 0, 0, 0xff, 0, 0, 0xff },
  {   -2,  0, 0, 0, 0, 0, 0xff, 0, 0, 0xff },
  {   -1,  1, 0, 0, 0, 0, 0xff, 0, 0, 0xff },
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
{ 12, 35, 101, 0, 0, 0, 201, 149, 113, 255 },
{ 17, 42, 128, 0, 0, 0, 153, 112, 84, 255 },
{ 22, 36, 156, 0, 0, 0, 153, 112, 84, 255 },
{ -1, 64, 75, 0, 0, 0, 67, 172, 237, 255 },
{ -13, 66, 138, 0, 0, 0, 52, 125, 201, 255 },
{ 11, 57, 178, 0, 0, 0, 19, 170, 255, 255 },
{ 12, -33, 101, 0, 0, 0, 201, 149, 113, 255 },
{ 17, -40, 128, 0, 0, 0, 153, 112, 84, 255 },
{ 22, -34, 156, 0, 0, 0, 153, 112, 84, 255 },
{ -10, 1, 112, 0, 0, 0, 255, 255, 255, 255 },
{ 44, 1, 163, 0, 0, 0, 153, 112, 84, 255 },
{ 37, 1, 94, 0, 0, 0, 201, 149, 113, 255 },
{ 46, 1, 128, 0, 0, 0, 201, 149, 113, 255 },
{ -1, -62, 75, 0, 0, 0, 67, 172, 237, 255 },
{ -13, -64, 138, 0, 0, 0, 52, 125, 201, 255 },
{ 11, -55, 178, 0, 0, 0, 19, 170, 255, 255 },
{ 41, 1, 188, 0, 0, 0, 19, 170, 255, 255 },
{ 27, 39, 158, 0, 0, 0, 0, 0, 0, 255 },
{ 39, 17, 158, 0, 0, 0, 0, 0, 0, 255 },
{ 34, 32, 133, 0, 0, 0, 51, 153, 27, 255 },
{ 46, 10, 133, 0, 0, 0, 255, 248, 253, 255 },
{ 30, -38, 158, 0, 0, 0, 0, 0, 0, 255 },
{ 41, -16, 158, 0, 0, 0, 0, 0, 0, 255 },
{ 32, -29, 133, 0, 0, 0, 51, 153, 27, 255 },
{ 43, -6, 133, 0, 0, 0, 255, 248, 253, 255 },
{ -71, 57, 87, 0, 0, 0, 46, 35, 153, 255 },
{ -43, 34, 169, 0, 0, 0, 67, 172, 237, 255 },
{ -71, -45, 87, 0, 0, 0, 46, 35, 153, 255 },
{ -43, -32, 169, 0, 0, 0, 67, 172, 237, 255 },
{ -20, 1, 198, 0, 0, 0, 67, 172, 237, 255 },
{ -73, 1, 107, 0, 0, 0, 46, 35, 153, 255 },
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

static u8 trail_geo_index;
static Vtx trail_geo[] = {
{ 803, -22, -12, 0, 0, 0, 0, 198, 215, 150 },
{ 803, -22, 12, 0, 0, 0, 215, 215, 215, 150 },
{ 1018, -25, -11, 0, 0, 0, 0, 198, 215, 150 },
{ 803, -22, -12, 0, 0, 0, 215, 215, 215, 150 },
{ 803, -22, 12, 0, 0, 0, 40, 198, 215, 150 },
{ 1018, -25, -11, 0, 0, 0, 215, 215, 215, 150 },
{ 803, -22, -12, 0, 0, 0, 0, 198, 215, 150 },
{ 8003, -22, 12, 0, 0, 0, 215, 215, 215, 150 },
{ 1018, -25, -11, 0, 0, 0, 0, 198, 215, 150 },
{ 803, -22, -12, 0, 0, 0, 215, 215, 215, 150 },
{ 803, -22, 12, 0, 0, 0, 40, 198, 215, 150 },
{ 1018, -25, -11, 0, 0, 0, 215, 215, 215, 150 },
{ 803, -22, -12, 0, 0, 0, 0, 198, 215, 150 },
{ 803, -22, 12, 0, 0, 0, 215, 215, 215, 150 },
{ 1018, -25, -11, 0, 0, 0, 0, 198, 215, 150 },
{ 803, -22, -12, 0, 0, 0, 215, 215, 215, 150 },
{ 803, -22, 12, 0, 0, 0, 40, 198, 215, 150 },
{ 1018, -25, -11, 0, 0, 0, 215, 215, 215, 150 },
{ 803, -22, -12, 0, 0, 0, 0, 198, 215, 150 },
{ 803, -22, 12, 0, 0, 0, 215, 215, 215, 150 },
{ 1018, -25, -11, 0, 0, 0, 0, 198, 215, 150 },
{ 803, -22, -12, 0, 0, 0, 215, 215, 215, 150 },
{ 803, -22, 12, 0, 0, 0, 40, 198, 215, 150 },
{ 118, -25, -11, 0, 0, 0, 215, 215, 215, 150 },
{ 803, -22, -12, 0, 0, 0, 0, 198, 215, 150 },
{ 803, -22, 12, 0, 0, 0, 215, 215, 215, 150 },
{ 1018, -25, -11, 0, 0, 0, 0, 198, 215, 150 },
{ 803, -22, -12, 0, 0, 0, 215, 215, 215, 150 },
{ 803, -22, 12, 0, 0, 0, 40, 198, 215, 150 },
{ 1018, -25, -11, 0, 0, 0, 215, 215, 215, 150 },
{ 803, -22, 12, 0, 0, 0, 40, 198, 215, 150 },
{ 1018, -25, -11, 0, 0, 0, 215, 215, 215, 150 },
};

static Vtx player_land_effect[] = {
{ 0, 54, -20, 0, 0, 0, 38, 148, 255, 255 },
{ -38, 38, -20, 0, 0, 0, 221, 240, 255, 255 },
{ -54, 0, -20, 0, 0, 0, 38, 148, 255, 255 },
{ -38, -38, -20, 0, 0, 0, 221, 240, 255, 255 },
{ 0, -54, -20, 0, 0, 0, 38, 148, 255, 255 },
{ 38, -38, -20, 0, 0, 0, 221, 240, 255, 255 },
{ 54, 0, -20, 0, 0, 0, 38, 148, 255, 255 },
{ 38, 38, -20, 0, 0, 0, 221, 240, 255, 255 },
{ 0, 112, -9, 0, 0, 0, 111, 237, 210, 255 },
{ -79, 79, -9, 0, 0, 0, 221, 240, 255, 255 },
{ -112, 0, -9, 0, 0, 0, 111, 237, 210, 255 },
{ -79, -79, -9, 0, 0, 0, 221, 240, 255, 255 },
{ 0, -112, -9, 0, 0, 0, 111, 237, 210, 255 },
{ 79, -79, -9, 0, 0, 0, 221, 240, 255, 255 },
{ 112, 0, -9, 0, 0, 0, 111, 237, 210, 255 },
{ 79, 79, -9, 0, 0, 0, 221, 240, 255, 255 },
{ 0, 105, -27, 0, 0, 0, 111, 237, 210, 255 },
{ -74, 74, -27, 0, 0, 0, 221, 240, 255, 255 },
{ -105, 0, -27, 0, 0, 0, 111, 237, 210, 255 },
{ -74, -74, -27, 0, 0, 0, 221, 240, 255, 255 },
{ 0, -105, -27, 0, 0, 0, 111, 237, 210, 255 },
{ 74, -74, -27, 0, 0, 0, 221, 240, 255, 255 },
{ 105, 0, -27, 0, 0, 0, 111, 237, 210, 255 },
{ 74, 74, -27, 0, 0, 0, 221, 240, 255, 255 },
};

static Vtx thing_geom[] = {
{ -108, -96, -113, 0, 0, 0, 33, 15, 84, 255 },
{ -108, 120, -113, 0, 0, 0, 33, 15, 84, 255 },
{ -108, 120, 54, 0, 0, 0, 33, 15, 84, 255 },
{ 108, -96, -113, 0, 0, 0, 33, 15, 84, 255 },
{ 108, 120, -113, 0, 0, 0, 33, 15, 84, 255 },
{ 108, 120, 54, 0, 0, 0, 33, 15, 84, 255 },
{ -82, 11, 146, 0, 0, 0, 33, 15, 84, 255 },
{ 0, 93, 146, 0, 0, 0, 33, 15, 84, 255 },
{ 82, -70, 146, 0, 0, 0, 33, 15, 84, 255 },
{ 82, 11, 146, 0, 0, 0, 33, 15, 84, 255 },
{ 0, 160, -145, 0, 0, 0, 33, 15, 84, 255 },
{ 0, 160, 85, 0, 0, 0, 33, 15, 84, 255 },
{ 0, -137, 4, 0, 0, 0, 33, 15, 84, 255 },
{ 0, -137, 85, 0, 0, 0, 36, 20, 84, 255 },
{ -148, 11, -145, 0, 0, 0, 33, 15, 84, 255 },
{ -148, 11, 85, 0, 0, 0, 33, 15, 84, 255 },
{ 148, 11, -145, 0, 0, 0, 33, 15, 84, 255 },
{ 148, 11, 85, 0, 0, 0, 33, 15, 84, 255 },
{ 0, 11, 168, 0, 0, 0, 33, 15, 84, 255 },
{ 0, 11, -145, 0, 0, 0, 33, 15, 84, 255 },
{ -103, -96, 12, 0, 0, 0, 33, 15, 84, 255 },
{ -65, -111, 65, 0, 0, 0, 255, 239, 0, 255 },
{ -92, -85, 94, 0, 0, 0, 255, 239, 0, 255 },
{ -117, -57, 65, 0, 0, 0, 255, 239, 0, 255 },
{ 92, -85, 94, 0, 0, 0, 255, 239, 0, 255 },
{ 65, -111, 65, 0, 0, 0, 255, 239, 0, 255 },
{ 103, -96, 12, 0, 0, 0, 33, 15, 84, 255 },
{ 117, -57, 65, 0, 0, 0, 255, 239, 0, 255 },
{ -87, -70, 146, 0, 0, 0, 59, 49, 56, 255 },
};

/* The initialization of stage 0 */
void initStage00(int floorNumber)
{
  int i;
  int numberOfGeneratedRooms;
  GeneratedRoom rooms[MAX_NUMBER_OF_ROOMS_PER_FLOOR];

  target_distance = DEFAULT_TARGET_DISTANCE;
  player_state = Move;
  player_t = 0.f;
  HIT_WALL_WHILE_JUMPING = 0;
  player_sword_angle = 0.f;
  player_bullets_collected = 50;

  trail_geo_index = 0;

  camera_x = 0.0f;
  camera_y = 0.0f;

  time = OS_CYCLES_TO_USEC(osGetTime());
  delta = 0;
  deltaSeconds = 0.f;

  camera_rotation = M_PI;
  player_rotation = -M_PI_2;

  resetTextRequests();

  // sprintf(testStringBuf, "curr. floor is %d", currentFloor);
  // getTextRequest(0)->enable = 1;
  // getTextRequest(0)->text = testStringBuf;
  // getTextRequest(0)->x = 16;
  // getTextRequest(0)->y = 16;
  // getTextRequest(0)->cutoff = 0;
  // getTextRequest(0)->typewriterTick = 0;

  initializeEntityData();

  // TODO: create a variable for this to change
  numberOfGeneratedRooms =initMap(rooms, &(roomSeeds[floorNumber]), floorNumber);
  initEnemiesForMap(rooms);
  updateMapFromInfo();

  if (previousFloor == NO_PREVIOUS_FLOOR) {
    // If we have no previous floor, let's simply place ourself in the lobby
    player_x = (rooms[0].x + (rooms[0].width / 2)) * TILE_SIZE;
    player_y = (rooms[0].y + (rooms[0].height / 2)) * TILE_SIZE;
  } else {
    // Place us with the corresponding staircase
    for (i = 0; i < numberOfGeneratedRooms; i++) {
      if (rooms[i].type == StaircaseRoom) {
        if (exitMap[currentFloor][rooms[i].stairsDirectionIndex] == previousFloor) {
          player_x = (rooms[i].x + (rooms[i].width / 2) + 2) * TILE_SIZE;
          player_y = (rooms[i].y + (rooms[i].height / 2)) * TILE_SIZE;
        }
      }
    }
  }

  camera_x = player_x;
  camera_y = player_y;

  isWarping = 1;
  isWarpingOut = 0;
  warpDelta = 0;
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
  int i;
  gSPVertex(glistp++,&(thing_geom[0]), 29, 0);
  for (i = 0; i < 21; i++) {
    int ir;
    int rolls[6];
    for (ir = 0; ir < 6; ir++) {
      rolls[ir] = guRandom() % 29;
    }
    gSP2Triangles(glistp++, rolls[0], rolls[1], rolls[2], 0, rolls[3], rolls[4], rolls[5], 0);
  }
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
gSP2Triangles(glistp++, 14, 28, 27, 0, 25, 26, 4, 0);
gSP2Triangles(glistp++, 13, 14, 27, 0, 4, 3, 25, 0);
gSP2Triangles(glistp++, 15, 29, 28, 0, 27, 28, 30, 0);
gSP2Triangles(glistp++, 28, 29, 30, 0, 26, 30, 29, 0);
gSP2Triangles(glistp++, 25, 30, 26, 0, 14, 15, 28, 0);
gSP2Triangles(glistp++, 5, 4, 26, 0, 29, 5, 26, 0);
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
  int i;
  gSPVertex(glistp++,&(player_sword[0]), 21, 0);
  if (player_state == Holding && (player_bullets_collected < JUMP_COST )) {
    for (i = 0; i < 21; i++) {
      int ir;
      int rolls[6];
      for (ir = 0; ir < 6; ir++) {
        rolls[ir] = guRandom() % 26;
      }
      gSP2Triangles(glistp++, rolls[0], rolls[1], rolls[2], 0, rolls[3], rolls[4], rolls[5], 0);
    }
  } else {
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
    gSP2Triangles(glistp++, 10, 17, 9, 0, 17, 20, 18, 0);
  }
}

void addLandEffectDisplayList() {
  gSPVertex(glistp++,&(player_land_effect[0]), 24, 0);
  gSP2Triangles(glistp++, 2, 9, 10, 0, 7, 14, 15, 0);
  gSP2Triangles(glistp++, 5, 12, 13, 0, 3, 10, 11, 0);
  gSP2Triangles(glistp++, 1, 8, 9, 0, 0, 15, 8, 0);
  gSP2Triangles(glistp++, 6, 13, 14, 0, 4, 11, 12, 0);
  gSP2Triangles(glistp++, 13, 20, 21, 0, 11, 18, 19, 0);
  gSP2Triangles(glistp++, 9, 16, 17, 0, 8, 23, 16, 0);
  gSP2Triangles(glistp++, 14, 21, 22, 0, 12, 19, 20, 0);
  gSP2Triangles(glistp++, 10, 17, 18, 0, 15, 22, 23, 0);
  gSP2Triangles(glistp++, 2, 1, 9, 0, 7, 6, 14, 0);
  gSP2Triangles(glistp++, 5, 4, 12, 0, 3, 2, 10, 0);
  gSP2Triangles(glistp++, 1, 0, 8, 0, 0, 7, 15, 0);
  gSP2Triangles(glistp++, 6, 5, 13, 0, 4, 3, 11, 0);
  gSP2Triangles(glistp++, 13, 12, 20, 0, 11, 10, 18, 0);
  gSP2Triangles(glistp++, 9, 8, 16, 0, 8, 15, 23, 0);
  gSP2Triangles(glistp++, 14, 13, 21, 0, 12, 11, 19, 0);
  gSP2Triangles(glistp++, 10, 9, 17, 0, 15, 14, 22, 0);
}

void makeDL00(void)
{
  Dynamic* dynamicp;
  char conbuf[20]; 
  int i;
  int j;
  float ox, oy, oz; // for CPU transforming the sword trail
  int running = (fabs_d(player_facing_x) > 0.1f) || (fabs_d(player_facing_y) > 0.1f);
  float warpRatio = (warpDelta / WARP_IN_TIME_IN_SECONDS);

  /* Perspective normal value; I don't know what this does yet. */
  u16 perspNorm;

  nuDebPerfMarkSet(2);

  /* Specify the display list buffer */
  dynamicp = &gfx_dynamic[gfx_gtask_no];
  glistp = &gfx_glist[gfx_gtask_no][0];

  /* Initialize RCP */
  gfxRCPInit();

  /* Clear the frame and Z-buffer */
  gfxClearCfb();

  /* projection,modeling matrix set */
  guPerspective(&dynamicp->projection, &perspNorm, CAMERA_FOV, (float)SCREEN_WD/(float)SCREEN_HT, 10.0f, 100.0f, 1.0f);
  guLookAt(&dynamicp->viewing, camera_x, camera_y + CAMERA_DISTANCE, CAMERA_HEIGHT, camera_x, camera_y, 0.0f, 0.0f, 0.0f, 1.0f);

  gSPPerspNormalize(glistp++, perspNorm);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->projection)), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->viewing)), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++, G_CYC_1CYCLE);
  gDPSetRenderMode(glistp++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
  gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
  gDPPipelineMode(glistp++, G_PM_NPRIMITIVE);
  gSPSetGeometryMode(glistp++, G_ZBUFFER | G_CULL_BACK | G_SHADE | G_SHADING_SMOOTH);

  if (isWarping) {
    if (isWarpingOut) {
      gDPSetEnvColor(glistp++, 255 * (warpRatio), 255 * (warpRatio), 255 * (warpRatio), 255 * (warpRatio));
    } else {
      gDPSetEnvColor(glistp++, 255 * (1.f - warpRatio), 255 * (1.f - warpRatio), 255 * (1.f - warpRatio), 255 * (1.f - warpRatio));
    }
    gDPSetCombineLERP(glistp++, SHADE,    ENVIRONMENT, 0,     0, 0,    0,     0, 0,
                                SHADE,    ENVIRONMENT, SHADE,     0, 0,    0,     0, SHADE);
  }

  // Render Player
  guTranslate(&(dynamicp->playerTranslation), player_x, player_y, ((player_state == Jumping) ? (sinf(player_t * M_PI) * 6.2f) : 0.f));
  guRotate(&(dynamicp->playerRotation), player_rotation / M_PI * 180.f, 0.0f, 0.0f, 1.0f);
  if (player_state == Jumping) {
    guRotate(&(dynamicp->playerJumpRotation), sinf(player_t * M_PI * 2.f) * 75.2f, 0.0f, 1.0f, 0.0f);
  } else if (player_state == Move) {
    if (running) {
      guRotate(&(dynamicp->playerJumpRotation), sinf(time / 100000.f) * 5.f, 0.0f, 1.0f, 0.0f);
    } else {
      guRotate(&(dynamicp->playerJumpRotation), 5.f, 0.0f, 1.0f, 0.0f);
    }
  } else if (player_state == Holding) {
    guMtxIdent(&(dynamicp->playerJumpRotation));
  } else if (player_state == Landed) {
    guRotate(&(dynamicp->playerJumpRotation), 51.7f, 0.0f, 1.0f, 0.0f);
  } else if (player_state == Dead) {
    guRotate(&(dynamicp->playerJumpRotation), lerp(0.f, -90.f, (player_t)), 0.0f, 1.0f, 0.0f);
  }
  
  guScale(&(dynamicp->playerScale), 0.01, 0.01, 0.01);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->playerTranslation)), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->playerRotation)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->playerJumpRotation)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->playerScale)), G_MTX_PUSH | G_MTX_MODELVIEW);

  addPlayerDisplayList();

  nuDebPerfMarkSet(3);

  // Determine sword display list
  guMtxIdent(&dynamicp->swordTranslation);
  if (player_state == Move) {
    if (player_bullets_collected >= JUMP_COST) {
      guScale(&(dynamicp->swordScale), 0.9f, 0.9f, 0.8f);
    } else {
      guScale(&dynamicp->swordScale, 0.f, 0.f, 0.f);
    }
    if (running) {
      guRotate(&(dynamicp->swordRotationX), 5.f + (6.3f * sinf(time / 150000.f)), 0.0f, 1.0f, 0.0f);
      guRotate(&(dynamicp->swordRotationZ), 145.f + (6.3f * sinf(time / 200000.f)), 0.0f, 0.0f, 1.0f);
    } else {
      guRotate(&(dynamicp->swordRotationX), 5.f + (4.f * sinf(time / 400000.f)), 0.0f, 1.0f, 0.0f);
      guRotate(&(dynamicp->swordRotationZ), 135.f, 0.0f, 0.0f, 1.0f);
    }
  } else if (player_state == Holding) {
      float extra = 0.1f * sinf(time * 0.000004f);
      guTranslate(&dynamicp->swordTranslation, -100.f, 0.f, 0.f);
      guScale(&(dynamicp->swordScale), 1.8f + extra, 1.2152f + extra, 1.4f + extra);
      guMtxIdent(&(dynamicp->swordRotationX));
      guRotate(&(dynamicp->swordRotationZ), (player_sword_angle - player_rotation) / M_PI * 180, 0.0f, 0.0f, 1.0f);
  } else if (player_state == Jumping) {
    float cubedScale = cubic(player_t);
    guTranslate(&dynamicp->swordTranslation, 0.f, 0.f, cubedScale * 20.f);
    if (cubedScale < 0.3f) {
      float scale = lerp(1.0f, 1.7f, cubedScale / 0.3f);
      guScale(&(dynamicp->swordScale), scale, scale, scale);
      guRotate(&(dynamicp->swordRotationX), lerp(5.f, -180.f, cubedScale / 0.3f), 0.0f, 1.0f, 0.0f);
      guRotate(&(dynamicp->swordRotationZ), lerp(135.f, 90.f, cubedScale / 0.3f), 0.0f, 0.0f, 1.0f);
    } else {
      float scale = lerp(1.7f, 2.1f, ((cubedScale - 0.3f) / 0.7f));
      guScale(&(dynamicp->swordScale), scale, scale, scale);
      guRotate(&(dynamicp->swordRotationX), lerp(-180.f, 90.f, ((cubedScale - 0.3f) / 0.7f)), 0.0f, 1.0f, 0.0f);
      guRotate(&(dynamicp->swordRotationZ), lerp(90.f, 0.f, ((cubedScale - 0.3f) / 0.7f)), 0.0f, 0.0f, 1.0f);
    }
  } else if (player_state == Landed) {
    float scale = lerp(2.0f, 1.0f, player_t);
    guTranslate(&dynamicp->swordTranslation, 0.f, 0.f, (1 - scale) * 5.f);
    guScale(&(dynamicp->swordScale), scale, scale, scale);
    guRotate(&(dynamicp->swordRotationX), lerp(90.f, 5.f, player_t), 0.0f, 1.0f, 0.0f);
    guRotate(&(dynamicp->swordRotationZ), lerp(0.f, 120.f, player_t), 0.0f, 0.0f, 1.0f);
  } else if (player_state == Dead) {
    guScale(&(dynamicp->swordScale), 0.5f, 0.5f, 0.5f);
    guRotate(&(dynamicp->swordRotationX), 5.f + (4.f * sinf(time / 400000.f)), 0.0f, 1.0f, 0.0f);
    guRotate(&(dynamicp->swordRotationZ), 135.f, 0.0f, 0.0f, 1.0f);
  }
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->swordTranslation)), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->swordRotationZ)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->swordRotationX)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->swordScale)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  addSwordDisplayList();


  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

  guTranslate(&(dynamicp->targetTranslation), target_distance, 0.0f, 0.f);
  guRotate(&(dynamicp->targetRotation), ((float)time) * 0.0005f,  0.f, 0.f, 1.f);

  if (player_state == Move && (player_bullets_collected >= JUMP_COST) && (!isWarping)) { 
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->targetTranslation)), G_MTX_PUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->targetRotation)), G_MTX_NOPUSH | G_MTX_MODELVIEW);

    gSPVertex(glistp++, &(jump_target_geom[0]), 8, 0);
    gSP2Triangles(glistp++, 0,3,1,0,3,6,4,0);
    //gSP2Triangles(glistp++, 0 + 1,3 + 1,1 + 1,0,3 + 1,6 + 1,4 + 1,0);
    //gSP2Triangles(glistp++, 0 + 2,3 + 2,1 + 2,0,3 + 2, 0,4 + 2,0);
    gSP1Triangle(glistp++, 0 + 2,3 + 2,1 + 2,0);

    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
  }

  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

  guMtxXFML(&(dynamicp->swordScale), 0.f , 0.f, 0.f, &ox, &oy, &oz);
  guMtxXFML(&(dynamicp->swordRotationX), ox, oy, oz, &ox, &oy, &oz);
  guMtxXFML(&(dynamicp->swordRotationZ), ox, oy, oz, &ox, &oy, &oz);
  guMtxXFML(&(dynamicp->swordTranslation), ox, oy, oz, &ox, &oy, &oz);
  guMtxXFML(&(dynamicp->playerScale), ox, oy, oz, &ox, &oy, &oz);
  guMtxXFML(&(dynamicp->playerJumpRotation), ox, oy, oz, &ox, &oy, &oz);
  guMtxXFML(&(dynamicp->playerRotation), ox, oy, oz, &ox, &oy, &oz);
  guMtxXFML((&(dynamicp->playerTranslation)), ox, oy, oz, &ox, &oy, &oz);
  trail_geo[trail_geo_index + 0].v.ob[0] = (short)ox;
  trail_geo[trail_geo_index + 0].v.ob[1] = (short)oy;
  trail_geo[trail_geo_index + 0].v.ob[2] = (short)oz;
  guMtxXFML(&(dynamicp->swordScale),(player_state == Jumping) ? (400.f * player_t) : 320.f , 0.f, 0.f, &ox, &oy, &oz);
  guMtxXFML(&(dynamicp->swordRotationX), ox, oy, oz, &ox, &oy, &oz);
  guMtxXFML(&(dynamicp->swordRotationZ), ox, oy, oz, &ox, &oy, &oz);
  guMtxXFML(&(dynamicp->swordTranslation), ox, oy, oz, &ox, &oy, &oz);
  guMtxXFML(&(dynamicp->playerScale), ox, oy, oz, &ox, &oy, &oz);
  guMtxXFML(&(dynamicp->playerJumpRotation), ox, oy, oz, &ox, &oy, &oz);
  guMtxXFML(&(dynamicp->playerRotation), ox, oy, oz, &ox, &oy, &oz);
  guMtxXFML((&(dynamicp->playerTranslation)), ox, oy, oz, &ox, &oy, &oz);
  trail_geo[trail_geo_index + 1].v.ob[0] = (short)ox;
  trail_geo[trail_geo_index + 1].v.ob[1] = (short)oy;
  trail_geo[trail_geo_index + 1].v.ob[2] = (short)oz;

  gDPPipeSync(glistp++);

  // Render Bullets
  renderBullets(player_x, player_y);

  // Render emitters
  renderEmitters(player_x, player_y, &dynamicp->playerScale);

  renderBoss(dynamicp);

  nuDebPerfMarkSet(4);

  if (isWarping) {
    gDPSetCombineLERP(glistp++, SHADE,    ENVIRONMENT, 0,     0, 0,    0,     0, 0,
                                SHADE,    ENVIRONMENT, SHADE,     0, 0,    0,     0, SHADE);
    
    if (isWarpingOut) {
      renderMapTiles(camera_x, camera_y, camera_rotation, cubic((warpDelta / WARP_IN_TIME_IN_SECONDS)) * 30.f);
      gDPSetEnvColor(glistp++, 255 * (warpRatio), 255 * (warpRatio), 255 * (warpRatio), 255 * (warpRatio));
    } else {
      renderMapTiles(camera_x, camera_y, camera_rotation, cubic(1.f - (warpDelta / WARP_IN_TIME_IN_SECONDS)) * 30.f);
      gDPSetEnvColor(glistp++, 255 * (1.f - warpRatio), 255 * (1.f - warpRatio), 255 * (1.f - warpRatio), 255 * (1.f - warpRatio));
    }
  } else {
    renderMapTiles(camera_x, camera_y, camera_rotation, 0.f);
  }

  if (player_state == Jumping || player_state == Landed || player_state == Holding) {
    gSPVertex(glistp++, &(trail_geo[0]), 32, 0);
    for (i = 0; i < 32; i += 2) {
      int i1 = (trail_geo_index + i - 0 + (sizeof(trail_geo) / sizeof(trail_geo[0]))) % (sizeof(trail_geo) / sizeof(trail_geo[0]));
      int i2 = (trail_geo_index + i - 1 + (sizeof(trail_geo) / sizeof(trail_geo[0]))) % (sizeof(trail_geo) / sizeof(trail_geo[0]));
      int i3 = (trail_geo_index + i - 2 + (sizeof(trail_geo) / sizeof(trail_geo[0]))) % (sizeof(trail_geo) / sizeof(trail_geo[0]));
      int i4 = (trail_geo_index + i - 3 + (sizeof(trail_geo) / sizeof(trail_geo[0]))) % (sizeof(trail_geo) / sizeof(trail_geo[0]));
      gSP2Triangles(glistp++, i1, i2, i3, 0, i1, i2, i4, 0);
    }
    trail_geo_index = (trail_geo_index + 2) % (sizeof(trail_geo) / sizeof(trail_geo[0]));

    if (player_state == Landed) {
      float hump = sinf((player_t) * M_PI);
      guRotate(&dynamicp->landEffectRotation, time, 0.f, 0.f, 1.f);
      guScale(&dynamicp->landEffectScale, cubic(hump + 0.5f) * 0.014523f, cubic(hump + 0.5f) * 0.014523f, (hump + 0.7f) * 0.02f);
      gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->playerTranslation)), G_MTX_PUSH | G_MTX_MODELVIEW);
      gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->landEffectScale)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
      gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->landEffectRotation)), G_MTX_NOPUSH | G_MTX_MODELVIEW);

      addLandEffectDisplayList();

      gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
    }
  }

  nuDebPerfMarkSet(5);

  drawTextRequests();

  gDPPipeSync(glistp++);

  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);

  assert((glistp - gfx_glist[gfx_gtask_no]) < GFX_GLIST_LEN);

  /* Activate the task and 
     switch display buffers */
  nuGfxTaskStart(&gfx_glist[gfx_gtask_no][0],
		 (s32)(glistp - gfx_glist[gfx_gtask_no]) * sizeof (Gfx),
		 NU_GFX_UCODE_F3DEX2_REJ , NU_SC_NOSWAPBUFFER);

  //nuDebTaskPerfBar1(1, 200, NU_SC_SWAPBUFFER);

  // if(contPattern & 0x1)
  // {
  //   nuDebConTextPos(0,1,3);
  //   sprintf(conbuf,"DL=%d / %d", (int)(glistp - gfx_glist[gfx_gtask_no]),  GFX_GLIST_LEN);
  //   nuDebConCPuts(0, conbuf);

  //   nuDebConTextPos(0,1,4);
  //   sprintf(conbuf,"time=%llu", time);
  //   nuDebConCPuts(0, conbuf);

  //   nuDebConTextPos(0,1,5);
  //   sprintf(conbuf,"delta=%llu", delta);
  //   nuDebConCPuts(0, conbuf);

  //   nuDebConTextPos(0,1,6);
  //   sprintf(conbuf,"deltaSeconds=%5.2f", deltaSeconds);
  //   nuDebConCPuts(0, conbuf);

  //   nuDebConTextPos(0,1,8);
  //   sprintf(conbuf,"warpDelta=%5.2f", warpDelta);
  //   nuDebConCPuts(0, conbuf);

  //   nuDebConTextPos(0,1,9);
  //   sprintf(conbuf,"isWarping=%d", isWarping);
  //   nuDebConCPuts(0, conbuf);

  //   nuDebConTextPos(0,1,10);
  //   sprintf(conbuf,"deltaSeconds=%5.2f", deltaSeconds);
  //   nuDebConCPuts(0, conbuf);


  //   nuDebConTextPos(0,1,21);
  //   sprintf(conbuf,"points=%d", player_bullets_collected);
  //   nuDebConCPuts(0, conbuf);

  //   nuDebConTextPos(0,1,22);
  //   sprintf(conbuf,"PlayerState=%d", player_state);
  //   nuDebConCPuts(0, conbuf);

  //   nuDebConTextPos(0,1,23);
  //   sprintf(conbuf,"plrX=%5.1f", player_x);
  //   nuDebConCPuts(0, conbuf);

  //   nuDebConTextPos(0,1,24);
  //   sprintf(conbuf,"plrY=%5.1f", player_y);
  //   nuDebConCPuts(0, conbuf);

  //   nuDebConTextPos(0,1,25);
  //   sprintf(conbuf,"sword_rot=%5.1f", player_sword_angle);
  //   nuDebConCPuts(0, conbuf);



  //   nuDebConTextPos(0,1,26);
  //   sprintf(conbuf,"camX=%5.1f", camera_x);
  //   nuDebConCPuts(0, conbuf);

  //   nuDebConTextPos(0,1,27);
  //   sprintf(conbuf,"camY=%5.1f", camera_y);
  //   nuDebConCPuts(0, conbuf);
  // }
  // else
  // {
  //   nuDebConTextPos(0,9,24);
  //   nuDebConCPuts(0, "Controller 1 not connected; please restart");
  // }

    
  /* Display characters on the frame buffer */
  nuDebConDisp(NU_SC_SWAPBUFFER);

  /* Switch display list buffers */
  gfx_gtask_no ^= 1;
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

  float stickX = 0;
  float stickY = 0;

  delta = (newTime - time);
  time = newTime;
  deltaSeconds = delta * 0.000001f;

  nuDebPerfMarkSet(0);

  tickTextRequests(deltaSeconds);

  /* Data reading of controller 1 */
  nuContDataGetEx(contdata,0);

  if ((contdata[0].trigger & START_BUTTON)) {
    resetStageFlag = 1;
    nextRoomRequest = -1;
    return;
  }

  if ((player_state == Move) || (player_state == Holding)) {
    if (contdata[0].button & (U_JPAD | D_JPAD | L_JPAD | R_JPAD)) {
      if (contdata[0].button & L_JPAD) {
        stickX = -1.f;
      } else if (contdata[0].button & R_JPAD) {
        stickX = 1.f;
      }

      if (contdata[0].button & D_JPAD) {
        stickY = -1.f;
      } else if (contdata[0].button & U_JPAD) {
        stickY = 1.f;
      }
    } else {
      stickX = MAX(-61.f, MIN(61.f, (contdata->stick_x))) / 61.f;
      stickY = MAX(-63.f, MIN(63.f, (contdata->stick_y))) / 63.f;
    }

    if (contdata[0].button & B_BUTTON) {
      if (player_state == Move) {
        player_sword_angle = player_rotation;
      }
      player_state = Holding;

      roll = guRandom() % 2;
      for (i = 0; i < (sizeof(player_sword) / sizeof(Vtx)); i++) {
        player_sword[i].v.cn[0] = (roll == 0) ? 180  : 255;
        player_sword[i].v.cn[1] = (roll == 0) ? 180  : 255;
        player_sword[i].v.cn[2] = 255;
      }
    } else if (player_state == Holding) {
      player_state = Move;
    }

    // If we're warping, don't allow motion
    if (isWarping) {
      stickX = 0.f;
      stickY = 0.f;
    }

    cosCamRot = cosf(-camera_rotation);
    sinCamRot = sinf(-camera_rotation);
    deltaX = stickX * 127;
    deltaY = stickY * 127;
    player_facing_x = (deltaX * cosCamRot) + (deltaY * sinCamRot);
    player_facing_y = (-deltaX * sinCamRot) + (deltaY * cosCamRot);
    playerStickRot = nu_atan2(player_facing_y, player_facing_x);

    // If we're pushing on the stick, update the player's rotation
    if ((fabs_d(stickX) > 0.01f) || (fabs_d(stickY) > 0.01f)) {
      if ((player_rotation < -(M_PI_2)) && (playerStickRot > (M_PI_2))) {
        player_rotation += M_PI * 2.f;
      } 
      if ((player_rotation > (M_PI_2)) && (playerStickRot < -(M_PI_2))) {
        player_rotation -= M_PI * 2.f;
      }

      player_rotation = lerp(player_rotation, playerStickRot, 0.342776562f);

      if (player_state == Holding) {
        // We need to keep the sword angle in the right range of the player angle
        if (fabs_d(player_sword_angle - player_rotation) > M_PI * 2.f) {
          if (player_sword_angle > player_rotation) {
            player_sword_angle -= M_PI * 2.f;
          } else {
            player_sword_angle += M_PI * 2.f;
          }
        }

        if (player_sword_angle > M_PI) {
          player_sword_angle -= M_PI * 2.f;
        } else if (player_sword_angle < (-M_PI)) {
          player_sword_angle += M_PI * 2.f;
        }

        if (player_sword_angle > player_rotation) {
          player_sword_angle = lerp( player_sword_angle, player_rotation + M_PI, 0.092553f);
        } else {
          player_sword_angle = lerp( player_sword_angle, player_rotation - M_PI, 0.092553f);
        }
      }
    }

    if (isWarping) {
      warpDelta += deltaSeconds;
      if (warpDelta > WARP_IN_TIME_IN_SECONDS) {
        isWarping = 0;

        if (isWarpingOut == 1) {
          isWarpingOut = 0;
          resetStageFlag = 1;
        }
      }
    }

    newX = player_x + player_facing_x * PLAYER_MOVE_SPEED * deltaSeconds * (player_state == Holding ? 0.5f : 1.f);
    newY = player_y + player_facing_y * PLAYER_MOVE_SPEED * deltaSeconds * (player_state == Holding ? 0.5f : 1.f);

    roll = guRandom() % 2;
    for (i = 0; i < (sizeof(player_sword) / sizeof(Vtx)); i++) {
      player_sword[i].v.cn[0] = (roll == 0) ? 40  : 120;
      player_sword[i].v.cn[1] = (roll == 0) ? 170  : 190;
      player_sword[i].v.cn[2] = 213;
    }

    if ((contdata[0].trigger & A_BUTTON) && (player_bullets_collected >= JUMP_COST)) {
      player_bullets_collected = MAX(0, (player_bullets_collected - JUMP_COST));
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
      player_sword[i].v.cn[0] = (roll == 0) ? 100  : 255;
      player_sword[i].v.cn[1] = (roll == 0) ? 100  : 255;
      player_sword[i].v.cn[2] = 243;
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

  if ((!isWarping) && (isTileBlocked(newTileX, newTileY) >= STAIRCASE_A) && (isTileBlocked(newTileX, newTileY) <= STAIRCASE_E)) {
    nextRoomRequest = exitMap[currentFloor][isTileBlocked(newTileX, newTileY) - STAIRCASE_A];

    isWarping = 1;
    warpDelta = 0;
    isWarpingOut = 1;
    return;
  }

  // step x
  if ((newTileX < (MAP_SIZE * TILE_SIZE)) && (newTileX >= 0) && (isTileBlocked(newTileX, (int)(player_y * INV_TILE_SIZE)))) {
    if (player_state == Jumping && (isTileBlocked(newTileX, (int)(player_y * INV_TILE_SIZE)) != LOW_WALL_TILE)) {
      HIT_WALL_WHILE_JUMPING = 1;
    }

    newX = player_x;
  }
  player_x = MIN(MAP_SIZE * TILE_SIZE, MAX(0, newX));

  // step y
  if ((newTileY < (MAP_SIZE * TILE_SIZE)) && (newTileY >= 0) && (isTileBlocked((int)(player_x * INV_TILE_SIZE), newTileY))) {
  
    if (player_state == Jumping && (isTileBlocked((int)(player_x * INV_TILE_SIZE), newTileY) != LOW_WALL_TILE)) {
      HIT_WALL_WHILE_JUMPING = 1;
    }

    newY = player_y;
  }
  player_y = MIN(MAP_SIZE * TILE_SIZE, MAX(0, newY));;

  // Lerp the camera
  camera_x = lerp(camera_x, player_x, CAMERA_LERP);
  camera_y = lerp(camera_y, player_y, CAMERA_LERP);
  
  if (!(isWarping)) {
    // Update bullets
    tickBullets(player_x, player_y, &player_state, deltaSeconds, &player_t);

    // Update emitters position/velocity/life
    tickEmitters(player_x, player_y, player_state, deltaSeconds, player_t);

    tickBoss(deltaSeconds, player_x, player_y);
  }

  nuDebPerfMarkSet(1);
  
}

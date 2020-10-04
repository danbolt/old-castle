#include <assert.h>
#include <nusys.h>
#include <nualsgi_n.h>
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
#include "audio_defines.h"

#define CAMERA_MOVE_SPEED 0.01726f
#define CAMERA_TURN_SPEED 0.03826f
#define CAMERA_DISTANCE 13.23f
#define CAMERA_HEIGHT 50.0f
#define CAMERA_LERP 0.13f
#define CAMERA_FOV 30.0f

#define WARP_IN_TIME_IN_SECONDS 2.34222f

#define LOCK_RADIUS 4.5f
#define LOCK_RADIUS_SQ (LOCK_RADIUS * LOCK_RADIUS)

#define ROOM_PURIFIED_TEXT_INDEX 5
#define ROOM_PURIFIED_TEXT_ONSCREEN_DURATION 5.16161f
const char* RoomPurifiedMessage = "Room Purified";

#define WARNING_TEXT_INDEX_A 6
#define WARNING_TEXT_INDEX_B 7

#define BATTLE_MODE_TRANSITION_TIME 1.8717623f

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
u8 bomb_count;
float player_sword_angle;
u8 player_bullets_collected;
s8 currentPlayerRoom;
u8 isInBattleMode;
float battleModeTime;

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
char bulletBuff[64];
char keysBuff[NUMBER_OF_SPECIAL_KEYS + 1];

char keyMessageBuffer[128];
DialogueLine keyLockLine = { keyMessageBuffer, NULL };

int isWarping;
int isWarpingOut;
float warpDelta;

SpecialKeyType specialKeyType;
float key_x;
float key_y;
u8 isThereASpecialKey;

SpecialKeyType lockType;
float lock_x;
float lock_y;
u8 isThereASpecialLock;

GeneratedRoom rooms[MAX_NUMBER_OF_ROOMS_PER_FLOOR];
int numberOfGeneratedRooms;

float roomPurifiedTime;

int frameBufferEmulationCheck[2];
int hasDoneFirstFrame;

extern u32 vertBuffUsage[MAX_NUMBER_OF_ROOMS_PER_FLOOR];

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

static Vtx key_geom[] = {
  { 33, 41, 106, 0, 0, 0, 0, 0, 0, 255 },
  { 33, -41, 106, 0, 0, 0, 2, 2, 2, 255 },
  { 42, 100, 133, 0, 0, 0, 166, 166, 166, 255 },
  { 42, -100, 133, 0, 0, 0, 158, 158, 158, 255 },
  { 25, 106, 275, 0, 0, 0, 255, 255, 255, 255 },
  { 25, -101, 275, 0, 0, 0, 255, 255, 255, 255 },
  { 38, 35, 31, 0, 0, 0, 176, 176, 176, 255 },
  { 38, -35, -23, 0, 0, 0, 108, 108, 108, 255 },
  { 38, -134, -57, 0, 0, 0, 235, 235, 235, 255 },
  { 44, 0, 133, 0, 0, 0, 132, 132, 132, 255 },
  { 38, -37, 29, 0, 0, 0, 255, 255, 255, 255 },
  { 36, -109, 6, 0, 0, 0, 255, 255, 255, 255 },
  { 0, 37, -88, 0, 0, 0, 32, 32, 32, 255 },
  { 0, -51, -92, 0, 0, 0, 32, 32, 32, 255 },
  { -33, 41, 106, 0, 0, 0, 0, 0, 0, 255 },
  { -33, -41, 106, 0, 0, 0, 2, 2, 2, 255 },
  { 0, 44, 105, 0, 0, 0, 0, 0, 0, 255 },
  { 0, -44, 105, 0, 0, 0, 11, 11, 11, 255 },
  { -42, 100, 133, 0, 0, 0, 166, 166, 166, 255 },
  { -42, -100, 133, 0, 0, 0, 158, 158, 158, 255 },
  { 0, -123, 121, 0, 0, 0, 158, 158, 158, 255 },
  { 0, 117, 128, 0, 0, 0, 158, 158, 158, 255 },
  { -25, 106, 275, 0, 0, 0, 255, 255, 255, 255 },
  { 0, 122, 298, 0, 0, 0, 255, 255, 255, 255 },
  { 0, -121, 298, 0, 0, 0, 255, 255, 255, 255 },
  { -25, -101, 275, 0, 0, 0, 255, 255, 255, 255 },
  { -38, 35, 31, 0, 0, 0, 176, 176, 176, 255 },
  { -38, -35, -23, 0, 0, 0, 108, 108, 108, 255 },
  { 0, 38, 27, 0, 0, 0, 176, 176, 176, 255 },
  { 0, -37, -24, 0, 0, 0, 69, 69, 69, 255 },
  { -38, -134, -57, 0, 0, 0, 235, 235, 235, 255 },
  { 0, -133, -86, 0, 0, 0, 89, 89, 89, 255 },
  { 0, -132, -30, 0, 0, 0, 251, 251, 251, 255 },
  { -44, 0, 133, 0, 0, 0, 132, 132, 132, 255 },
  { 0, 86, 204, 0, 0, 0, 16, 16, 16, 255 },
  { 0, 2, 259, 0, 0, 0, 84, 84, 84, 255 },
  { 0, -84, 204, 0, 0, 0, 28, 28, 28, 255 },
  { 0, 0, 150, 0, 0, 0, 158, 158, 158, 255 },
  { -38, -37, 29, 0, 0, 0, 255, 255, 255, 255 },
  { 0, -39, 29, 0, 0, 0, 255, 255, 255, 255 },
  { -36, -109, 6, 0, 0, 0, 255, 255, 255, 255 },
  { 0, -110, -11, 0, 0, 0, 255, 255, 255, 255 },
  { 0, -112, 22, 0, 0, 0, 255, 255, 255, 255 },
};

static Gfx key_dl[] = {
  gsSPVertex( key_geom, 43, 0),
  gsSP2Triangles( 3, 5, 24, 0, 10, 17, 39, 0),
  gsSP2Triangles( 28, 0, 6, 0, 6, 1, 10, 0),
  gsSP2Triangles( 21, 0, 16, 0, 3, 17, 1, 0),
  gsSP2Triangles( 9, 1, 0, 0, 24, 5, 4, 0),
  gsSP2Triangles( 12, 7, 13, 0, 12, 28, 6, 0),
  gsSP2Triangles( 7, 32, 8, 0, 8, 32, 31, 0),
  gsSP2Triangles( 13, 8, 31, 0, 13, 7, 8, 0),
  gsSP2Triangles( 3, 37, 36, 0, 2, 4, 34, 0),
  gsSP2Triangles( 6, 10, 7, 0, 10, 42, 11, 0),
  gsSP2Triangles( 11, 42, 41, 0, 7, 10, 11, 0),
  gsSP2Triangles( 29, 11, 41, 0, 4, 2, 21, 0),
  gsSP2Triangles( 21, 23, 4, 0, 24, 20, 3, 0),
  gsSP2Triangles( 10, 1, 17, 0, 28, 16, 0, 0),
  gsSP2Triangles( 6, 0, 1, 0, 21, 2, 0, 0),
  gsSP2Triangles( 3, 20, 17, 0, 0, 2, 9, 0),
  gsSP2Triangles( 9, 3, 1, 0, 4, 23, 24, 0),
  gsSP2Triangles( 12, 6, 7, 0, 7, 29, 32, 0),
  gsSP2Triangles( 36, 5, 3, 0, 3, 9, 37, 0),
  gsSP2Triangles( 37, 2, 34, 0, 37, 9, 2, 0),
  gsSP2Triangles( 5, 36, 35, 0, 4, 5, 35, 0),
  gsSP2Triangles( 35, 34, 4, 0, 10, 39, 42, 0),
  gsSP2Triangles( 29, 7, 11, 0, 19, 24, 25, 0),
  gsSP2Triangles( 38, 39, 17, 0, 28, 26, 14, 0),
  gsSP2Triangles( 26, 38, 15, 0, 21, 16, 14, 0),
  gsSP2Triangles( 19, 15, 17, 0, 33, 14, 15, 0),
  gsSP2Triangles( 24, 22, 25, 0, 12, 13, 27, 0),
  gsSP2Triangles( 12, 26, 28, 0, 27, 30, 32, 0),
  gsSP2Triangles( 30, 31, 32, 0, 13, 31, 30, 0),
  gsSP2Triangles( 13, 30, 27, 0, 19, 36, 37, 0),
  gsSP2Triangles( 18, 34, 22, 0, 26, 27, 38, 0),
  gsSP2Triangles( 38, 40, 42, 0, 40, 41, 42, 0),
  gsSP2Triangles( 27, 40, 38, 0, 29, 41, 40, 0),
  gsSP2Triangles( 22, 21, 18, 0, 21, 22, 23, 0),
  gsSP2Triangles( 24, 19, 20, 0, 38, 17, 15, 0),
  gsSP2Triangles( 28, 14, 16, 0, 26, 15, 14, 0),
  gsSP2Triangles( 21, 14, 18, 0, 19, 17, 20, 0),
  gsSP2Triangles( 14, 33, 18, 0, 33, 15, 19, 0),
  gsSP2Triangles( 22, 24, 23, 0, 12, 27, 26, 0),
  gsSP2Triangles( 27, 32, 29, 0, 36, 19, 25, 0),
  gsSP2Triangles( 19, 37, 33, 0, 37, 34, 18, 0),
  gsSP2Triangles( 37, 18, 33, 0, 25, 35, 36, 0),
  gsSP2Triangles( 22, 35, 25, 0, 35, 22, 34, 0),
  gsSP2Triangles( 38, 42, 39, 0, 29, 40, 27, 0),
  gsSPEndDisplayList()
};

static Vtx lock_geom[] = {
{ 89, -52, 71, 0, 0, 0, 255, 255, 255, 255 },
{ 112, -89, -71, 0, 0, 0, 1, 1, 1, 255 },
{ 94, -87, 0, 0, 0, 0, 120, 120, 120, 255 },
{ 58, -113, -61, 0, 0, 0, 97, 97, 97, 255 },
{ 39, -76, 73, 0, 0, 0, 255, 255, 255, 255 },
{ 50, -100, 0, 0, 0, 0, 193, 193, 193, 255 },
{ 100, -56, 0, 0, 0, 0, 194, 194, 194, 255 },
{ -89, -52, 71, 0, 0, 0, 255, 255, 255, 255 },
{ -112, -89, -71, 0, 0, 0, 1, 1, 1, 255 },
{ 0, -100, 100, 0, 0, 0, 255, 255, 255, 255 },
{ 0, -119, -104, 0, 0, 0, 4, 4, 4, 255 },
{ -94, -87, 0, 0, 0, 0, 120, 120, 120, 255 },
{ -58, -113, -61, 0, 0, 0, 97, 97, 97, 255 },
{ -39, -76, 73, 0, 0, 0, 255, 255, 255, 255 },
{ -50, -100, 0, 0, 0, 0, 187, 187, 187, 255 },
{ 0, -124, -49, 0, 0, 0, 97, 97, 97, 255 },
{ 0, -100, 50, 0, 0, 0, 255, 255, 255, 255 },
{ 0, -26, 3, 0, 0, 0, 0, 0, 0, 255 },
{ -100, -56, 0, 0, 0, 0, 192, 192, 192, 255 },
{ 89, 52, 71, 0, 0, 0, 255, 255, 255, 255 },
{ 112, 89, -71, 0, 0, 0, 1, 1, 1, 255 },
{ 125, 0, -104, 0, 0, 0, 108, 108, 108, 255 },
{ 100, 0, 100, 0, 0, 0, 255, 255, 255, 255 },
{ 94, 87, 0, 0, 0, 0, 120, 120, 120, 255 },
{ 58, 113, -61, 0, 0, 0, 97, 97, 97, 255 },
{ 39, 76, 73, 0, 0, 0, 190, 190, 190, 255 },
{ 50, 100, 0, 0, 0, 0, 194, 194, 194, 255 },
{ 100, 0, 51, 0, 0, 0, 255, 255, 255, 255 },
{ 124, 0, -51, 0, 0, 0, 138, 138, 138, 255 },
{ 100, 56, 0, 0, 0, 0, 192, 192, 192, 255 },
{ 50, 0, -2, 0, 0, 0, 0, 0, 0, 255 },
{ -89, 52, 71, 0, 0, 0, 255, 255, 255, 255 },
{ -112, 89, -71, 0, 0, 0, 1, 1, 1, 255 },
{ 0, 0, 100, 0, 0, 0, 255, 255, 255, 255 },
{ 0, 0, -100, 0, 0, 0, 16, 16, 16, 255 },
{ -125, 0, -104, 0, 0, 0, 108, 108, 108, 255 },
{ -100, 0, 100, 0, 0, 0, 255, 255, 255, 255 },
{ 0, 100, 100, 0, 0, 0, 255, 255, 255, 255 },
{ 0, 119, -104, 0, 0, 0, 4, 4, 4, 255 },
{ -94, 87, 0, 0, 0, 0, 189, 189, 189, 255 },
{ -58, 113, -61, 0, 0, 0, 97, 97, 97, 255 },
{ -39, 76, 73, 0, 0, 0, 255, 255, 255, 255 },
{ -50, 100, 0, 0, 0, 0, 194, 194, 194, 255 },
{ 0, 124, -49, 0, 0, 0, 97, 97, 97, 255 },
{ 0, 100, 50, 0, 0, 0, 255, 255, 255, 255 },
{ 0, 26, 3, 0, 0, 0, 0, 0, 0, 255 },
{ -100, 0, 51, 0, 0, 0, 255, 255, 255, 255 },
{ -124, 0, -51, 0, 0, 0, 138, 138, 138, 255 },
{ -100, 56, 0, 0, 0, 0, 150, 150, 150, 255 },
{ -50, 0, -2, 0, 0, 0, 12, 12, 12, 255 },
};

static Gfx lock_dl[] = {
  gsSPVertex( lock_geom, 50, 0),
  gsSP2Triangles(0, 6, 27, 0, 1, 3, 21, 0),
  gsSP2Triangles(10, 21, 3, 0, 22, 9, 4, 0),
  gsSP2Triangles(5, 3, 2, 0, 1, 2, 3, 0),
  gsSP2Triangles(5, 2, 4, 0, 10, 5, 15, 0),
  gsSP2Triangles(0, 22, 4, 0, 2, 0, 4, 0),
  gsSP2Triangles(16, 4, 9, 0, 5, 16, 17, 0),
  gsSP2Triangles(15, 5, 17, 0, 1, 21, 28, 0),
  gsSP2Triangles(2, 28, 6, 0, 22, 0, 27, 0),
  gsSP2Triangles(27, 6, 30, 0, 6, 28, 30, 0),
  gsSP2Triangles(7, 18, 11, 0, 8, 35, 12, 0),
  gsSP2Triangles(10, 35, 34, 0, 36, 9, 33, 0),
  gsSP2Triangles(14, 11, 12, 0, 8, 12, 11, 0),
  gsSP2Triangles(14, 13, 11, 0, 10, 14, 12, 0),
  gsSP2Triangles(7, 13, 36, 0, 11, 13, 7, 0),
  gsSP2Triangles(16, 13, 14, 0, 14, 17, 16, 0),
  gsSP2Triangles(15, 17, 14, 0, 8, 47, 35, 0),
  gsSP2Triangles(11, 47, 8, 0, 36, 46, 7, 0),
  gsSP2Triangles(46, 49, 18, 0, 18, 49, 47, 0),
  gsSP2Triangles(19, 29, 23, 0, 20, 21, 24, 0),
  gsSP2Triangles(38, 21, 34, 0, 22, 37, 33, 0),
  gsSP2Triangles(26, 23, 24, 0, 20, 24, 23, 0),
  gsSP2Triangles(26, 25, 23, 0, 38, 26, 24, 0),
  gsSP2Triangles(19, 25, 22, 0, 23, 25, 19, 0),
  gsSP2Triangles(44, 25, 26, 0, 26, 45, 44, 0),
  gsSP2Triangles(43, 45, 26, 0, 20, 28, 21, 0),
  gsSP2Triangles(23, 28, 20, 0, 22, 27, 19, 0),
  gsSP2Triangles(27, 30, 29, 0, 29, 30, 28, 0),
  gsSP2Triangles(31, 48, 46, 0, 32, 40, 35, 0),
  gsSP2Triangles(38, 35, 40, 0, 36, 37, 41, 0),
  gsSP2Triangles(42, 40, 39, 0, 32, 39, 40, 0),
  gsSP2Triangles(42, 39, 41, 0, 38, 42, 43, 0),
  gsSP2Triangles(31, 36, 41, 0, 39, 31, 41, 0),
  gsSP2Triangles(44, 41, 37, 0, 42, 44, 45, 0),
  gsSP2Triangles(43, 42, 45, 0, 32, 35, 47, 0),
  gsSP2Triangles(39, 47, 48, 0, 36, 31, 46, 0),
  gsSP2Triangles(46, 48, 49, 0, 48, 47, 49, 0),
  gsSP2Triangles(0, 2, 6, 0, 10, 34, 21, 0),
  gsSP2Triangles(22, 33, 9, 0, 10, 3, 5, 0),
  gsSP2Triangles(16, 5, 4, 0, 2, 1, 28, 0),
  gsSP2Triangles(7, 46, 18, 0, 10, 12, 35, 0),
  gsSP2Triangles(36, 13, 9, 0, 10, 15, 14, 0),
  gsSP2Triangles(16, 9, 13, 0, 11, 18, 47, 0),
  gsSP2Triangles(19, 27, 29, 0, 38, 24, 21, 0),
  gsSP2Triangles(22, 25, 37, 0, 38, 43, 26, 0),
  gsSP2Triangles(44, 37, 25, 0, 23, 29, 28, 0),
  gsSP2Triangles(31, 39, 48, 0, 38, 34, 35, 0),
  gsSP2Triangles(36, 33, 37, 0, 38, 40, 42, 0),
  gsSP2Triangles(44, 42, 41, 0, 39, 32, 47, 0),
  gsSPEndDisplayList()
};

inline int isInside(float x, float y, float minX, float minY, float maxX, float maxY) {
  const int insideX = (x > minX) && (x < maxX);
  const int insideY = (y > minY) && (y < maxY);

  return (insideX && insideY);
}

int isInsideRoom(float x, float y, GeneratedRoom* room, float extraBuffer) {
  const float roomMinX = (room->x * TILE_SIZE) - extraBuffer;
  const float roomMinY = (room->y * TILE_SIZE) - extraBuffer;
  const float roomMaxX = ((room->x + room->width) * TILE_SIZE) + extraBuffer;
  const float roomMaxY = ((room->y + room->height) * TILE_SIZE) + extraBuffer;

  return isInside(x, y, roomMinX, roomMinY, roomMaxX, roomMaxY);
}

/* The initialization of stage 0 */
void initStage00(int floorNumber)
{
  int i;

  target_distance = DEFAULT_TARGET_DISTANCE;
  player_state = Move;
  player_t = 0.f;
  HIT_WALL_WHILE_JUMPING = 0;
  player_sword_angle = 0.f;
  player_bullets_collected = 100;
  bomb_count = 3;
  isThereASpecialLock = 0;
  isThereASpecialKey = 0;
  currentPlayerRoom = -1;
  isInBattleMode = 0;
  battleModeTime = 0.f;

  trail_geo_index = 0;

  camera_x = 0.0f;
  camera_y = 0.0f;

  time = OS_CYCLES_TO_USEC(osGetTime());
  delta = 0;
  deltaSeconds = 0.f;

  camera_rotation = M_PI;
  player_rotation = -M_PI_2;

  roomPurifiedTime = 999.f;

  frameBufferEmulationCheck[1] = 0;

  resetTextRequests();

  initializeEntityData();

  // TODO: create a variable for this to change
  numberOfGeneratedRooms = initMap(rooms, &(roomSeeds[floorNumber]), floorNumber);
  initEnemiesForMap(rooms);

  for (i = 0; i < numberOfGeneratedRooms; i++) {
    if (rooms[i].type == LockRoom) {
      lockType = rooms[i].lockIndex;
      lock_x = (rooms[i].x + (rooms[i].width / 2)) * TILE_SIZE;
      lock_y = (rooms[i].y + (rooms[i].height / 2)) * TILE_SIZE;
      isThereASpecialLock = 1;
    }
  }

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

  if (currentFloor == 0) {
    isThereASpecialKey = 1;
    key_x = player_x + 4;
    key_y = player_y + 2;
    specialKeyType = SpecialKey_Red;
  }

  isWarping = 1;
  isWarpingOut = 0;
  warpDelta = 0;

  hasDoneFirstFrame = 0;
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

void renderRestRoom(GeneratedRoom* room, Dynamic* dynamicp) {
  if (!(isInsideRoom(player_x, player_y, room, 0.f))) {
    return;
  }

  // draw a little character in the center of the room
  guTranslate(&(dynamicp->roomEntityMatricies[0]), (room->x + (room->width * 0.5f)) * TILE_SIZE, (room->y + (room->height * 0.5f)) * TILE_SIZE, 0.f);
  guScale(&(dynamicp->roomEntityMatricies[1]), 0.01, 0.01, 0.01);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->roomEntityMatricies[0])), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->roomEntityMatricies[1])), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  addPlayerDisplayList();

  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
}

#define BATTLE_ROOM_R 0x4f
#define BATTLE_ROOM_G 0x6f
#define BATTLE_ROOM_B 0x72
#define BATTLE_ROOM_A 0xff

static Vtx battle_room_geom[] = {
  { -1,  -1, 0, 0, 0, 0, BATTLE_ROOM_R, BATTLE_ROOM_G, BATTLE_ROOM_B, BATTLE_ROOM_A },
  {  1,  -1, 0, 0, 0, 0, BATTLE_ROOM_R, BATTLE_ROOM_G, BATTLE_ROOM_B, BATTLE_ROOM_A },
  {  1,   1, 0, 0, 0, 0, BATTLE_ROOM_R, BATTLE_ROOM_G, BATTLE_ROOM_B, BATTLE_ROOM_A },
  { -1,   1, 0, 0, 0, 0, BATTLE_ROOM_R, BATTLE_ROOM_G, BATTLE_ROOM_B, BATTLE_ROOM_A },
};

static Gfx battle_room_commands[] = {
  gsSPClipRatio(FRUSTRATIO_6),
  gsSPVertex( battle_room_geom, 4, 0),
  gsSP2Triangles( 0, 1, 2, 0, 0, 2, 3, 0),
  gsSPClipRatio(FRUSTRATIO_2),
  gsSPEndDisplayList()
};

void renderForRooms(Dynamic* dynamicp) {
  int i;

  gDPPipeSync(glistp++);

  if ((isInBattleMode || (battleModeTime > 0.01f)) && (currentPlayerRoom != -1)) {
    GeneratedRoom* currentRoom = &(rooms[currentPlayerRoom]);
    const float t = (MIN(battleModeTime, BATTLE_MODE_TRANSITION_TIME - 1.2f) / (BATTLE_MODE_TRANSITION_TIME - 1.2f));

    guTranslate(&(dynamicp->battleRoomTranslation), (currentRoom->x + (currentRoom->width * 0.5f)) * TILE_SIZE, (currentRoom->y + (currentRoom->height * 0.5f)) * TILE_SIZE, -0.8f);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->battleRoomTranslation)), G_MTX_PUSH | G_MTX_MODELVIEW);

    guScale(&(dynamicp->battleRoomScale), currentRoom->width * t, currentRoom->height * (t * t), 1.f);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->battleRoomScale)), G_MTX_NOPUSH | G_MTX_MODELVIEW);

    gSPDisplayList(glistp++, battle_room_commands);

    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
  }

  guScale(&(dynamicp->roomRenderScale), 0.01f, 0.01f, 0.01f);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->roomRenderScale)), G_MTX_PUSH | G_MTX_MODELVIEW);

  if (!(isInBattleMode)) {
    for (i = 0; i < numberOfGeneratedRooms; i++) {
      GeneratedRoom* room = &(rooms[i]);

      gSPDisplayList(glistp++, rooms[i].commands);

      if (room->type == RestRoom) {
        renderRestRoom(room, dynamicp);
      }
    }
  } else if (battleModeTime < (BATTLE_MODE_TRANSITION_TIME - 0.0001f)) {
    
  }

  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
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

  // On the first 2 display lists, we check if we're running in an emulator by checking D-cache/FB information
  // TODO: cold-run this in a different scene before setup
  if (frameBufferEmulationCheck[1] == 0) {
    *((u8*)(0x8038F800)) = 255;
    osInvalDCache((void*)0x8038F800, 1);
    frameBufferEmulationCheck[1] = 1;
  } else if (frameBufferEmulationCheck[1] == 1) {
    if (*((u8*)(0x8038F800)) == 255) {
      frameBufferEmulationCheck[1] = 32;
    } else {
      frameBufferEmulationCheck[1] = 101;
    }
  }

  //nuDebPerfMarkSet(2);

  /* Specify the display list buffer */
  dynamicp = &gfx_dynamic[gfx_gtask_no];
  glistp = &gfx_glist[gfx_gtask_no][0];

  /* Initialize RCP */
  gfxRCPInit();

  /* Clear the frame and Z-buffer */
  gfxClearCfb();

  if (isInBattleMode || (battleModeTime > 0.01f)) {
    const int offset = ((time / 50000) % 32) - 32;
    const float t = battleModeTime / (WARP_IN_TIME_IN_SECONDS + 1.1456);
    const u8 r = 10 * t;
    const u8 g = 32 * t;
    const u8 b = 40 * t;

    for (i = 0; i < (SCREEN_WD / 16) + 2; i++) {
      for (j = 0; j < (SCREEN_HT / 16) + 2; j++) {
        if ((i + j) % 2 != 0) {
          continue;
        }

        gDPSetFillColor(glistp++, (GPACK_RGBA5551(r, g, b, 1) << 16 | GPACK_RGBA5551(r, g, b, 1)));
        gDPScisFillRectangle(glistp++, (i << 4) + offset, (j << 4) + offset, (i << 4) + 16 + offset,(j << 4) + 16 + offset);
      }
    }
    
  }

  /* projection,modeling matrix set */
  guPerspective(&dynamicp->projection, &perspNorm, CAMERA_FOV, (float)SCREEN_WD/(float)SCREEN_HT, 10.0f, 100.0f, 1.0f);
  guLookAt(&dynamicp->viewing, camera_x, camera_y + CAMERA_DISTANCE, CAMERA_HEIGHT, camera_x, camera_y, 0.0f, 0.0f, 0.0f, 1.0f);

  gSPPerspNormalize(glistp++, perspNorm);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->projection)), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->viewing)), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++, G_CYC_1CYCLE);
  if (!isInBattleMode || (battleModeTime < (BATTLE_MODE_TRANSITION_TIME - 0.0001f))) {
    gDPSetRenderMode(glistp++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
  } else {
    gDPSetRenderMode(glistp++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
  }
  gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
  gDPPipelineMode(glistp++, G_PM_NPRIMITIVE);

  if (!isInBattleMode || (battleModeTime < (BATTLE_MODE_TRANSITION_TIME - 0.0001f))) {
    gSPSetGeometryMode(glistp++, G_ZBUFFER | G_CULL_BACK | G_SHADE | G_SHADING_SMOOTH);
  } else {
    gSPSetGeometryMode(glistp++, G_CULL_BACK | G_SHADE | G_SHADING_SMOOTH);
  }

  if (isWarping) {
    gDPSetCombineLERP(glistp++, ENVIRONMENT,    0, 1,     0, 0,    0,     0, 0,
                                ENVIRONMENT,    0, SHADE,     0, 0,    0,     0, SHADE);
    if (isWarpingOut) {
      renderForRooms(dynamicp);
      gDPSetEnvColor(glistp++, 255 * (1.f - warpRatio), 255 * (1.f - warpRatio), 255 * (1.f - warpRatio), 255 * (1.f - warpRatio));
    } else {
      renderForRooms(dynamicp);
      gDPSetEnvColor(glistp++, 255 * (warpRatio), 255 * (warpRatio), 255 * (warpRatio), 255 * (warpRatio));
    }
  } else {
    renderForRooms(dynamicp);
  }

  guScale(&(dynamicp->grandBulletScale), 0.1f, 0.1f, 0.1f);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->grandBulletScale)), G_MTX_PUSH | G_MTX_MODELVIEW);

  // Render Bullets
  renderBullets(player_x, player_y, dynamicp);

  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

  // Render emitters
  renderEmitters(player_x, player_y, &dynamicp->playerScale, dynamicp);

  gDPPipeSync(glistp++);

    gDPPipeSync(glistp++);

  if (isThereASpecialKey && (!isWarping) && (fabs_d(key_x - player_x) < 30 && fabs_d(key_y - player_y) < 30)) {
    KeyColor* keyColor = getKeyColor(specialKeyType);

    guTranslate(&(dynamicp->specialKeyTranslation), key_x, key_y, 0.0f);
    guRotate(&dynamicp->specialKeyRotation, time * 0.0001f, 0.f, 0.f, 1.f);
    guRotate(&dynamicp->specialKeyRotation2, 20.f, 1.f, 0.f, 0.f);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->specialKeyTranslation)), G_MTX_PUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->specialKeyRotation2)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->specialKeyRotation)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->playerScale)), G_MTX_NOPUSH | G_MTX_MODELVIEW);

    gDPSetEnvColor(glistp++, 255 - keyColor->r, 255 - keyColor->g, 255 - keyColor->b, 255);
    gDPSetCombineLERP(glistp++, SHADE, ENVIRONMENT, 0, 0, 0, 0, 0, 0, SHADE, ENVIRONMENT, SHADE, 0, 0, 0, 0, SHADE);

    gDPPipeSync(glistp++);

    gSPDisplayList(glistp++, key_dl);
    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

    gDPSetCombineMode(glistp++, G_CC_SHADE, G_CC_SHADE);
    gDPPipeSync(glistp++);
  }

  if (isThereASpecialLock && (!isWarping) && (fabs_d(lock_x - player_x) < 30 && fabs_d(lock_y - player_y) < 30)) {
    KeyColor* lockColor = getKeyColor(lockType);

    guTranslate(&(dynamicp->specialLockTranslation), lock_x, lock_y, 2.0f);
    guScale(&(dynamicp->specialLockScale), 0.04f, 0.04f, 0.04f);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->specialLockTranslation)), G_MTX_PUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->specialLockScale)), G_MTX_NOPUSH | G_MTX_MODELVIEW);

    gDPSetEnvColor(glistp++, 255 - lockColor->r, 255 - lockColor->g, 255 - lockColor->b, 255);
    gDPSetCombineLERP(glistp++, SHADE, ENVIRONMENT, 0, 0, 0, 0, 0, 0, SHADE, ENVIRONMENT, SHADE, 0, 0, 0, 0, SHADE);

    gDPPipeSync(glistp++);

    gSPDisplayList(glistp++, lock_dl);
    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

    gDPSetCombineMode(glistp++, G_CC_SHADE, G_CC_SHADE);
    gDPPipeSync(glistp++);
  }

  gDPSetRenderMode(glistp++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
  gSPSetGeometryMode(glistp++, G_ZBUFFER | G_CULL_BACK | G_SHADE | G_SHADING_SMOOTH);

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

  //nuDebPerfMarkSet(3);

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

  renderBombEffect(player_x, player_y, dynamicp);

  renderBoss(dynamicp);

  //nuDebPerfMarkSet(4);

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

  //nuDebPerfMarkSet(5);

  drawTextRequests();

  gDPPipeSync(glistp++);

  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);

  assert((glistp - gfx_glist[gfx_gtask_no]) < GFX_GLIST_LEN);

  /* Activate the task and 
     switch display buffers */
  nuGfxTaskStart(&gfx_glist[gfx_gtask_no][0],
		 (s32)(glistp - gfx_glist[gfx_gtask_no]) * sizeof (Gfx),
		 NU_GFX_UCODE_F3DLX2_REJ , NU_SC_NOSWAPBUFFER);

  // nuDebTaskPerfBar1(1, 200, NU_SC_NOSWAPBUFFER);

  // for (i = 0; i < MAX_NUMBER_OF_ROOMS_PER_FLOOR; i++) {
    // nuDebConTextPos(0,1,3 + i);
    // sprintf(conbuf,"vb=%d / %d", vertBuffUsage[i], ROOM_VERT_BUFFER_SIZE);
    // nuDebConCPuts(0, conbuf);
  // }

  // if(contPattern & 0x1)
  // {
  //   nuDebConTextPos(0,1,3);
  //   sprintf(conbuf,"DL=%d / %d", (int)(glistp - gfx_glist[gfx_gtask_no]),  GFX_GLIST_LEN);
  //   nuDebConCPuts(0, conbuf);

    nuDebConTextPos(0,1,4);
    sprintf(conbuf,"current room = %8d", alCSeqGetTicks(&(nuAuSeqPlayer[0].sequence)));
    nuDebConCPuts(0, conbuf);

    // nuDebConTextPos(0,1,5);
    // sprintf(conbuf,"isInBattleMode=%3d", isInBattleMode);
    // nuDebConCPuts(0, conbuf);

    // nuDebConTextPos(0,1,6);
    // sprintf(conbuf,"battleModeTime=%5.2f", battleModeTime);
    // nuDebConCPuts(0, conbuf);

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

DialogueLine b = { "what do they have to say?", NULL };
DialogueLine a = { "it's a caretaker for the manor", &b };

void updateRestRoom(GeneratedRoom* room, int index) {
  if (!(isInsideRoom(player_x, player_y, room, -8.f))) {
    return;
  }

  if ((!isDialogueInProcess()) && (!hasRoomBeenCleared(currentFloor, index))) {
    setDialogue(&a);

    clearRoom(currentFloor, index);
  }
}

void updateEnemyRoom(GeneratedRoom* room, int index) {
  int i;
  int hasClearedAllEnemies = 1;

  if (hasRoomBeenCleared(currentFloor, index)) {
    return;
  }

  if (room->numberOfEnemies == 0) {
    clearRoom(currentFloor, index);
    return;
  }

  if ((currentPlayerRoom == index) && (isInBattleMode == 0)) {
    isInBattleMode = 1;
    battleModeTime = 0.f;
  }

  for (i = 0; i < room->numberOfEnemies; i++) {
    const int enemyIndex = room->enemies[i];
    const int hasDefeatedEnemy = isEmitterAlive(enemyIndex);

    if (hasDefeatedEnemy != 0) {
      hasClearedAllEnemies = 0;
    }
  }

  if (hasClearedAllEnemies) {
    clearRoom(currentFloor, index);
    isInBattleMode = 0;

    roomPurifiedTime = 0.f;

    getTextRequest(ROOM_PURIFIED_TEXT_INDEX)->enable = 1;
    getTextRequest(ROOM_PURIFIED_TEXT_INDEX)->text = RoomPurifiedMessage;
    getTextRequest(ROOM_PURIFIED_TEXT_INDEX)->x = (SCREEN_WD / 2) - ((13 * 8) / 2);
    getTextRequest(ROOM_PURIFIED_TEXT_INDEX)->y = SCREEN_HT / 2 + 4;
    getTextRequest(ROOM_PURIFIED_TEXT_INDEX)->cutoff = 0;
    getTextRequest(ROOM_PURIFIED_TEXT_INDEX)->typewriterTick = 0;
  }
  
}

void updateForRooms() {
  int i;

  for (i = 0; i < numberOfGeneratedRooms; i++) {
    GeneratedRoom* room = &(rooms[i]);

    if (isInsideRoom(player_x, player_y, room, 0.f)) {
      currentPlayerRoom = i;
    }

    if (room->type == RestRoom) {
      updateRestRoom(room, i);
    } else if (room->type == EnemyRoom) {
      updateEnemyRoom(room, i);
    }
  }
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

  //nuDebPerfMarkSet(0);

  if (!hasDoneFirstFrame) {
    hasDoneFirstFrame = 1;
    playSound(FadeIn);
  }

  sprintf(testStringBuf, "found %d", player_bullets_collected);
  getTextRequest(0)->enable = 1;
  getTextRequest(0)->text = testStringBuf;
  getTextRequest(0)->x = 8;
  getTextRequest(0)->y = 8;
  getTextRequest(0)->cutoff = -1;
  getTextRequest(0)->typewriterTick = 0;

  sprintf(bulletBuff, "remain %d", bomb_count);
  getTextRequest(1)->enable = 1;
  getTextRequest(1)->text = bulletBuff;
  getTextRequest(1)->x = 8;
  getTextRequest(1)->y = 16;
  getTextRequest(1)->cutoff = -1;
  getTextRequest(1)->typewriterTick = 0;

  for (i = 0; i < NUMBER_OF_SPECIAL_KEYS; i++) {
    keysBuff[i] = hasSpecialKey(i) ? 'K' : ' ';
  }
  keysBuff[NUMBER_OF_SPECIAL_KEYS] = '\0';
  getTextRequest(2)->enable = 1;
  getTextRequest(2)->text = keysBuff;
  getTextRequest(2)->x = 8;
  getTextRequest(2)->y = 24;
  getTextRequest(2)->cutoff = -1;
  getTextRequest(2)->typewriterTick = 0;

  if (isDialogueInProcess()) {
    getTextRequest(0)->enable = 0;
    getTextRequest(1)->enable = 0;
    getTextRequest(2)->enable = 0;
  }

  if (isInBattleMode && (battleModeTime < (BATTLE_MODE_TRANSITION_TIME - 0.001f))) {
    getTextRequest(WARNING_TEXT_INDEX_A)->enable = 1;
    getTextRequest(WARNING_TEXT_INDEX_A)->text = "WARNING";
    getTextRequest(WARNING_TEXT_INDEX_A)->x =  -(8 * 7) + ((battleModeTime / BATTLE_MODE_TRANSITION_TIME) * (SCREEN_WD + (8 * 8)));
    getTextRequest(WARNING_TEXT_INDEX_A)->y =  32;
    getTextRequest(WARNING_TEXT_INDEX_A)->cutoff = -1;
    getTextRequest(WARNING_TEXT_INDEX_A)->typewriterTick = 0;

    getTextRequest(WARNING_TEXT_INDEX_B)->enable = 1;
    getTextRequest(WARNING_TEXT_INDEX_B)->text = "spirits of the manor";
    getTextRequest(WARNING_TEXT_INDEX_B)->x =  -(8 * 20) + ((battleModeTime / BATTLE_MODE_TRANSITION_TIME) * (SCREEN_WD + (8 * 21)));
    getTextRequest(WARNING_TEXT_INDEX_B)->y = SCREEN_HT - 40;
    getTextRequest(WARNING_TEXT_INDEX_B)->cutoff = -1;
    getTextRequest(WARNING_TEXT_INDEX_B)->typewriterTick = 0;
  }

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

    if ((contdata[0].button & A_BUTTON) && (!isDialogueInProcess())) {
      if (player_state == Move) {
        player_sword_angle = player_rotation;
      }
      if (player_state != Holding) {
        playSound(SwordOut);
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

    // If we're warping or showing text, don't allow motion
    if (isWarping || isDialogueInProcess()) {
      stickX = 0.f;
      stickY = 0.f;
    } else {
      if (bomb_count > 0) {
        if ((contdata[0].trigger & Z_TRIG) || (contdata[0].trigger & L_TRIG)) {
          bomb_count--;

          fireBomb();
        }
      }
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

    if ((contdata[0].trigger & B_BUTTON) && (player_bullets_collected >= JUMP_COST)) {
      player_bullets_collected = MAX(0, (player_bullets_collected - JUMP_COST));
      player_state = Jumping;
      player_t = 0.f;
      player_jump_x = player_x;
      player_jump_y = player_y;
      HIT_WALL_WHILE_JUMPING = 0;

      playSound(Jump);
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

      playSound(AttackLand);
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

    playSound(FadeOut);
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

  // If we're in battle mode, don't let the player leave the room!
  if (isInBattleMode && (currentPlayerRoom > -1)) {
    GeneratedRoom* room = &(rooms[currentPlayerRoom]);
    player_x = MIN(MAX( player_x, room->x * TILE_SIZE ), ((room->x + room->width) * TILE_SIZE));
    player_y = MIN(MAX( player_y, room->y * TILE_SIZE ), ((room->y + room->height) * TILE_SIZE));
  }

  // step y
  if ((newTileY < (MAP_SIZE * TILE_SIZE)) && (newTileY >= 0) && (isTileBlocked((int)(player_x * INV_TILE_SIZE), newTileY))) {
  
    if (player_state == Jumping && (isTileBlocked((int)(player_x * INV_TILE_SIZE), newTileY) != LOW_WALL_TILE)) {
      HIT_WALL_WHILE_JUMPING = 1;
    }

    newY = player_y;
  }
  player_y = MIN(MAP_SIZE * TILE_SIZE, MAX(0, newY));

  // If we're in battle mode, don't let the player leave the room!
  if (isInBattleMode && (currentPlayerRoom > -1)) {
    GeneratedRoom* room = &(rooms[currentPlayerRoom]);
    player_x = MIN(MAX( player_x, room->x * TILE_SIZE ), ((room->x + room->width) * TILE_SIZE));
    player_y = MIN(MAX( player_y, room->y * TILE_SIZE ), ((room->y + room->height) * TILE_SIZE));
  }

  if (isThereASpecialKey) {
    float dx = (player_x - key_x);
    float dy = (player_y - key_y);
    dx = (dx * dx);
    dy = (dy * dy);

    if ((dx + dy) <= PLAYER_HIT_RADIUS_SQ) {
      isThereASpecialKey = 0;
      giveSpecialKey(specialKeyType);

      sprintf(keyMessageBuffer, "\n\n\n      Got a %s\n      Key of %s.", getKeyAdjective(specialKeyType), getKeyName(specialKeyType));
      setDialogue(&keyLockLine);
    }
  }

  if (isThereASpecialLock) {
    float dx = (lock_x - player_x);
    float dy = (lock_y - player_y);
    float dxSq = (dx * dx);
    float dySq = (dy * dy);

    if ((dxSq + dySq) <= LOCK_RADIUS_SQ) {
      if (hasSpecialKey(lockType)) {
        isThereASpecialLock = 0;
      } else {
        float pushbackAngle = nu_atan2(dy, dx);
        player_x = lock_x - cosf(pushbackAngle) * (LOCK_RADIUS + 0.2f);
        player_y = lock_y - sinf(pushbackAngle) * (LOCK_RADIUS + 0.2f);

        sprintf(keyMessageBuffer, "\n\n\n It looks like a \n\n  %s Lock of %s\n\n blocks the way.", getKeyAdjective(lockType), getKeyName(lockType));
        setDialogue(&keyLockLine);
      }
    }
  }

  // Lerp the camera
  camera_x = lerp(camera_x, player_x, CAMERA_LERP);
  camera_y = lerp(camera_y, player_y, CAMERA_LERP);
  
  if (!(isWarping)) {
    tickBoss(deltaSeconds, player_x, player_y);

    // Update bullets
    tickBullets(player_x, player_y, &player_state, deltaSeconds, &player_t);

    // Update emitters position/velocity/life
    if (isInBattleMode) {
      tickEmitters(player_x, player_y, player_state, deltaSeconds, player_t);
    }
  }

  roomPurifiedTime += deltaSeconds;
  if ((getTextRequest(ROOM_PURIFIED_TEXT_INDEX)->enable == 1) && (roomPurifiedTime > ROOM_PURIFIED_TEXT_ONSCREEN_DURATION)) {
    getTextRequest(ROOM_PURIFIED_TEXT_INDEX)->enable = 0;
  }

  updateForRooms();

  if (isInBattleMode) {
    battleModeTime = MIN(battleModeTime + deltaSeconds, BATTLE_MODE_TRANSITION_TIME);
  } else {
    battleModeTime = MAX(battleModeTime - deltaSeconds, 0.f);
  }

  //nuDebPerfMarkSet(1);
  
}

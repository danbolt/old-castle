
// This file corresponds to the enemies of the game

#ifndef ENTITY_DATA_H
#define ENTITY_DATA_H

#include <nusys.h>

#include "PlayerData.h"
#include "RoomData.h"

#include "graphic.h"

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
} AimEmitterData;

typedef struct {
  s8 emitterIndex;

  float spinSpeed; // Radians
  float totalTime;
} SpinEmitterData;

// Clears all entity data; this should be done on level start
void initializeEntityData();

// Creates a aim emitter entity and returns its index
// Returns -1 if there's no room in emitter memory (for positon, state, etc.)
// Returns -2 if there's no room in aim emitter memory (aiming logic)
int generateAimEmitterEntity(float x, float y);
int generateSpinEmitterEntity(float x, float y);

int generateBossA(float x, float y);

void tickAimEmitters(float player_x, float player_y, PlayerState player_state, float deltaSeconds, float player_t);
void renderAimEmitters(float player_x, float player_y, Mtx* aimEmitterScale);

void tickBoss(float deltaSeconds, float player_x, float player_y);
void renderBoss(Dynamic* dynamic);

// Creates a bullet and returns its index
int consumeNextBullet();
void setBulletState(int bulletIndex, u8 state);

void tickBullets(float player_x, float player_y, PlayerState* player_state, float deltaSeconds, float* player_t);
void renderBullets(float view_x, float view_y);

Position* getBulletPosition(int bulletIndex);
Velocity* getBulletVelocity(int bulletIndex);

#endif 
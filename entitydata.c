
#include "EntityData.h"

#include "graphic.h"

#include "game_math.h"

// Constants
#define BULLET_COUNT 200
#define EMITTER_COUNT 64
#define AIM_EMITTER_COUNT 64
#define EMITTER_RADIUS 4.f
#define EMITTER_RADIUS_SQ (EMITTER_RADIUS * EMITTER_RADIUS)

#define EMITTER_DEAD 0
#define EMITTER_ALIVE 1

// Static memory data
static Position BulletPositions[BULLET_COUNT];
static Velocity BulletVelocities[BULLET_COUNT];
static u8 BulletStates[BULLET_COUNT];
static u8 BulletRadiiSquared[BULLET_COUNT];
static EntityTransform BulletMatricies[BULLET_COUNT];
int NextBulletIndex;

static AimEmitterData AimEmitters[AIM_EMITTER_COUNT];
static int NextAimEmitterIndex;

static Position EmitterPositions[EMITTER_COUNT];
static Velocity EmitterVelocities[EMITTER_COUNT];
static EntityTransform EmitterMatricies[EMITTER_COUNT];
static EntityTransform EmitterRotations[EMITTER_COUNT];
static u8 EmitterStates[EMITTER_COUNT];
static u32 EmitterTicks[EMITTER_COUNT];
static int NextEmitterIndex;

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

void initializeEntityData() {
	int i;

	// Initialize bullets
	for (i = 0; i < BULLET_COUNT; i++) {
		BulletPositions[i].x = MAP_SIZE * TILE_SIZE * 0.15f;
		BulletPositions[i].y = MAP_SIZE * TILE_SIZE * 0.15f;

		BulletVelocities[i].x = cosf(guRandom() % 7) * 0.05f;
		BulletVelocities[i].y = sinf(guRandom() % 7) * 0.05f;

		BulletStates[i] = 0;
		BulletRadiiSquared[i] = 1 * 1;

		guMtxIdent(&(BulletMatricies[i].mat));   
	}
	NextBulletIndex = 0;

	// Initialize emitters
	for (i = 0; i < EMITTER_COUNT; i++) {
		EmitterStates[i] = EMITTER_DEAD;
		EmitterPositions[i].x = 0.f;
		EmitterPositions[i].y = 0.f;
		EmitterVelocities[i].x = 0.f;
		EmitterVelocities[i].y = 0.f;
		EmitterTicks[i] = 0;

		guMtxIdent(&(EmitterMatricies[i].mat));   
	}
	NextEmitterIndex = 0;

	// Initialize aim emitters
	for (i = 0; i < AIM_EMITTER_COUNT; i++) { 
		AimEmitters[i].emitterIndex = -1;
		AimEmitters[i].period = 2.0f;
		AimEmitters[i].shotsToFire = 1;
		AimEmitters[i].spreadSpeadInDegrees = 90.f;
		AimEmitters[i].t = 0.f + (guRandom() % 5);
	}
	NextAimEmitterIndex = 0;
}

int generateAimEmitterEntity(float x, float y) {

	int newEmitterIndex = NextEmitterIndex;
	int newAimEmitterIndex = NextAimEmitterIndex;

	if (newEmitterIndex == EMITTER_COUNT) {
		return -1;
	}

	if (newAimEmitterIndex == AIM_EMITTER_COUNT) {
		return -2;
	}

	EmitterStates[newEmitterIndex] = EMITTER_ALIVE;
    EmitterPositions[newEmitterIndex].x = x;
    EmitterPositions[newEmitterIndex].y = y;
    EmitterVelocities[newEmitterIndex].x = 0.f;
    EmitterVelocities[newEmitterIndex].y = 0.f;
    NextEmitterIndex++;

    AimEmitters[newAimEmitterIndex].emitterIndex = newEmitterIndex;
    NextAimEmitterIndex++;

	return newAimEmitterIndex;
}

extern float test;

void tickAimEmitters(float player_x, float player_y, PlayerState player_state, float deltaSeconds, float player_t) {
	int i;

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
    Position* bulletPosition = NULL;
	Velocity* bulletVelocity = NULL;

    if (AimEmitters[i].emitterIndex == -1) {
      continue;
    }

    if ((EmitterStates[AimEmitters[i].emitterIndex]) == EMITTER_DEAD) {
      AimEmitters[i].emitterIndex = -1;
      continue;
    }

    // if we're really far away, don't worry about updating for now
    if ((fabs_d(player_y - EmitterPositions[AimEmitters[i].emitterIndex].y) > (RENDER_DISTANCE * 2)) || (fabs_d(player_x - EmitterPositions[AimEmitters[i].emitterIndex].x) > (RENDER_DISTANCE * 2))) {
      continue;
    }

    AimEmitters[i].t += deltaSeconds;
    if (AimEmitters[i].t < AimEmitters[i].period) {
      continue;
    }

    // If we've made it here, fire
    AimEmitters[i].t = 0;

    // If the player's just about to land on or near us, we should avoid making a cruel shot
    if ((fabs_d(player_y - BulletPositions[newBulletIndex].y) < 1.3f) || (fabs_d(player_x - BulletPositions[newBulletIndex].x) < 1.3f)) {
      if ((player_state == Jumping) && (player_t > 0.8f)) {
        continue;
      }

      if ((player_state == Landed) && (player_t > 0.5f)) {
        continue;
      }
    }

    // Skip if there are no available bullets
    newBulletIndex = consumeNextBullet();
    if (newBulletIndex == -1) {
      continue;
    }
	bulletPosition = getBulletPosition(newBulletIndex);
	bulletVelocity = getBulletVelocity(newBulletIndex);
	setBulletState(newBulletIndex, 1);
	bulletPosition->x = EmitterPositions[AimEmitters[i].emitterIndex].x;
	bulletPosition->y = EmitterPositions[AimEmitters[i].emitterIndex].y;
	theta = nu_atan2(player_y - bulletPosition->y, player_x - bulletPosition->x);
	bulletVelocity->x = 5.831332f * cosf(theta);
	bulletVelocity->y = 5.831332f * sinf(theta);
  }
}

void renderAimEmitters(float player_x, float player_y, Mtx* aimEmitterScale) {
	int i;

  gDPSetCombineLERP(glistp++, NOISE,    0, SHADE,     0,
                                  0,    0,     0, SHADE,
                              NOISE,    0, SHADE,     0,
                                  0,    0,     0, SHADE);
  for (i = 0; i < EMITTER_COUNT; i++) {
    float dxSq;
    float dySq;
    float dyRot;
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

    dyRot = nu_atan2(player_y - EmitterPositions[i].y, player_x - EmitterPositions[i].x) + M_PI_2;
    guTranslate(&(EmitterMatricies[i].mat), EmitterPositions[i].x, EmitterPositions[i].y, 0.f);
    guRotate(&(EmitterRotations[i].mat), dyRot / M_PI * 180, 0.f, 0.f, 1.f);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(EmitterMatricies[i])), G_MTX_PUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(EmitterRotations[i])), G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(aimEmitterScale), G_MTX_NOPUSH | G_MTX_MODELVIEW);
    addEmitterToDisplayList();
    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
  }
  gDPSetCombineMode(glistp++, G_CC_SHADE, G_CC_SHADE);
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
    //NextBulletIndex = i;
    break;
  }

  return result;
}

void setBulletState(int bulletIndex, u8 state) {
	if ((bulletIndex < 0) || (bulletIndex >= BULLET_COUNT)) {
		// TODO: create some sort of warning output or potentially crash
		return;
	}

	BulletStates[bulletIndex] = state;
}

// hack: make these extern for now and move into a header later
extern float player_sword_angle;
extern u8 player_bullets_collected;

void tickBullets(float player_x, float player_y, PlayerState* player_state, float deltaSeconds, float* player_t) {
	int i;

	for (i = 0; i < BULLET_COUNT; i++) {
		float dxSq = 9999.f;
		float dySq = 9999.f;
		u8 computedRadiusForHoldingAlready = 0;

		if (BulletStates[i] == 0) {
		  continue;
		}

		if ((BulletPositions[i].x < 0) || (BulletPositions[i].x > MAP_SIZE * TILE_SIZE)
		    || (BulletPositions[i].y < 0) || (BulletPositions[i].y > MAP_SIZE * TILE_SIZE)) {
		  BulletStates[i] = 0;
		}

		if (isTileBlocked((int)(BulletPositions[i].x * INV_TILE_SIZE), (int)(BulletPositions[i].y * INV_TILE_SIZE))) {
		  BulletStates[i] = 0;
		  continue;
		}

		BulletPositions[i].x += BulletVelocities[i].x * deltaSeconds;
		BulletPositions[i].y += BulletVelocities[i].y * deltaSeconds;

		// If the player's in the air or dead, there's no point in checking death
		if ((*player_state == Jumping) || (*player_state == Dead)) {
		  continue;
		}

		if (*player_state == Holding) {
		  dxSq = player_x - BulletPositions[i].x;
		  dxSq = dxSq * dxSq;
		  dySq = player_y - BulletPositions[i].y;
		  dySq = dySq * dySq;
		  computedRadiusForHoldingAlready = 1;

		  if ((dxSq + dySq) <= SWORD_RADUS_SQ) {
		    float angleToPlayer = nu_atan2(BulletPositions[i].y - player_y, BulletPositions[i].x - player_x);
		    float angleDelta = fabs_d(angleToPlayer - player_sword_angle);

		    if ((angleDelta < 0.1f)
		      || ((dxSq + dySq) <= (SWORD_RADUS_SQ * 0.63f)) && (angleDelta < 0.2f)
		      || ((dxSq + dySq) <= (SWORD_RADUS_SQ * 0.23f)) && (angleDelta < 0.3f)) {
		      BulletStates[i] = 0;
		      player_bullets_collected = MIN(100, (player_bullets_collected + POINTS_PER_BULLET));
	    	  continue;
		    }
		  }
		}

		if (!computedRadiusForHoldingAlready) {
		  dxSq = player_x - BulletPositions[i].x;
		  dxSq = dxSq * dxSq;
		}
		if (dxSq > BulletRadiiSquared[i]) {
		  continue;
		}

		if (!computedRadiusForHoldingAlready) {
		  dySq = player_y - BulletPositions[i].y;
		  dySq = dySq * dySq;
		}
		if (dySq > BulletRadiiSquared[i]) {
		  continue;
		}

		if ((dySq + dxSq) >= PLAYER_HIT_RADIUS_SQ) {
		  continue;
		}

		// If we've reached this line, the bullet has hit the player
		*player_state = Dead;
		*player_t = 0;
	}
}

void renderBullets(float view_x, float view_y) {
	int i;

	for (i = 0; i < BULLET_COUNT; i++) {
	    float dxSq;
	    float dySq;
	    if (BulletStates[i] == 0) {
	      continue;
	    }

	    dxSq = view_x - BulletPositions[i].x;
	    dxSq = dxSq * dxSq;
	    if (dxSq > RENDER_DISTANCE_SQ) {
	      continue;
	    }

	    dySq = view_y - BulletPositions[i].y;
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
}

Position* getBulletPosition(int bulletIndex) {
	if ((bulletIndex < 0) || (bulletIndex >= BULLET_COUNT)) {
		return NULL;
	}

	return &(BulletPositions[bulletIndex]);
}
Velocity* getBulletVelocity(int bulletIndex) {
	if ((bulletIndex < 0) || (bulletIndex >= BULLET_COUNT)) {
		return NULL;
	}

	return &(BulletVelocities[bulletIndex]);
}





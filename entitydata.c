
#include "EntityData.h"

#include "graphic.h"

#include "game_math.h"

// Constants
#define BULLET_COUNT 200
#define EMITTER_COUNT 64
#define AIM_EMITTER_COUNT 64
#define SPIN_EMITTER_COUNT 64
#define EMITTER_RADIUS 4.f
#define EMITTER_RADIUS_SQ (EMITTER_RADIUS * EMITTER_RADIUS)

#define EMITTER_DEAD 0
#define EMITTER_ALIVE 1

typedef struct {
  float period;
  float t;
} EmitterTimeData;

// Static memory data
static Position BulletPositions[BULLET_COUNT];
static Velocity BulletVelocities[BULLET_COUNT];
static u8 BulletStates[BULLET_COUNT];
static u8 BulletRadiiSquared[BULLET_COUNT];
static EntityTransform BulletMatricies[BULLET_COUNT];
int NextBulletIndex;

static AimEmitterData AimEmitters[AIM_EMITTER_COUNT];
static int NextAimEmitterIndex;

static SpinEmitterData SpinEmitters[SPIN_EMITTER_COUNT];
static int NextSpinEmitterIndex;

static Position EmitterPositions[EMITTER_COUNT];
static Velocity EmitterVelocities[EMITTER_COUNT];
static EntityTransform EmitterMatricies[EMITTER_COUNT];
static EntityTransform EmitterRotations[EMITTER_COUNT];
static u8 EmitterStates[EMITTER_COUNT];
static EmitterTimeData EmitterTimes[EMITTER_COUNT];
static u32 EmitterTicks[EMITTER_COUNT];
static int NextEmitterIndex;

#define NO_BOSS_SET 0
#define BOSS_A_SET 1

static int BossSetting;
static float boss_x;
static float boss_y;
static float boss_starting_x;
static float boss_starting_y;
static float boss_rotation;

static Vtx test_boss_main_geo[] = {
{ 274, 46, 158, 0, 0, 0, 0, 0, 0, 255 },
{ 238, 272, -246, 0, 0, 0, 5, 5, 5, 255 },
{ 238, -63, -246, 0, 0, 0, 78, 74, 59, 255 },
{ 300, 337, -26, 0, 0, 0, 0, 0, 0, 255 },
{ 265, -106, -58, 0, 0, 0, 38, 36, 29, 255 },
{ 119, -93, 211, 0, 0, 0, 2, 2, 2, 255 },
{ 225, -145, -26, 0, 0, 0, 255, 243, 194, 255 },
{ 133, -121, 129, 0, 0, 0, 255, 243, 194, 255 },
{ 248, -123, 145, 0, 0, 0, 255, 243, 194, 255 },
{ 145, -147, 50, 0, 0, 0, 255, 243, 194, 255 },
{ 269, -96, -136, 0, 0, 0, 228, 217, 174, 255 },
{ 77, -85, 159, 0, 0, 0, 233, 249, 237, 255 },
{ 238, -85, 146, 0, 0, 0, 255, 255, 255, 255 },
{ 95, -122, 90, 0, 0, 0, 221, 12, 0, 255 },
{ 140, -128, 17, 0, 0, 0, 222, 12, 0, 255 },
{ 127, -108, 143, 0, 0, 0, 222, 12, 0, 255 },
{ 160, -120, 99, 0, 0, 0, 222, 12, 0, 255 },
{ -274, 46, 158, 0, 0, 0, 0, 0, 0, 255 },
{ -238, 272, -246, 0, 0, 0, 5, 5, 5, 255 },
{ -238, -63, -246, 0, 0, 0, 78, 74, 59, 255 },
{ 0, 280, -288, 0, 0, 0, 10, 10, 9, 255 },
{ 0, -122, 236, 0, 0, 0, 24, 24, 19, 255 },
{ 0, -71, -288, 0, 0, 0, 155, 148, 118, 255 },
{ 0, 346, -15, 0, 0, 0, 0, 0, 0, 255 },
{ -300, 337, -26, 0, 0, 0, 0, 0, 0, 255 },
{ -265, -106, -58, 0, 0, 0, 38, 36, 29, 255 },
{ 0, -195, -26, 0, 0, 0, 255, 243, 195, 255 },
{ -119, -93, 211, 0, 0, 0, 2, 2, 2, 255 },
{ 0, -159, 105, 0, 0, 0, 255, 243, 194, 255 },
{ -225, -145, -26, 0, 0, 0, 255, 243, 194, 255 },
{ -133, -121, 129, 0, 0, 0, 255, 243, 194, 255 },
{ 0, -177, 39, 0, 0, 0, 255, 243, 194, 255 },
{ -248, -123, 145, 0, 0, 0, 255, 243, 194, 255 },
{ -145, -147, 50, 0, 0, 0, 255, 243, 194, 255 },
{ 0, -133, -220, 0, 0, 0, 156, 148, 118, 255 },
{ -269, -96, -136, 0, 0, 0, 228, 217, 174, 255 },
{ 0, -164, -123, 0, 0, 0, 255, 243, 197, 255 },
{ -77, -85, 159, 0, 0, 0, 233, 249, 237, 255 },
{ 0, -85, 37, 0, 0, 0, 255, 255, 255, 255 },
{ -238, -85, 146, 0, 0, 0, 255, 255, 255, 255 },
{ 0, -85, -93, 0, 0, 0, 255, 243, 194, 255 },
{ -95, -122, 90, 0, 0, 0, 221, 12, 0, 255 },
{ -140, -128, 17, 0, 0, 0, 222, 12, 0, 255 },
{ -127, -108, 143, 0, 0, 0, 222, 12, 0, 255 },
{ -160, -120, 99, 0, 0, 0, 222, 12, 0, 255 },
};

static Vtx test_boss_hair_geo[] = {
  {  1,   1, 1, 0, 0, 0, 0, 0, 0, 0xff },
  { -1,   1, 1, 0, 0, 0, 0, 0, 0, 0xff },
  { -1,  -1, 1, 0, 0, 0, 0, 0, 0, 0xff },
  {  1,  -1, 1, 0, 0, 0, 0, 0, 0, 0xff },
  {  1,   1, -1, 0, 0, 0, 0, 0, 0, 0xff },
  { -1,   1, -1, 0, 0, 0, 0, 0, 0, 0xff },
  { -1,  -1, -1, 0, 0, 0, 0, 0, 0, 0xff },
  {  1,  -1, -1, 0, 0, 0, 0, 0, 0, 0xff },
};

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
    EmitterTimes[i].t = 0.f;

		guMtxIdent(&(EmitterMatricies[i].mat));   
	}
	NextEmitterIndex = 0;

	// Initialize aim emitters
	for (i = 0; i < AIM_EMITTER_COUNT; i++) { 
		AimEmitters[i].emitterIndex = -1;
		EmitterTimes[i].period = 2.0f;
		AimEmitters[i].shotsToFire = 1;
		AimEmitters[i].spreadSpeadInDegrees = 90.f;
		EmitterTimes[i].t = 0.f + (guRandom() % 5);
	}
	NextAimEmitterIndex = 0;

  // Initialize spin emitters
  for (i = 0; i < SPIN_EMITTER_COUNT; i++) { 
    SpinEmitters[i].emitterIndex = -1;
    EmitterTimes[i].period = 0.4f;
    SpinEmitters[i].spinSpeed = 60.f;
    EmitterTimes[i].t = 0.f + (guRandom() % 5);
  }
  NextSpinEmitterIndex = 0;

  BossSetting = NO_BOSS_SET;
  boss_x = 0.f;
  boss_y = 0.f;
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
    EmitterTimes[newEmitterIndex].t = 0.f;
    EmitterTimes[newEmitterIndex].period = 2.f;
    NextEmitterIndex++;

    AimEmitters[newAimEmitterIndex].emitterIndex = newEmitterIndex;
    NextAimEmitterIndex++;

	return newAimEmitterIndex;
}

int generateSpinEmitterEntity(float x, float y) {
    int newEmitterIndex = NextEmitterIndex;
    int newSpinEmitterIndex = NextSpinEmitterIndex;

    if (newEmitterIndex == EMITTER_COUNT) {
      return -1;
    }

    if (newSpinEmitterIndex == SPIN_EMITTER_COUNT) {
      return -2;
    }

    EmitterStates[newEmitterIndex] = EMITTER_ALIVE;
    EmitterPositions[newEmitterIndex].x = x;
    EmitterPositions[newEmitterIndex].y = y;
    EmitterVelocities[newEmitterIndex].x = 0.f;
    EmitterVelocities[newEmitterIndex].y = 0.f;
    EmitterTimes[newEmitterIndex].t = 0.f;
    EmitterTimes[newEmitterIndex].period = 0.4f;
    NextEmitterIndex++;

    SpinEmitters[newSpinEmitterIndex].emitterIndex = newEmitterIndex;
    SpinEmitters[newSpinEmitterIndex].totalTime = 0;
    NextSpinEmitterIndex++;

  return newSpinEmitterIndex;
}

int generateBossA(float x, float y) {
  if (BossSetting) {
    return 0;
  }

  BossSetting = BOSS_A_SET;
  boss_x = x;
  boss_y = y;
  boss_starting_x = x;
  boss_starting_y = y;
  boss_rotation = 180;

  return 1;
}

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

    EmitterTimes[i].t += deltaSeconds;
    if (EmitterTimes[i].t < EmitterTimes[i].period) {
      continue;
    }

    // If we've made it here, fire
    EmitterTimes[i].t = 0;

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

  // Update spin emitters
  for (i = 0; i < SPIN_EMITTER_COUNT; i++) {
    int newBulletIndex;
    float theta;
    Position* bulletPosition = NULL;
    Velocity* bulletVelocity = NULL;

    if (SpinEmitters[i].emitterIndex == -1) {
      continue;
    }

    if ((EmitterStates[SpinEmitters[i].emitterIndex]) == EMITTER_DEAD) {
      SpinEmitters[i].emitterIndex = -1;
      continue;
    }

    // if we're really far away, don't worry about updating for now
    if ((fabs_d(player_y - EmitterPositions[SpinEmitters[i].emitterIndex].y) > (RENDER_DISTANCE * 2)) || (fabs_d(player_x - EmitterPositions[SpinEmitters[i].emitterIndex].x) > (RENDER_DISTANCE * 2))) {
      continue;
    }

    EmitterTimes[i].t += deltaSeconds;
    SpinEmitters[i].totalTime += deltaSeconds;
    if (EmitterTimes[i].t < EmitterTimes[i].period) {
      continue;
    }

    // If we've made it here, fire
    EmitterTimes[i].t = 0;

    // Skip if there are no available bullets
    newBulletIndex = consumeNextBullet();
    if (newBulletIndex == -1) {
      continue;
    }
    bulletPosition = getBulletPosition(newBulletIndex);
    bulletVelocity = getBulletVelocity(newBulletIndex);
    setBulletState(newBulletIndex, 1);
    bulletPosition->x = EmitterPositions[SpinEmitters[i].emitterIndex].x;
    bulletPosition->y = EmitterPositions[SpinEmitters[i].emitterIndex].y;
    bulletVelocity->x = 8.4f * cosf(SpinEmitters[i].totalTime * SpinEmitters[i].spinSpeed);
    bulletVelocity->y = 8.4f * sinf(SpinEmitters[i].totalTime * SpinEmitters[i].spinSpeed);
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

void addBossDisplayList(Dynamic* dynamicp) {
  int i;
  float midAShiftX;
  float midAShiftY;
  float midBShiftX;
  float midBShiftY;
  float midCShiftX;
  float midCShiftY;
  float midDShiftX;
  float midDShiftY;

  // TODO: load proper boss scaling

  guTranslate(&(dynamicp->bossTranslate), boss_x, boss_y, 2.f);
  guRotate(&(dynamicp->bossRotate), boss_rotation, 0.f, 0.f, 1.f);
  guScale(&(dynamicp->bossScale), 0.01, 0.01, 0.01);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->bossTranslate)), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->bossRotate)), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->bossScale)), G_MTX_NOPUSH | G_MTX_MODELVIEW);

  gSPVertex(glistp++,&(test_boss_main_geo[0]), 45, 0);
  gSP2Triangles(glistp++, 20, 2, 22, 0, 23, 0, 3, 0);
  gSP2Triangles(glistp++, 28, 31, 7, 0, 23, 5, 0, 0);
  gSP2Triangles(glistp++, 4, 3, 0, 0, 20, 3, 1, 0);
  gSP2Triangles(glistp++, 3, 10, 2, 0, 4, 0, 8, 0);
  gSP2Triangles(glistp++, 6, 8, 9, 0, 4, 8, 6, 0);
  gSP2Triangles(glistp++, 6, 31, 26, 0, 6, 9, 31, 0);
  gSP2Triangles(glistp++, 5, 28, 7, 0, 5, 8, 0, 0);
  gSP2Triangles(glistp++, 34, 22, 2, 0, 2, 10, 34, 0);
  gSP2Triangles(glistp++, 10, 4, 36, 0, 26, 36, 6, 0);
  gSP2Triangles(glistp++, 6, 36, 4, 0, 38, 12, 11, 0);
  gSP2Triangles(glistp++, 14, 15, 13, 0, 20, 1, 2, 0);
  gSP2Triangles(glistp++, 23, 21, 5, 0, 20, 23, 3, 0);
  gSP2Triangles(glistp++, 2, 1, 3, 0, 3, 4, 10, 0);
  gSP2Triangles(glistp++, 5, 21, 28, 0, 5, 7, 8, 0);
  gSP2Triangles(glistp++, 38, 40, 12, 0, 14, 16, 15, 0);
  gSP2Triangles(glistp++, 20, 22, 19, 0, 23, 24, 17, 0);
  gSP2Triangles(glistp++, 28, 30, 31, 0, 23, 17, 27, 0);
  gSP2Triangles(glistp++, 25, 17, 24, 0, 20, 18, 24, 0);
  gSP2Triangles(glistp++, 24, 19, 35, 0, 25, 32, 17, 0);
  gSP2Triangles(glistp++, 29, 33, 32, 0, 25, 29, 32, 0);
  gSP2Triangles(glistp++, 29, 26, 31, 0, 29, 31, 33, 0);
  gSP2Triangles(glistp++, 27, 30, 28, 0, 27, 17, 32, 0);
  gSP2Triangles(glistp++, 34, 19, 22, 0, 19, 34, 35, 0);
  gSP2Triangles(glistp++, 35, 36, 25, 0, 26, 29, 36, 0);
  gSP2Triangles(glistp++, 29, 25, 36, 0, 38, 37, 39, 0);
  gSP2Triangles(glistp++, 42, 41, 43, 0, 20, 19, 18, 0);
  gSP2Triangles(glistp++, 23, 27, 21, 0, 20, 24, 23, 0);
  gSP2Triangles(glistp++, 19, 24, 18, 0, 24, 35, 25, 0);
  gSP2Triangles(glistp++, 27, 28, 21, 0, 27, 32, 30, 0);
  gSP2Triangles(glistp++, 38, 39, 40, 0, 42, 43, 44, 0);

  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

  midAShiftX = ((guRandom() % 100) - 50) * 0.01f;
  midAShiftY = ((guRandom() % 100) - 50) * 0.01f;
  midBShiftX = ((guRandom() % 100) - 50) * 0.01f;
  midBShiftY = ((guRandom() % 100) - 50) * 0.01f;
  midCShiftX = ((guRandom() % 100) - 50) * 0.01f;
  midCShiftY = ((guRandom() % 100) - 50) * 0.01f;
  midDShiftX = ((guRandom() % 100) - 50) * 0.01f;
  midDShiftY = ((guRandom() % 100) - 50) * 0.01f;

  guTranslate(&(dynamicp->bossHairMidTranslationA), 4.f + midAShiftX, -4.0f + midAShiftY, 4.f);
  guTranslate(&(dynamicp->bossHairMidTranslationB), -4.f + midBShiftX, -4.0f + midBShiftY, 4.f);
  guTranslate(&(dynamicp->bossHairMidTranslationC), -8.f + midCShiftX, 1.4f + midCShiftY, 4.f);
  guTranslate(&(dynamicp->bossHairMidTranslationD), 8.f + midDShiftX, 1.4f + midDShiftY, 4.f);
  guTranslate(&(dynamicp->bossHairTranslationA), 4.f - midAShiftX, -4.0f - midAShiftY, -6.5f);
  guTranslate(&(dynamicp->bossHairTranslationB), -4.f- midBShiftX, -4.0f - midBShiftY, -6.5f);
  guTranslate(&(dynamicp->bossHairTranslationC), -4.f- midCShiftX, 2.0f - midCShiftY, -6.5f);
  guTranslate(&(dynamicp->bossHairTranslationD), 4.f- midDShiftX, 2.0f - midDShiftY, -6.5f);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->bossHairMidTranslationA)), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPVertex(glistp++,&(test_boss_hair_geo[0]), 8, 46);
  for (i = 0; i < 8; i++) {
    gSP2Triangles(glistp++, 20, 25, 46 + i, 0, 20, 23, 46 + i, 0);
    gSP2Triangles(glistp++, 20, 22, 46 + i, 0, 22, 23, 46 + i, 0);
    gSP2Triangles(glistp++, 23, 22, 46 + i, 0, 22, 24, 46 + i, 0);
  }

  gSP2Triangles(glistp++, 0 + 46, 1 + 46, 2 + 46, 0, 0 + 46, 2 + 46, 3 + 46, 0);
  gSP2Triangles(glistp++, 1 + 46, 0 + 46, 4 + 46, 0, 1 + 46, 4 + 46, 5 + 46, 0);
  gSP2Triangles(glistp++, 2 + 46, 3 + 46, 5 + 46, 0, 2 + 46, 5 + 46, 6 + 46, 0);
  gSP2Triangles(glistp++, 0 + 46, 5 + 46, 4 + 46, 0, 0 + 46, 5 + 46, 6 + 46, 0);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->bossHairTranslationA)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  gSPVertex(glistp++,&(test_boss_hair_geo[0]), 8, 54);
  for (i = 0; i < 8; i++) {
    gSP2Triangles(glistp++, 46, 47, 54 + i, 0, 48, 49, 54 + i, 0);
    gSP2Triangles(glistp++, 50, 51, 54 + i, 0, 52, 53, 54 + i, 0);
  }
  gSP2Triangles(glistp++, 0 + 54, 1 + 54, 2 + 54, 0, 0 + 54, 2 + 54, 3 + 54, 0);
  gSP2Triangles(glistp++, 1 + 54, 0 + 54, 4 + 54, 0, 1 + 54, 4 + 54, 5 + 54, 0);
  gSP2Triangles(glistp++, 2 + 54, 3 + 54, 5 + 54, 0, 2 + 54, 5 + 54, 6 + 54, 0);
  gSP2Triangles(glistp++, 0 + 54, 5 + 54, 4 + 54, 0, 0 + 54, 5 + 54, 6 + 54, 0);
  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
  
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->bossHairMidTranslationB)), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPVertex(glistp++,&(test_boss_hair_geo[0]), 8, 46);
  for (i = 0; i < 8; i++) {
    gSP2Triangles(glistp++, 20, 25, 46 + i, 0, 20, 23, 46 + i, 0);
    gSP2Triangles(glistp++, 20, 22, 46 + i, 0, 22, 23, 46 + i, 0);
    gSP2Triangles(glistp++, 23, 22, 46 + i, 0, 22, 24, 46 + i, 0);
  }

  gSP2Triangles(glistp++, 0 + 46, 1 + 46, 2 + 46, 0, 0 + 46, 2 + 46, 3 + 46, 0);
  gSP2Triangles(glistp++, 1 + 46, 0 + 46, 4 + 46, 0, 1 + 46, 4 + 46, 5 + 46, 0);
  gSP2Triangles(glistp++, 2 + 46, 3 + 46, 5 + 46, 0, 2 + 46, 5 + 46, 6 + 46, 0);
  gSP2Triangles(glistp++, 0 + 46, 5 + 46, 4 + 46, 0, 0 + 46, 5 + 46, 6 + 46, 0);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->bossHairTranslationB)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  gSPVertex(glistp++,&(test_boss_hair_geo[0]), 8, 54);
  for (i = 0; i < 8; i++) {
    gSP2Triangles(glistp++, 46, 47, 54 + i, 0, 48, 49, 54 + i, 0);
    gSP2Triangles(glistp++, 50, 51, 54 + i, 0, 52, 53, 54 + i, 0);
  }
  gSP2Triangles(glistp++, 0 + 54, 1 + 54, 2 + 54, 0, 0 + 54, 2 + 54, 3 + 54, 0);
  gSP2Triangles(glistp++, 1 + 54, 0 + 54, 4 + 54, 0, 1 + 54, 4 + 54, 5 + 54, 0);
  gSP2Triangles(glistp++, 2 + 54, 3 + 54, 5 + 54, 0, 2 + 54, 5 + 54, 6 + 54, 0);
  gSP2Triangles(glistp++, 0 + 54, 5 + 54, 4 + 54, 0, 0 + 54, 5 + 54, 6 + 54, 0);
  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
  
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->bossHairMidTranslationC)), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPVertex(glistp++,&(test_boss_hair_geo[0]), 8, 46);
  for (i = 0; i < 8; i++) {
    gSP2Triangles(glistp++, 20, 4, 46 + i, 0, 20, 23, 46 + i, 0);
    gSP2Triangles(glistp++, 20, 22, 46 + i, 0, 22, 23, 46 + i, 0);
    gSP2Triangles(glistp++, 23, 22, 46 + i, 0, 22, 3, 46 + i, 0);
  }

  gSP2Triangles(glistp++, 0 + 46, 1 + 46, 2 + 46, 0, 0 + 46, 2 + 46, 3 + 46, 0);
  gSP2Triangles(glistp++, 1 + 46, 0 + 46, 4 + 46, 0, 1 + 46, 4 + 46, 5 + 46, 0);
  gSP2Triangles(glistp++, 2 + 46, 3 + 46, 5 + 46, 0, 2 + 46, 5 + 46, 6 + 46, 0);
  gSP2Triangles(glistp++, 0 + 46, 5 + 46, 4 + 46, 0, 0 + 46, 5 + 46, 6 + 46, 0);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->bossHairTranslationC)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  gSPVertex(glistp++,&(test_boss_hair_geo[0]), 8, 54);
  for (i = 0; i < 8; i++) {
    gSP2Triangles(glistp++, 46, 47, 54 + i, 0, 48, 49, 54 + i, 0);
    gSP2Triangles(glistp++, 50, 51, 54 + i, 0, 52, 53, 54 + i, 0);
  }
  gSP2Triangles(glistp++, 0 + 54, 1 + 54, 2 + 54, 0, 0 + 54, 2 + 54, 3 + 54, 0);
  gSP2Triangles(glistp++, 1 + 54, 0 + 54, 4 + 54, 0, 1 + 54, 4 + 54, 5 + 54, 0);
  gSP2Triangles(glistp++, 2 + 54, 3 + 54, 5 + 54, 0, 2 + 54, 5 + 54, 6 + 54, 0);
  gSP2Triangles(glistp++, 0 + 54, 5 + 54, 4 + 54, 0, 0 + 54, 5 + 54, 6 + 54, 0);
  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
  
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->bossHairMidTranslationD)), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPVertex(glistp++,&(test_boss_hair_geo[0]), 8, 46);
  for (i = 0; i < 8; i++) {
    gSP2Triangles(glistp++, 20, 47, 46 + i, 0, 20, 23, 46 + i, 0);
    gSP2Triangles(glistp++, 20, 22, 46 + i, 0, 22, 23, 46 + i, 0);
    gSP2Triangles(glistp++, 23, 22, 46 + i, 0, 22, 24, 46 + i, 0);
  }

  gSP2Triangles(glistp++, 0 + 46, 1 + 46, 2 + 46, 0, 0 + 46, 2 + 46, 3 + 46, 0);
  gSP2Triangles(glistp++, 1 + 46, 0 + 46, 4 + 46, 0, 1 + 46, 4 + 46, 5 + 46, 0);
  gSP2Triangles(glistp++, 2 + 46, 3 + 46, 5 + 46, 0, 2 + 46, 5 + 46, 6 + 46, 0);
  gSP2Triangles(glistp++, 0 + 46, 5 + 46, 4 + 46, 0, 0 + 46, 5 + 46, 6 + 46, 0);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->bossHairTranslationD)), G_MTX_NOPUSH | G_MTX_MODELVIEW);
  gSPVertex(glistp++,&(test_boss_hair_geo[0]), 8, 54);
  for (i = 0; i < 8; i++) {
    gSP2Triangles(glistp++, 46, 47, 54 + i, 0, 48, 49, 54 + i, 0);
    gSP2Triangles(glistp++, 50, 51, 54 + i, 0, 52, 53, 54 + i, 0);
  }
  gSP2Triangles(glistp++, 0 + 54, 1 + 54, 2 + 54, 0, 0 + 54, 2 + 54, 3 + 54, 0);
  gSP2Triangles(glistp++, 1 + 54, 0 + 54, 4 + 54, 0, 1 + 54, 4 + 54, 5 + 54, 0);
  gSP2Triangles(glistp++, 2 + 54, 3 + 54, 5 + 54, 0, 2 + 54, 5 + 54, 6 + 54, 0);
  gSP2Triangles(glistp++, 0 + 54, 5 + 54, 4 + 54, 0, 0 + 54, 5 + 54, 6 + 54, 0);
  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

}

void tickBossA(float deltaSeconds, float player_x, float player_y) {
  //
}

void tickBoss(float deltaSeconds, float player_x, float player_y) {
  if (BossSetting == NO_BOSS_SET) {
    return;
  }

  if (BossSetting == BOSS_A_SET) {
    tickBossA(deltaSeconds, player_x, player_y);
  }
}

void renderBossA(Dynamic* dynamic) {
  addBossDisplayList(dynamic);
}

void renderBoss(Dynamic* dynamic) {
  if (BossSetting == NO_BOSS_SET) {
    return;
  }

  if (BossSetting == BOSS_A_SET) {
    renderBossA(dynamic);
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





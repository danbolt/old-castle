#include <assert.h>
#include <nusys.h>
#include "main.h"
#include "graphic.h"
#include <gu.h>

#define PLAYER_MOVE_SPEED 0.003f
#define DEFAULT_TARGET_DISTANCE 8.6f
#define JUMP_SPEED_PER_TICK 0.0513f
#define LAND_SPEED_PER_TICK 0.3f

typedef enum {
  Move,
  Jumping,
  Landed
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

static float camera_rotation;
static float player_rotation;

static float camera_x; /* The display position-X */
static float camera_y; /* The display position-Y */

static int newTileX;
static int newTileY;

#define BULLET_COUNT 10

#define CAMERA_MOVE_SPEED 0.01726f
#define CAMERA_TURN_SPEED 0.03826f
#define CAMERA_DISTANCE 8.3f

#define MAP_SIZE 100
#define TILE_SIZE 2
#define INV_TILE_SIZE (1.0f / TILE_SIZE)

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

static Position BulletPositions[BULLET_COUNT];
static Velocity BulletVelocities[BULLET_COUNT];
static EntityTransform BulletMatricies[BULLET_COUNT];

static int MapInfo[MAP_SIZE * MAP_SIZE]; // 0 for empty, 1 for filled
static Vtx map_geom[MAP_SIZE * MAP_SIZE * 4];

#define IS_TILE_BLOCKED(x, y) MapInfo[x + (y * MAP_SIZE)]

static Vtx bullet_test_geom[] =  {
        {         1,  1, 1, 0, 0, 0, 0xff, 0, 0, 0xff },
        {        -1,  1, 1, 0, 0, 0, 0xff, 0, 0, 0xff },
        {        -1, -1, 1, 0, 0, 0, 0xff, 0, 0, 0xff },
        {         1, -1, 1, 0, 0, 0, 0xff, 0, 0, 0xff },
        {        -1,  -1,  1, 0, 0, 0, 0,    0xff, 0,    0xff },
        {        -1,   1,  1, 0, 0, 0, 0,    0xff, 0, 0 },
        {        -1,   1, -1, 0, 0, 0, 0,    0xff, 0xff, 0xff },
        {        -1,  -1, -1, 0, 0, 0, 0xff, 0xff, 0,    0xff },
        {        1,  1, 1, 0, 0, 0, 0, 0, 0, 0xff     },
        {        1,  -1, 1, 0, 0, 0, 0, 0xff, 0, 0xff  },
        {        1, -1, -1, 0, 0, 0, 0xff, 0xff, 0, 0xff },
        {        1, 1, -1, 0, 0, 0, 0, 0xff, 0xff, 0xff },
        {        -1,  1, 1, 0, 0, 0, 0, 0, 0xff, 0xff },
        {         1,  1, 1, 0, 0, 0, 0, 0, 0xff, 0xff },
        {         1, -1, 1, 0, 0, 0, 0, 0, 0xff, 0xff },
        {        -1, -1, 1, 0, 0, 0, 0, 0, 0xff, 0xff },
        {        -1,  1, 1, 0, 0, 0, 0, 0xff, 0xff, 0xff },
        {         1,  1, 1, 0, 0, 0, 0, 0xff, 0xff, 0xff },
        {         1,  1, -1, 0, 0, 0, 0, 0xff, 0xff, 0xff },
        {        -1,  1, -1, 0, 0, 0, 0, 0xff, 0xff, 0xff },
        {         1,  -1, 1, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
        {        -1,  -1, 1, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
        {        -1,  -1, -1, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
        {         1,  -1, -1, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
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

// TODO: ensure this is all correct statically
void updateMapFromInfo() {
  int i;

  for (i = 0; i < MAP_SIZE * MAP_SIZE; i++) {
    short x = i % MAP_SIZE;
    short y = i / MAP_SIZE;

    map_geom[(i * 4) + 0].v.ob[0] = (x * TILE_SIZE);
    map_geom[(i * 4) + 0].v.ob[1] = (y * TILE_SIZE);
    map_geom[(i * 4) + 0].v.ob[2] = (MapInfo[i] == 0) ? -1 : 1;
    map_geom[(i * 4) + 0].v.flag = 0;
    map_geom[(i * 4) + 0].v.tc[0] = 0;
    map_geom[(i * 4) + 0].v.tc[1] = 0;
    map_geom[(i * 4) + 0].v.cn[0] = (MapInfo[i] == 0) ? 0xaa : 0x11;
    map_geom[(i * 4) + 0].v.cn[1] = (MapInfo[i] == 0) ? 0x88 : 0x44;
    map_geom[(i * 4) + 0].v.cn[2] = (MapInfo[i] == 0) ? 0x99 : 0x12;
    map_geom[(i * 4) + 0].v.cn[3] = (MapInfo[i] == 0) ? 0x44 : 0x01;

    map_geom[(i * 4) + 1].v.ob[0] = (x * TILE_SIZE) + TILE_SIZE;
    map_geom[(i * 4) + 1].v.ob[1] = (y * TILE_SIZE);
    map_geom[(i * 4) + 1].v.ob[2] = (MapInfo[i] == 0) ? -1 : 1;
    map_geom[(i * 4) + 1].v.flag = 0;
    map_geom[(i * 4) + 1].v.tc[0] = 0;
    map_geom[(i * 4) + 1].v.tc[1] = 0;
    map_geom[(i * 4) + 1].v.cn[0] = (MapInfo[i] == 0) ? 0xaa : 0x11;
    map_geom[(i * 4) + 1].v.cn[1] = (MapInfo[i] == 0) ? 0x88 : 0x44;
    map_geom[(i * 4) + 1].v.cn[2] = (MapInfo[i] == 0) ? 0x99 : 0x12;
    map_geom[(i * 4) + 1].v.cn[3] = (MapInfo[i] == 0) ? 0x44 : 0x01;

    map_geom[(i * 4) + 2].v.ob[0] = (x * TILE_SIZE) + TILE_SIZE;
    map_geom[(i * 4) + 2].v.ob[1] = (y * TILE_SIZE) + TILE_SIZE;
    map_geom[(i * 4) + 2].v.ob[2] = (MapInfo[i] == 0) ? -1 : 1;
    map_geom[(i * 4) + 2].v.flag = 0;
    map_geom[(i * 4) + 2].v.tc[0] = 0;
    map_geom[(i * 4) + 2].v.tc[1] = 0;
    map_geom[(i * 4) + 2].v.cn[0] = (MapInfo[i] == 0) ? 0xaa : 0x11;
    map_geom[(i * 4) + 2].v.cn[1] = (MapInfo[i] == 0) ? 0x88 : 0x44;
    map_geom[(i * 4) + 2].v.cn[2] = (MapInfo[i] == 0) ? 0x99 : 0x12;
    map_geom[(i * 4) + 2].v.cn[3] = (MapInfo[i] == 0) ? 0x44 : 0x01;

    map_geom[(i * 4) + 3].v.ob[0] = (x * TILE_SIZE);
    map_geom[(i * 4) + 3].v.ob[1] = (y * TILE_SIZE) + TILE_SIZE;
    map_geom[(i * 4) + 3].v.ob[2] = (MapInfo[i] == 0) ? -1 : 1;
    map_geom[(i * 4) + 3].v.flag = 0;
    map_geom[(i * 4) + 3].v.tc[0] = 0;
    map_geom[(i * 4) + 3].v.tc[1] = 0;
    map_geom[(i * 4) + 3].v.cn[0] = (MapInfo[i] == 0) ? 0xaa : 0x11;
    map_geom[(i * 4) + 3].v.cn[1] = (MapInfo[i] == 0) ? 0x88 : 0x44;
    map_geom[(i * 4) + 3].v.cn[2] = (MapInfo[i] == 0) ? 0x99 : 0x12;
    map_geom[(i * 4) + 3].v.cn[3] = (MapInfo[i] == 0) ? 0x44 : 0x01;
  }
}

/* The initialization of stage 0 */
void initStage00(void)
{
  int i;
  int j;

  player_x = 6.0f;
  player_y = 6.0f;
  target_distance = DEFAULT_TARGET_DISTANCE;
  player_state = Move;
  player_t = 0.f;

  camera_x = 0.0f;
  camera_y = 0.0f;

  camera_rotation = 0.1f;
  player_rotation = 0.f;

  for (i = 0; i < BULLET_COUNT; i++) {
    float r = i / (float)BULLET_COUNT * M_PI * 2.f;

    BulletPositions[i].x = cosf(r) * 1.f;
    BulletPositions[i].y = sinf(r) * 1.f;

    BulletVelocities[i].x = cosf(r) * 0.05f;
    BulletVelocities[i].y = sinf(r) * 0.05f;

    guMtxIdent(&(BulletMatricies[i].mat));   
  }

  for (i = 0; i < MAP_SIZE; i++) {
    for (j = 0; j < MAP_SIZE; j++) {
      int roll = guRandom() % 10;

      MapInfo[j * MAP_SIZE + i] = (roll == 0) ? 1 : 0;
    }
  }
  updateMapFromInfo();

}

void addCubeToDisplayList()
{
  gSPVertex(glistp++,&(bullet_test_geom[0]), 4, 0);
  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
  gSPVertex(glistp++,&(bullet_test_geom[4]), 4, 0);
  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
  gSPVertex(glistp++,&(bullet_test_geom[8]), 4, 0);
  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
  gSPVertex(glistp++,&(bullet_test_geom[12]), 4, 0);
  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
  gSPVertex(glistp++,&(bullet_test_geom[16]), 4, 0);
  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
  gSPVertex(glistp++,&(bullet_test_geom[20]), 4, 0);
  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
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
  Mtx targetTranslation;
  Mtx targetScale;

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
  guPerspective(&dynamicp->projection, &perspNorm, 45.0f, (float)SCREEN_WD/(float)SCREEN_HT, 10.0f, 100.0f, 1.0f);
  guLookAt(&dynamicp->viewing, camera_x + (cosf(camera_rotation - (M_PI * 0.5f) ) * CAMERA_DISTANCE), camera_y + (sinf(camera_rotation - (M_PI * 0.5f) ) * CAMERA_DISTANCE), 26.8365f, camera_x, camera_y, 0.0f, 0.0f, 0.0f, 1.0f);

  gSPPerspNormalize(glistp++, perspNorm);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->projection)), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->viewing)), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++, G_CYC_1CYCLE);
  gDPSetRenderMode(glistp++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
  gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
  gSPSetGeometryMode(glistp++, G_ZBUFFER | G_CULL_BACK | G_SHADE);

  // Render Player
  guTranslate(&(playerTranslation), player_x, player_y, ((player_state == Jumping) ? (sinf(player_t * M_PI) * 3.2f) : 0.f));
  guRotate(&(playerRotation), player_rotation / M_PI * 180.f, 0.0f, 0.0f, 1.0f);
  guRotate(&(playerJumpRotation), ((player_state == Jumping) ? (sinf(player_t * M_PI * 2.f) * 60.2f) : 0.f), 0.0f, 1.0f, 0.0f);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(playerTranslation)), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(playerRotation)), G_MTX_PUSH | G_MTX_MODELVIEW);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(playerJumpRotation)), G_MTX_NOPUSH | G_MTX_MODELVIEW);

  addCubeToDisplayList();

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
  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

  gDPPipeSync(glistp++);

  // Render Bullets
  for (i = 0; i < BULLET_COUNT; i++) {
    guTranslate(&(BulletMatricies[i]), BulletPositions[i].x, BulletPositions[i].y, 0.f);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(BulletMatricies[i])), G_MTX_PUSH | G_MTX_MODELVIEW);

    addCubeToDisplayList();

    gDPPipeSync(glistp++);

    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
  }

  // Render map tiles
  for (i = MAX(0, (int)((camera_x / TILE_SIZE) - 11)); i < MIN(MAP_SIZE, (int)((camera_x / TILE_SIZE) + 11)); i++) {
    for (j = MAX(0, (int)((camera_y / TILE_SIZE) - 11)); j < MIN(MAP_SIZE, (int)((camera_y / TILE_SIZE) + 11)); j++) {
      gSPVertex(glistp++,&(map_geom[((j * MAP_SIZE) + i) * 4]), 4, 0);
      gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
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

  if(contPattern & 0x1)
  {

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
  nuDebConDisp(NU_SC_SWAPBUFFER);

  /* Switch display list buffers */
  gfx_gtask_no ^= 1;
}

float lerp(float v0, float v1, float t) {
  return (1 - t) * v0 + t * v1;
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

float fabs_d(float x) {
  if (x < 0.f) {
    return -x;
  }
  return x;
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
      player_rotation = lerp(player_rotation, playerStickRot, 0.18f);
    }

    newX = player_x + player_facing_x * PLAYER_MOVE_SPEED;
    newY = player_y + player_facing_y * PLAYER_MOVE_SPEED;

    if (contdata[0].trigger & A_BUTTON) {
      player_state = Jumping;
      player_t = 0.f;
      player_jump_x = player_x;
      player_jump_y = player_y;
    }
  } else if (player_state == Jumping) {
    newX = player_jump_x + cosf(player_rotation) * player_t * target_distance;
    newY = player_jump_y + sinf(player_rotation) * player_t * target_distance;

    player_t += JUMP_SPEED_PER_TICK;
    if (player_t > 1.f) {
      player_state = Landed;
      player_t = 0.f;
    }
  } else if (player_state == Landed) {
    newX = player_x;
    newY = player_y;

    player_t += LAND_SPEED_PER_TICK;
    if (player_t > 2.2f) {
      player_state = Move;
    }
  }

  newTileX = (int)(newX * INV_TILE_SIZE);
  newTileY = (int)(newY * INV_TILE_SIZE);

  // step x
  if ((newTileX < (MAP_SIZE * TILE_SIZE)) && (newTileX >= 0) && (IS_TILE_BLOCKED(newTileX, (int)(player_y * INV_TILE_SIZE)))) {
    newX = player_x;
  }
  player_x = newX;

  // step y
  if ((newTileY < (MAP_SIZE * TILE_SIZE)) && (newTileY >= 0) && (IS_TILE_BLOCKED((int)(player_x * INV_TILE_SIZE), newTileY))) {
    newY = player_y;
  }
  player_y = newY;

  // Lerp the camera
  camera_x = lerp(camera_x, player_x, 0.13f);
  camera_y = lerp(camera_y, player_y, 0.13f);
  
  for (i = 0; i < BULLET_COUNT; i++) {
    BulletPositions[i].x += BulletVelocities[i].x;
    BulletPositions[i].y += BulletVelocities[i].y;
  }
  
}

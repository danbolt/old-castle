#include <assert.h>
#include <nusys.h>
#include "main.h"
#include "graphic.h"
#include <gu.h>

static float player_x; /* The display position-X */
static float player_y; /* The display position-Y */

static float camera_rotation;

static float camera_x; /* The display position-X */
static float camera_y; /* The display position-Y */

#define BULLET_COUNT 16

#define CAMERA_MOVE_SPEED 0.01726f
#define CAMERA_TURN_SPEED 0.03826f
#define CAMERA_DISTANCE 51.f

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

/* The vertex coordinate */
static Vtx shade_vtx[] =  {
        {        -4,  4, -5, 0, 0, 0, 0, 0xff, 0, 0xff  },
        {         4,  4, -5, 0, 0, 0, 0, 0, 0, 0xff     },
        {         4, -4, -5, 0, 0, 0, 0, 0xff, 0xff, 0xff },
        {        -4, -4, -5, 0, 0, 0, 0xff, 0xff, 0, 0xff },
};

static Vtx bullet_test_geom[] =  {
        {         4,  4, 4, 0, 0, 0, 0xff, 0, 0, 0xff },
        {        -4,  4, 4, 0, 0, 0, 0xff, 0, 0, 0xff },
        {        -4, -4, 4, 0, 0, 0, 0xff, 0, 0, 0xff },
        {         4, -4, 4, 0, 0, 0, 0xff, 0, 0, 0xff },
        {        -4,  -4,  4, 0, 0, 0, 0,    0xff, 0,    0xff },
        {        -4,   4,  4, 0, 0, 0, 0,    0xff, 0, 0 },
        {        -4,   4, -4, 0, 0, 0, 0,    0xff, 0xff, 0xff },
        {        -4,  -4, -4, 0, 0, 0, 0xff, 0xff, 0,    0xff },
        {        4,  4, 4, 0, 0, 0, 0, 0, 0, 0xff     },
        {        4,  -4, 4, 0, 0, 0, 0, 0xff, 0, 0xff  },
        {        4, -4, -4, 0, 0, 0, 0xff, 0xff, 0, 0xff },
        {        4, 4, -4, 0, 0, 0, 0, 0xff, 0xff, 0xff },
        {        -4,  4, 4, 0, 0, 0, 0, 0, 0xff, 0xff },
        {         4,  4, 4, 0, 0, 0, 0, 0, 0xff, 0xff },
        {         4, -4, 4, 0, 0, 0, 0, 0, 0xff, 0xff },
        {        -4, -4, 4, 0, 0, 0, 0, 0, 0xff, 0xff },
        {        -4,  4, 4, 0, 0, 0, 0, 0xff, 0xff, 0xff },
        {         4,  4, 4, 0, 0, 0, 0, 0xff, 0xff, 0xff },
        {         4,  4, -4, 0, 0, 0, 0, 0xff, 0xff, 0xff },
        {        -4,  4, -4, 0, 0, 0, 0, 0xff, 0xff, 0xff },
        {         4,  -4, 4, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
        {        -4,  -4, 4, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
        {        -4,  -4, -4, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
        {         4,  -4, -4, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
};

/* The initialization of stage 0 */
void initStage00(void)
{
  int i;

  player_x = 0.0f;
  player_y = 0.0f;

  camera_x = 0.0f;
  camera_y = 0.0f;

  camera_rotation = 0.f;

  for (i = 0; i < BULLET_COUNT; i++) {
    float r = i / (float)BULLET_COUNT * M_PI * 2.f;

    BulletPositions[i].x = cosf(r) * 1.f;
    BulletPositions[i].y = sinf(r) * 1.f;

    BulletVelocities[i].x = cosf(r) * 0.05f;
    BulletVelocities[i].y = sinf(r) * 0.05f;

    guMtxIdent(&(BulletMatricies[i].mat));   
  }

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
  Mtx playerTranslation;
  Mtx playerRotation;

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
  guLookAt(&dynamicp->viewing, camera_x + (cosf(camera_rotation - (M_PI * 0.5f) ) * CAMERA_DISTANCE), camera_y + (sinf(camera_rotation - (M_PI * 0.5f) ) * CAMERA_DISTANCE), 50.f, camera_x, camera_y, 0.0f, 0.0f, 0.0f, 1.0f);

  gSPPerspNormalize(glistp++, perspNorm);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->projection)), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->viewing)), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++, G_CYC_1CYCLE);
  gDPSetRenderMode(glistp++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
  gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
  gSPSetGeometryMode(glistp++, G_ZBUFFER | G_CULL_BACK | G_SHADE);

  // Render Player
  guTranslate(&(playerTranslation), player_x, player_y, 0.f);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(playerTranslation)), G_MTX_PUSH | G_MTX_MODELVIEW);

  addCubeToDisplayList();

  gDPPipeSync(glistp++);

  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
  gSPPopMatrix(glistp++, G_MTX_MODELVIEW);

  // Render Bullets
  for (i = 0; i < BULLET_COUNT; i++) {
    guTranslate(&(BulletMatricies[i]), BulletPositions[i].x, BulletPositions[i].y, 0.f);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(BulletMatricies[i])), G_MTX_PUSH | G_MTX_MODELVIEW);

    addCubeToDisplayList();

    gDPPipeSync(glistp++);

    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
  }

  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);

  assert((glistp - gfx_glist[gfx_gtask_no]) < GFX_GLIST_LEN);

  /* Activate the task and 
     switch display buffers */
  nuGfxTaskStart(&gfx_glist[gfx_gtask_no][0],
		 (s32)(glistp - gfx_glist[gfx_gtask_no]) * sizeof (Gfx),
		 NU_GFX_UCODE_F3DEX , NU_SC_NOSWAPBUFFER);

  if(contPattern & 0x1)
  {

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

/* The game progressing process for stage 0 */
void updateGame00(void)
{  
  int i;
  float cosCamRot;
  float sinCamRot;
  float deltaX;
  float deltaY;
  float rotatedX;
  float rotatedY;

  /* Data reading of controller 1 */
  nuContDataGetEx(contdata,0);

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
  deltaX = contdata->stick_x * 0.03f;
  deltaY = contdata->stick_y * 0.03f;
  rotatedX = (deltaX * cosCamRot) + (deltaY * sinCamRot);
  rotatedY = (-deltaX * sinCamRot) + (deltaY * cosCamRot);

  player_x += rotatedX;
  player_y += rotatedY;

  // Lerp the camera
  camera_x = lerp(camera_x, player_x, 0.13f);
  camera_y = lerp(camera_y, player_y, 0.13f);
  
  for (i = 0; i < BULLET_COUNT; i++) {
    BulletPositions[i].x += BulletVelocities[i].x;
    BulletPositions[i].y += BulletVelocities[i].y;
  }
  
}

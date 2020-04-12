#include <assert.h>
#include <nusys.h>
#include "main.h"
#include "graphic.h"
#include <gu.h>

static float camera_x; /* The display position-X */
static float camera_y; /* The display position-Y */

#define BULLET_COUNT 16

#define CAMERA_MOVE_SPEED 0.01726f
#define CAMERA_DISTANCE -14.f

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
  int j;

  camera_x = 0.0;
  camera_y = 0.0;

  for (i = 0; i < BULLET_COUNT; i++) {
    float r = i / (float)BULLET_COUNT * M_PI * 2.f;

    BulletPositions[i].x = cosf(r) * 1.f;
    BulletPositions[i].y = sinf(r) * 1.f;

    BulletVelocities[i].x = cosf(r) * 0.05f;
    BulletVelocities[i].y = sinf(r) * 0.05f;

    guMtxIdent(&(BulletMatricies[i].mat));   
  }

}

/* Make the display list and activate the task */
void makeDL00(void)
{
  Dynamic* dynamicp;
  char conbuf[20]; 
  int i;

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
  guPerspective(&dynamicp->projection, &perspNorm, 60.0f, (float)SCREEN_WD/(float)SCREEN_HT, 10.0f, 100.0f, 1.0f);
  guLookAt(&dynamicp->viewing, camera_x, camera_y + CAMERA_DISTANCE, 50.f, camera_x, camera_y, 0.0f, 0.0f, 1.0f, 0.0f);

  gSPPerspNormalize(glistp++, perspNorm);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->projection)), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->viewing)), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++, G_CYC_1CYCLE);
  gDPSetRenderMode(glistp++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
  gSPClearGeometryMode(glistp++,0xFFFFFFFF);
  gSPSetGeometryMode(glistp++,G_ZBUFFER | G_CULL_BACK | G_SHADE);

  for (i = 0; i < BULLET_COUNT; i++) {
    guTranslate(&(BulletMatricies[i]), BulletPositions[i].x, BulletPositions[i].y, 0.f);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(BulletMatricies[i])), G_MTX_PUSH | G_MTX_MODELVIEW);

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
      /* Change character representation positions */
      nuDebConTextPos(0,12,23);
      sprintf(conbuf,"analog X=%5.1f",camera_x);
      nuDebConCPuts(0, conbuf);

      nuDebConTextPos(0,12,24);
      sprintf(conbuf,"analog Y=%5.1f",camera_y);
      nuDebConCPuts(0, conbuf);
    }
  else
    {
      nuDebConTextPos(0,9,24);
      nuDebConCPuts(0, "Controller1 not connect");
    }
    
  /* Display characters on the frame buffer */
  nuDebConDisp(NU_SC_SWAPBUFFER);

  /* Switch display list buffers */
  gfx_gtask_no ^= 1;
}


/* The game progressing process for stage 0 */
void updateGame00(void)
{  
  int i;

  /* Data reading of controller 1 */
  nuContDataGetEx(contdata,0);

  /* Change the display position by stick data */
  camera_x += contdata->stick_x * CAMERA_MOVE_SPEED;
  camera_y += contdata->stick_y * CAMERA_MOVE_SPEED;

  /* The reverse rotation by the A button */
  if(contdata[0].trigger & A_BUTTON)
  {
      //
  }

  
  for (i = 0; i < BULLET_COUNT; i++) {
    BulletPositions[i].x += BulletVelocities[i].x;
    BulletPositions[i].y += BulletVelocities[i].y;
  }
  
}

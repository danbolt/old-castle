

#include <nusys.h>
#include "main.h"
#include "graphic.h"
#include <gu.h>

static int note;

void initInterstitial(void) {
	note = 0;
}

static Vtx shade_vtx[] =  {
        {        -64,  64, -10, 0, 0, 0, 0, 0xff, 0, 0xff	},
        {         64,  64, -10, 0, 0, 0, 0, 0, 0, 0xff    	},
        {         64, -64, -10, 0, 0, 0, 0, 0, 0xff, 0xff	},
        {        -64, -64, -10, 0, 0, 0, 0xff, 0, 0, 0xff	},
};

void makeDLInsterstital(void) {
  Dynamic* dynamicp;
  char conbuf[20]; 

  /* Specify the display list buffer */
  dynamicp = &gfx_dynamic[gfx_gtask_no];
  glistp = &gfx_glist[gfx_gtask_no][0];

  /* Initialize RCP */
  gfxRCPInit();

  /* Clear the frame and Z-buffer */
  gfxClearCfb();

  guOrtho(&dynamicp->projection, -(float)SCREEN_WD/2.0F, (float)SCREEN_WD/2.0F, -(float)SCREEN_HT/2.0F, (float)SCREEN_HT/2.0F, 1.0F, 10.0F, 1.0F);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->projection)), G_MTX_PROJECTION|G_MTX_LOAD|G_MTX_NOPUSH);

  gSPVertex(glistp++,&(shade_vtx[0]),4, 0);

  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++,G_CYC_1CYCLE);
  gDPSetRenderMode(glistp++,G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
  gSPClearGeometryMode(glistp++,0xFFFFFFFF);
  gSPSetGeometryMode(glistp++,G_SHADE| G_SHADING_SMOOTH);

  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);

  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);


  nuGfxTaskStart(&gfx_glist[gfx_gtask_no][0],
		 (s32)(glistp - gfx_glist[gfx_gtask_no]) * sizeof (Gfx),
		 NU_GFX_UCODE_F3DEX , NU_SC_NOSWAPBUFFER);

  if(contPattern & 0x1)
    {
      nuDebConTextPos(0,6,6);
      sprintf(conbuf,"interstitial tick %d", note);
      nuDebConCPuts(0, conbuf);
    }
  else
    {
      nuDebConTextPos(0,9,24);
      nuDebConCPuts(0, "Controller1 not connect");
    }


  nuDebConDisp(NU_SC_SWAPBUFFER);

  gfx_gtask_no ^= 1;

}

void updateGameInterstital(void) {
  nuContDataGetEx(contdata,0);

  if ((contdata[0].trigger & START_BUTTON)) {
    resetStageFlag = 1;
    return;
  }

  note++;
}
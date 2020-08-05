

#include <nusys.h>
#include "main.h"
#include "graphic.h"
#include <gu.h>
#include "letters.h"
#include "dialogueline.h"
#include "segment.h"

static int note;

#define TRANSITION_IN_DURATION 0.9f

OSTime interStitialTime;
float interStitialDeltaSeconds;
float secondsSinceSceneStart;
float secondsSinceTransitionOut;
int transitioningOut;

extern DialogueLine* foyer_dialogues[];
extern const int foyer_dialogues_count;

DialogueLine* current;

float noiseHeightValue;

// Loads a collection of `DialogueLine`s from ROM, as defined by the spec file
void loadTextFromROM(void) {
	NUPiOverlaySegment	textSegmentDescription;

	textSegmentDescription.romStart 	= _foyer_dialoguesSegmentRomStart;
	textSegmentDescription.romEnd		= _foyer_dialoguesSegmentRomEnd;
	textSegmentDescription.ramStart	    = _foyer_dialoguesSegmentStart;
	textSegmentDescription.textStart	= _foyer_dialoguesSegmentTextStart;
	textSegmentDescription.textEnd		= _foyer_dialoguesSegmentTextEnd;
	textSegmentDescription.dataStart	= _foyer_dialoguesSegmentDataStart;
	textSegmentDescription.dataEnd		= _foyer_dialoguesSegmentDataEnd;
	textSegmentDescription.bssStart	    = _foyer_dialoguesSegmentBssStart;
	textSegmentDescription.bssEnd		= _foyer_dialoguesSegmentBssEnd;

	nuPiReadRomOverlay(&textSegmentDescription);
}


void initInterstitial(int randomIndex) {
	int i;

	note = 0;
	interStitialTime = OS_CYCLES_TO_USEC(osGetTime());
  	interStitialDeltaSeconds = 0.f;
  	secondsSinceSceneStart = 0;
  	noiseHeightValue = 1.f;
  	transitioningOut = 0;
  	secondsSinceTransitionOut = 0.f;

  	loadTextFromROM();

  	current = foyer_dialogues[randomIndex % foyer_dialogues_count];

	resetTextRequests();
}

static Vtx backing_verts[] =  {
        {        -SCREEN_WD/2.0f,  0, -5, 0, 0, 0, 0, 0, 0, 0	},
        {         SCREEN_WD/2.0f,  0, -5, 0, 0, 0, 0, 0, 0, 0    	},
        {         SCREEN_WD/2.0f, -(float)SCREEN_HT/2.0f, -5, 0, 0, 0, 0x69, 0x69, 0x69, 0xff	},
        {        -SCREEN_WD/2.0f, -(float)SCREEN_HT/2.0f, -5, 0, 0, 0, 0x69, 0x69, 0x69, 0xff	},
};

void makeDLInsterstital(void) {
  Dynamic* dynamicp;
  const float flicker = sinf(note * 0.0001f);

  /* Specify the display list buffer */
  dynamicp = &gfx_dynamic[gfx_gtask_no];
  glistp = &gfx_glist[gfx_gtask_no][0];

  /* Initialize RCP */
  gfxRCPInit();

  /* Clear the frame and Z-buffer */
  gfxClearCfb();

  guOrtho(&dynamicp->projection,
	  -(float)SCREEN_WD/2.0f, (float)SCREEN_WD/2.0f,
	  -(float)SCREEN_HT/2.0f, (float)SCREEN_HT/2.0f,
	  1.0F, 10.0F, 1.0F);
  guTranslate(&dynamicp->viewing, 0.0f, noiseHeightValue * -(float)SCREEN_HT/2.0f, 0.0f);

  gDPSetEnvColor(glistp++, 160 + (flicker * 5), 160 + (flicker * 5), 160 + (flicker * 5), 0);
  gDPSetCombineLERP(glistp++, NOISE,    ENVIRONMENT, SHADE,     0,
                                  0,    0,     0, SHADE,
                              NOISE,    ENVIRONMENT, SHADE,     0,
                                  0,    0,     0, SHADE);

  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicp->projection)), G_MTX_PROJECTION|G_MTX_LOAD|G_MTX_NOPUSH);
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicp->viewing)), G_MTX_MODELVIEW|G_MTX_LOAD|G_MTX_NOPUSH);

  gSPVertex(glistp++,&(backing_verts[0]),4, 0);

  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++,G_CYC_1CYCLE);
  gDPSetRenderMode(glistp++,G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
  gSPClearGeometryMode(glistp++,0xFFFFFFFF);
  gSPSetGeometryMode(glistp++, G_SHADE| G_SHADING_SMOOTH);

  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);

  gDPPipeSync(glistp++);

  drawTextRequests();

  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);


  nuGfxTaskStart(&gfx_glist[gfx_gtask_no][0],
		 (s32)(glistp - gfx_glist[gfx_gtask_no]) * sizeof (Gfx),
		 NU_GFX_UCODE_F3DEX , NU_SC_NOSWAPBUFFER);

  nuDebConDisp(NU_SC_SWAPBUFFER);

  gfx_gtask_no ^= 1;

}

void updateGameInterstital(void) {
	OSTime newTime = OS_CYCLES_TO_USEC(osGetTime());
	interStitialDeltaSeconds = 0.000001f * (newTime - interStitialTime);
	interStitialTime = newTime;
	secondsSinceSceneStart += interStitialDeltaSeconds;

	if ((secondsSinceSceneStart < TRANSITION_IN_DURATION)) {
	    noiseHeightValue = 1.f - MIN(1.f, (secondsSinceSceneStart) / TRANSITION_IN_DURATION);
	}

	if (transitioningOut) {
		secondsSinceTransitionOut += interStitialDeltaSeconds;
		noiseHeightValue = (secondsSinceTransitionOut) / TRANSITION_IN_DURATION;

		if (secondsSinceTransitionOut > TRANSITION_IN_DURATION) {
			resetStageFlag = 1;
			return;
		}
	}

	if ((secondsSinceSceneStart > TRANSITION_IN_DURATION) && (getTextRequest(0)->enable == 0) && !(transitioningOut)) {
		getTextRequest(0)->enable = 1;
		getTextRequest(0)->text = current->text;
		getTextRequest(0)->x = 8;
		getTextRequest(0)->y = 10;
		getTextRequest(0)->cutoff = 0;
		getTextRequest(0)->typewriterTick = 0;
	}

	tickTextRequests(interStitialDeltaSeconds);

	nuContDataGetEx(contdata,0);

	if ((contdata[0].trigger & A_BUTTON)) {
		if (getTextRequest(0)->cutoff == -1) {
			if (current->next) {
				current = current->next;
				getTextRequest(0)->text = current->text;
				getTextRequest(0)->cutoff = 0;
				getTextRequest(0)->typewriterTick = 0;
			} else {
				transitioningOut = 1;
				getTextRequest(0)->enable = 0;
			}
		} else {
			getTextRequest(0)->cutoff = -1;
		}
	}
}
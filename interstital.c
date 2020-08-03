

#include <nusys.h>
#include "main.h"
#include "graphic.h"
#include <gu.h>
#include "letters.h"
#include "dialogueline.h"

extern u8 _foyer_dialoguesSegmentRomStart[];
extern u8 _foyer_dialoguesSegmentRomEnd[];

static int note;

OSTime interStitialTime;
float interStitialDeltaSeconds;

DialogueLine replaceMeDaniel = { "this is a test", 0x0 };

DialogueLine* current;
float timeSinceCurrentFinishedTyping;

void initInterstitial(void) {
	note = 0;
	interStitialTime = OS_CYCLES_TO_USEC(osGetTime());
  	interStitialDeltaSeconds = 0.f;

  	current = &replaceMeDaniel;
  	timeSinceCurrentFinishedTyping = 0;

	resetTextRequests();

	getTextRequest(0)->enable = 1;
	getTextRequest(0)->text = current->text;
	getTextRequest(0)->x = 101;
	getTextRequest(0)->y = 64;
	getTextRequest(0)->cutoff = 0;
	getTextRequest(0)->typewriterTick = 0;
}

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

  drawTextRequests();

  gDPPipeSync(glistp++);

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

	tickTextRequests(interStitialDeltaSeconds);

	// If we've reached the end of the bip bip, go to the next
	if (getTextRequest(0)->cutoff == -1) {
		if (current->next) {
			timeSinceCurrentFinishedTyping += interStitialDeltaSeconds;

			if (timeSinceCurrentFinishedTyping > 1.f) {
				timeSinceCurrentFinishedTyping = 0;
				current = current->next;
				getTextRequest(0)->text = current->text;
				getTextRequest(0)->cutoff = 0;
				getTextRequest(0)->typewriterTick = 0;
			}
		}
	}

	nuContDataGetEx(contdata,0);

	if ((contdata[0].trigger & A_BUTTON)) {
	resetStageFlag = 1;
	return;
	}

	note++;
}
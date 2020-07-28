/*
   main.c

   NuSYSTEM sample nu1

   Copyright (C) 1997-1999, NINTENDO Co,Ltd.
   */

#include <nusys.h>
#include "main.h"

/* Declaration of the prototype  */
void stage00(int);

/* Declaration of the external function  */
void initStage00(void);
void makeDL00(void);
void updateGame00(void);

volatile int resetStageFlag;

/* The global variable  */
NUContData	contdata[1]; /* Read data of 1 controller  */
u8 contPattern;		     /* The pattern connected to the controller  */

// TODO: move these to their own functionality
#define NUMBER_OF_FLOORS 12
xorshift32_state grandSeed;
xorshift32_state roomSeeds[NUMBER_OF_FLOORS];

/*------------------------
	Main
--------------------------*/
void mainproc(void)
{
  int i;

  /* The initialization of graphic  */
  nuGfxInit();

  /* The initialization of the controller manager  */
  contPattern = nuContInit();

  // TODO: randomize this per tick + input
  grandSeed.a = 82675341;

  // TODO: have this begin when the player "starts a new game"
  for (i = 0; i < NUMBER_OF_FLOORS; i++) {
    roomSeeds[i].a = xorshift32(&grandSeed);
  }

  while (1) {
    resetStageFlag = 0; // change this to 0 to cause a loop to wait

    /* The initialization for stage00()  */
    initStage00();
    /* Register call-back  */
    nuGfxFuncSet((NUGfxFunc)stage00);
    /* The screen display is ON */
    nuGfxDisplayOn();

    while(resetStageFlag == 0)
      ;

    nuGfxFuncRemove();
    nuGfxDisplayOff();
  }
}

/*-----------------------------------------------------------------------------
  The call-back function 

  pendingGfx which is passed from Nusystem as the argument of the call-back 
  function is the total of RCP tasks that are currently processing and 
  waiting for the process. 
-----------------------------------------------------------------------------*/
void stage00(int pendingGfx)
{
  /* Provide the display process if 2 or less RCP tasks are processing or
	waiting for the process.  */
  if( (resetStageFlag == 0) && (pendingGfx < 3))
    makeDL00();		

  /* The process of game progress  */
  updateGame00(); 
}


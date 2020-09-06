/*
   main.c

   NuSYSTEM sample nu1

   Copyright (C) 1997-1999, NINTENDO Co,Ltd.
   */

#include <nusys.h>

#include "main.h"

#include "floordata.h"

/* Declaration of scene prototypes  */
void stage00(int);
void interstitial(int);

/* Declaration of the main stage functions  */
void initStage00(int floorNumber);
void makeDL00(void);
void updateGame00(void);

/* declaration of the interstitial functions */
void initInterstitial(int randomIndex);
void makeDLInsterstital(void);
void updateGameInterstital(void);

volatile int resetStageFlag;

/* The global variable  */
NUContData	contdata[1]; /* Read data of 1 controller  */
u8 contPattern;		     /* The pattern connected to the controller  */

volatile int currentFloor;
volatile int previousFloor;
volatile int nextRoomRequest;

int frameRandom;

/*------------------------
	Main
--------------------------*/
void mainproc(void)
{
  frameRandom = guRandom();

  /* The initialization of graphic  */
  nuGfxInit();

  /* The initialization of the controller manager  */
  contPattern = nuContInit();

  initializeSeeds();

  initalizeConnections();

  unClearAllRooms();

  // TODO: have this begin when the player "starts a new game"
  nextRoomRequest = NO_PREVIOUS_FLOOR;
  currentFloor = 0;
  previousFloor = NO_PREVIOUS_FLOOR;

  while (1) {
    resetStageFlag = 0; // change this to 0 to cause a loop to wait

    /* The initialization for stage00()  */
    initStage00(currentFloor);
    /* Register call-back  */
    nuGfxFuncSet((NUGfxFunc)stage00);
    /* The screen display is ON */
    nuGfxDisplayOn();

    while(resetStageFlag == 0)
      ;


    nuGfxFuncRemove();
    nuGfxDisplayOff();

    previousFloor = currentFloor;
    if (nextRoomRequest == -1) {
      previousFloor = NO_PREVIOUS_FLOOR;
      nextRoomRequest = 0;
    }
    currentFloor = nextRoomRequest;

    resetStageFlag = 0;

    initInterstitial(frameRandom);
    nuGfxFuncSet((NUGfxFunc)interstitial);
    /* The screen display is ON */
    nuGfxDisplayOn();

    while(resetStageFlag == 0)
      ;

    nuGfxFuncRemove();
    nuGfxDisplayOff();
  }
}

// Stage00 callback (main gameplay)
void stage00(int pendingGfx)
{
  frameRandom = guRandom();

  /* Provide the display process if 2 or less RCP tasks are processing or
	waiting for the process.  */
  if( (resetStageFlag == 0) && (pendingGfx < 3))
    makeDL00();		

  /* The process of game progress  */
  updateGame00(); 
}

// Interstitial callback
void interstitial(int pendingGfx) {
  frameRandom = guRandom();

  if( (resetStageFlag == 0) && (pendingGfx < 3)) {
    makeDLInsterstital();   
  }

  updateGameInterstital(); 
}


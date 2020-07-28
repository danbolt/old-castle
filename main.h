/*
   main.h

   Copyright (C) 1997-1999, NINTENDO Co,Ltd.
*/

#ifndef MAIN_H
#define MAIN_H

#ifdef _LANGUAGE_C

#include "xorshift.h"

/* Definition of the external variable  */
extern NUContData	contdata[1]; /* Read data of the controller  */
extern u8 contPattern;		     /* The pattern of the connected controller  */

extern volatile int resetStageFlag;

extern xorshift32_state grandSeed; // The seed of the entire playthrough
extern xorshift32_state roomSeeds[12]; // The individual seeds for each floor

#endif /* _LANGUAGE_C */
#endif /* MAIN_H */





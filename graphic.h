/*
   graphic.h

   The definition of graphic and around 

   Copyright (C) 1997-1999, NINTENDO Co,Ltd.
*/

#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

#include <nusys.h>

#define BULLET_COUNT 256
#define EMITTER_COUNT 64

/* The screen size  */
#define SCREEN_HT        240
#define SCREEN_WD        320

/* The maximum length of the display list of one task  */
#define GFX_GLIST_LEN     2048 * 2

/*-------------------------- define structure ------------------------------ */
/* The structure of the projection-matrix  */
typedef struct {
  Mtx     projection;
  Mtx     viewing;

  Mtx playerTranslation;
  Mtx playerRotation;
  Mtx playerJumpRotation;
  Mtx playerScale;
  Mtx targetTranslation;
  Mtx targetRotation;
  Mtx swordTranslation;
  Mtx swordScale;
  Mtx particleScale;
  Mtx swordRotationX;
  Mtx swordRotationZ;
  Mtx landEffectRotation;
  Mtx landEffectScale;

  Mtx bossScale;
  Mtx bossRotate;
  Mtx bossTranslate;
  Mtx bossHairTranslationA;
  Mtx bossHairTranslationB;
  Mtx bossHairTranslationC;
  Mtx bossHairTranslationD;
  Mtx bossHairMidTranslationA;
  Mtx bossHairMidTranslationB;
  Mtx bossHairMidTranslationC;
  Mtx bossHairMidTranslationD;

  Mtx grandBulletScale;

  Mtx bombEffectTranslation;
  Mtx bombEffectScale;
  Mtx bombEffectRotation;

  Mtx EmitterTranslations[EMITTER_COUNT];
  Mtx EmitterRotations[EMITTER_COUNT];

  Mtx BulletMatricies[BULLET_COUNT];
  Mtx DefeatedEffectScaleMatricies[BULLET_COUNT];

  Mtx roomEntityMatricies[8];

  Mtx roomRenderScale;

  Mtx battleRoomScale;
  Mtx battleRoomTranslation;

  Mtx specialKeyTranslation;
  Mtx specialKeyRotation;
  Mtx specialKeyRotation2;

  Mtx specialLockTranslation;
  Mtx specialLockScale;

} Dynamic;

/*-------------------------------- parameter---------------------------------*/
extern Dynamic gfx_dynamic[];
extern Gfx* glistp;
extern Gfx gfx_glist[][GFX_GLIST_LEN];
extern u32 gfx_gtask_no;
/*-------------------------------- function ---------------------------------*/
extern void gfxRCPInit(void);
extern void gfxClearCfb(void);
/*------------------------------- other extern define -----------------------*/
extern Gfx setup_rdpstate[];
extern Gfx setup_rspstate[];

#endif /* _GRAPHIC_H_ */




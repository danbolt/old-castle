
#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H

#define PLAYER_MOVE_SPEED 0.094753f
#define DEFAULT_TARGET_DISTANCE 9.9f
#define JUMP_SPEED_PER_TICK 0.03115f
#define LAND_SPEED_PER_TICK 0.035f
#define DEATH_FALL_PER_TICK 0.18f

#define POINTS_PER_BULLET 10
#define JUMP_COST 50

#define PLAYER_HIT_RADIUS 0.8f
#define PLAYER_HIT_RADIUS_SQ (PLAYER_HIT_RADIUS * PLAYER_HIT_RADIUS)

#define SWORD_RADUS 8
#define SWORD_RADUS_SQ (SWORD_RADUS * SWORD_RADUS)

#define STICK_X_MIN -61.f
#define STICK_X_MAX 61.f
#define STICK_Y_MIN -63.f
#define STICK_Y_MAX 63.f

// TODO: let the user set this
#define JOYSTICK_DEADZONE_X 0.1f
#define JOYSTICK_DEADZONE_Y 0.1f

typedef enum {
  Move,
  Jumping,
  Landed,
  Dead,
  Holding
} PlayerState;

#endif 
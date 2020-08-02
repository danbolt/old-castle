
#ifndef GAME_MATH_H
#define GAME_MATH_H

float fabs_d(float x);

float lerp(float v0, float v1, float t);

float cubic(float t);
float cubicOut(float t);

float sineEase(float t);

/* copy-paste from the nusnake example */
#define TOL ((float)1.0E-7)    /* Fix precision to 10^-7 because of the float type  */
#define M_PI_2    1.57079632679489661923
#define M_PI_4    0.78539816339744830962
#define M_RTOD    (180.0/3.14159265358979323846)

float nu_atan2(float y, float x);

#endif
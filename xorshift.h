

#ifndef XORSHIFT_H
#define XORSHIFT_H

#include <PR/ultratypes.h>

// Xorship implemention, copied from:
// https://en.wikipedia.org/wiki/Xorshift

typedef struct {
  u32 a;
} xorshift32_state;

/* The state word must be initialized to non-zero */
u32 xorshift32(xorshift32_state *state);

#endif // XORSHIFT_H
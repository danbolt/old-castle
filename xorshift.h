

#ifndef XORSHIFT_H
#define XORSHIFT_H

#include <PR/ultratypes.h>

// Xorship implemention, copied from:
// https://en.wikipedia.org/wiki/Xorshift

typedef struct {
  u32 a;
} xorshift32_state;

/* The state word must be initialized to non-zero */
u32 xorshift32(xorshift32_state *state)
{
	u32 x = state->a;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return state->a = x;
}

#endif // XORSHIFT_H
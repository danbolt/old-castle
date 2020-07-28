
#include "xorshift.h"

u32 xorshift32(xorshift32_state *state)
{
	u32 x = state->a;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return state->a = x;
}

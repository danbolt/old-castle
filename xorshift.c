
#include "xorshift.h"

unsigned int xorshift32(xorshift32_state *xorState)
{
	unsigned int x = xorState->a;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return xorState->a = x;
}

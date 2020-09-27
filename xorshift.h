

#ifndef XORSHIFT_H
#define XORSHIFT_H

// Xorship implemention, copied from:
// https://en.wikipedia.org/wiki/Xorshift

typedef struct {
  unsigned int a;
} xorshift32_state;

/* The state word must be initialized to non-zero */
unsigned int xorshift32(xorshift32_state *xorState);

#endif // XORSHIFT_H
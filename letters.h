

#ifndef LETTERS_BIN_H
#define LETTERS_BIN_H

extern unsigned char test2_bin[];
extern unsigned int test2_bin_len;

#define TEXT_REQUEST_BUF_SIZE 16
typedef struct {
  int enable;
  const char* text;
  int x;
  int y;
  int cutoff; // -1 to disable
  double typewriterTick;
  // TODO: add scalng
  // TODO: add some sort of "timeout"
  // TODO: make typewriter effect globally disable-able for accessibility
} TextRequest;
extern TextRequest textRequests[TEXT_REQUEST_BUF_SIZE];


void resetTextRequests();

void drawTextRequests();

TextRequest* getTextRequest(int slot);

void tickTextRequests(float deltaSeconds);

#endif


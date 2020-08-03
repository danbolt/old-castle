#ifndef DIALOGUE_LINE_H
#define DIALOGUE_LINE_H

typedef struct {
	const char* text;
	void* next;
} DialogueLine;

#endif
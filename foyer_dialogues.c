
#include "dialogueline.h"

DialogueLine testA;
DialogueLine testB;
DialogueLine testC;
DialogueLine testA = { "The halls are empty,\n\ndust is everywhere.", &testB };
DialogueLine testB = { "In the corners,\n\nalong the baseboards,\n\nfloating in the air.", &testC };
DialogueLine testC = { "It makes it hard to\n\nsee things for what\n\nthey really are here.", 0x0 };

DialogueLine test2A;
DialogueLine test2B;
DialogueLine test2C;
DialogueLine test2A = { "I have faint\n\nmemories of this place\n\nas a child.", &test2B };
DialogueLine test2B = { "I didn't come here often,\n\nBut I'd hear about the\n\nManor of Count Ultra\n\nfrom others.", &test2C };
DialogueLine test2C = { "I was young,\n\nbut I didn't know\n\nthe Count's career was\n\nso flawed.", 0x0 };

DialogueLine* foyer_dialogues[2] = { &testA, &test2A };
const int foyer_dialogues_count = 2;

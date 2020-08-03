
#include "dialogueline.h"

DialogueLine testA;
DialogueLine testB;
DialogueLine testA = { "Now entering\n\nbasement 2", &testB };
DialogueLine testB = { "WARNING\n WARNING\n  WARNING\n\nno refuge", 0x0 };

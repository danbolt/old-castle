#include "dialogueline.h"

DialogueLine foyer_dialogues_0_0;
DialogueLine foyer_dialogues_0_1;
DialogueLine foyer_dialogues_0_2;
DialogueLine foyer_dialogues_0_0 = { "hello world", &foyer_dialogues_0_1 };
DialogueLine foyer_dialogues_0_1 = { "it's me,\n\nDaniel", &foyer_dialogues_0_2 };
DialogueLine foyer_dialogues_0_2 = { "The curse\nstill remains", 0x0 };

DialogueLine foyer_dialogues_1_0;
DialogueLine foyer_dialogues_1_1;
DialogueLine foyer_dialogues_1_2;
DialogueLine foyer_dialogues_1_0 = { "this is another", &foyer_dialogues_1_1 };
DialogueLine foyer_dialogues_1_1 = { "does it work okay?", &foyer_dialogues_1_2 };
DialogueLine foyer_dialogues_1_2 = { "I hope so!", 0x0 };



DialogueLine* foyer_dialogues[2] = { &foyer_dialogues_0_0, &foyer_dialogues_1_0 };
const int foyer_dialogues_count = 2;


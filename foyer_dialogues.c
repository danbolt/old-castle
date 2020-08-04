#include "dialogueline.h"

DialogueLine foyer_dialogues_0_0;
DialogueLine foyer_dialogues_0_1;
DialogueLine foyer_dialogues_0_2;
DialogueLine foyer_dialogues_0_0 = { "It's dark in this old mansion.\n\nI can barely see ahead of me.", &foyer_dialogues_0_1 };
DialogueLine foyer_dialogues_0_1 = { "It must be years since this place\nhas had any visitors...", &foyer_dialogues_0_2 };
DialogueLine foyer_dialogues_0_2 = { "I wonder what was\nthe last time someone was here\nother than me?", 0x0 };

DialogueLine foyer_dialogues_1_0;
DialogueLine foyer_dialogues_1_1;
DialogueLine foyer_dialogues_1_2;
DialogueLine foyer_dialogues_1_0 = { "I came to this place once or twice\nwhen I was very young.\n\nI didn't know Count Ultra well,\nbut the charm and colour of the\nmanor left an effect on me.", &foyer_dialogues_1_1 };
DialogueLine foyer_dialogues_1_1 = { "I remember blissfully playing games\nwith friends and family.\n\nBright synthesizers played happy songs\nwith samples of horns and fiddles.", &foyer_dialogues_1_2 };
DialogueLine foyer_dialogues_1_2 = { "Looking back, even though those mom-\nents were brief, the memories still\nfeel so vivid today.\n\n\nThey likely affected me in ways I\ndon't totally understand.", 0x0 };

DialogueLine foyer_dialogues_2_0;
DialogueLine foyer_dialogues_2_1;
DialogueLine foyer_dialogues_2_2;
DialogueLine foyer_dialogues_2_0 = { "I read an article on Count Ultra's\nwork from back when I was young.", &foyer_dialogues_2_1 };
DialogueLine foyer_dialogues_2_1 = { "Apparently the contemporaries of the\ntime were seen as more interesting.\n\nIn comparison, Ultra's works seem more\nsimple, or perhaps more 'stagnant'.", &foyer_dialogues_2_2 };
DialogueLine foyer_dialogues_2_2 = { "Looking back from what was popular at\nthe time, I somewhat have to agree.\n\nThere are some timeless works, but\nalso a lot of mundane ones too.", 0x0 };



DialogueLine* foyer_dialogues[3] = { &foyer_dialogues_0_0, &foyer_dialogues_1_0, &foyer_dialogues_2_0 };
const int foyer_dialogues_count = 3;


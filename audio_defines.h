
#ifndef AUDIO_DEFINES_H
#define AUDIO_DEFINES_H

typedef struct {
	const char* trackName;
	const char* trackAuthor;
	const unsigned int midiTempo; 
	const float loopStartPoint;
	const float loopEndPoint;
} TrackInformation;

void initializeAudioLogic();

void playBossMusic();

void stopAllMusic();

typedef enum {
	Absorb0 = 0,
	Absorb1 = 1,
	Absorb2 = 2,
	Absorb3 = 3,
	AttackLand = 4,
	FadeIn = 5,
	FadeOut = 6,
	Jump = 7,
	SwordOut = 8,
} SoundEffectKey;

void playSound(SoundEffectKey sound);

#endif /* AUDIO_DEFINES_H */
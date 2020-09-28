
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

#endif /* AUDIO_DEFINES_H */
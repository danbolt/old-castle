#ifndef SEGMENT_H
#define SEGMENT_H

// This is used for the main segment data
extern char _codeSegmentStart[];
extern char _codeSegmentEnd[];

extern u8 _foyer_dialoguesSegmentRomStart[];
extern u8 _foyer_dialoguesSegmentRomEnd[];
extern u8 _foyer_dialoguesSegmentStart[];
extern u8 _foyer_dialoguesSegmentTextStart[];
extern u8 _foyer_dialoguesSegmentTextEnd[];
extern u8 _foyer_dialoguesSegmentDataStart[];
extern u8 _foyer_dialoguesSegmentDataEnd[];
extern u8 _foyer_dialoguesSegmentBssStart[];
extern u8 _foyer_dialoguesSegmentBssEnd[];

#endif // SEGMENT_H
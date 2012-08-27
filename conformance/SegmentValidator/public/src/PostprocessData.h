#ifndef _SRC_POST_PROCESS_DATA_H_
#define _SRC_POST_PROCESS_DATA_H_

#include "ValidateMP4.h"

class AudioVisualRollRecoveryEntry
{
    public:
        SInt16 roll_distance;
};


OSErr postprocessFragmentInfo(MovieInfoRec *mir);
void verifyLeafDurations(MovieInfoRec *mir);
void initializeLeafInfo(MovieInfoRec *mir, long numMediaSegments);
void checkNonIndexedSamples(MovieInfoRec *mir);
void verifyAlignment(MovieInfoRec *mir);
void verifyBSS(MovieInfoRec *mir);
void processSAP34(MovieInfoRec *mir);
OSErr processIndexingInfo(MovieInfoRec *mir);
void checkDASHBoxOrder(long cnt, atomOffsetEntry *list, long segmentInfoSize, bool initializationSegment, UInt64 *segmentSizes, MovieInfoRec *mir);
void checkSegmentStartWithSAP(int startWithSAP, MovieInfoRec *mir);
void estimatePresentationTimes(MovieInfoRec*mir);

#endif //#define _SRC_POST_PROCESS_DATA_H_

#ifndef _SRC_HELPER_METHODS_H_
#define _SRC_HELPER_METHODS_H_

#include "ValidateMP4.h"

#define ABS(a) (((a) < 0) ? -(a) : (a));

int FindAtomOffsets( atomOffsetEntry *aoe, UInt64 minOffset, UInt64 maxOffset, 
			long *atomCountOut, atomOffsetEntry **atomOffsetsOut );
TrackInfoRec * check_track( UInt32 theID );
UInt32 getTrakIndexByID(UInt32 track_ID);
UInt32 getMoofIndexByOffset(MoofInfoRec *moofInfo, UInt32 numFragments, UInt64 offset);
UInt32 getSgpdIndex(SgpdInfoRec *sgpd, UInt32 numSgpd, UInt32 grouping_type);
SidxInfoRec *getSidxByOffset(SidxInfoRec *sidxInfo, UInt32 numSidx, UInt64 offset);
bool checkSegmentBoundry(UInt64 offsetLow, UInt64 offsetHigh);
void logLeafInfo(MovieInfoRec *mir);

#endif //#define _SRC_HELPER_METHODS_H_


#pragma once
#include "CommonHeader.h"
#include "MobyData.h"

class PointerInfo {
public:
	// Explicit data
	UUID uuid; // e.g. for that of the corresponding Moby
	enum LayoutBlock : char {
		HEADER = 0,
		BLOCK1 = 1,
		BLOCK2 = 2,
		TEXTUREANIM = 3,
		WORLDANIM = 4,
		CUTS = 5,
		COLLANIM = 6,
		VERTCOLANIM = 7,
		BLOCK8 = 8,
		HELP = 9,
		VG1 = 10,
		VG2 = 11,
		SPRITEDATA = 12,
		SPRITEINFO = 13,
		MOBY = 14,
		TAG = 15,
		BLOCKA = 16,
		POINTER = 17,
		DEMO = 18,
		OTHER = 19 /* Error */
	} type;
	UUID tagPointerUuid; // tags only
	int allocatedMobysNumber; // i.e. if it's larger than the number of mobys, otherwise make it -1

	// Relative data
	int layoutOffset; // doesn't really need to be used except when building
	int blockOffset; // Inside block, for calculation
	int localOffset; // e.g. inside Moby or Tag

	PointerInfo();
	PointerInfo(char* layoutDataPointer, int offset, vector<int> originalOffsets, vector<MobyData> mobyData);


	// NEED:
	// Content data
		// All of the above
	// Could have "PointerInfo" class + "PointerOffsetInfo" class containing two copies of this



};


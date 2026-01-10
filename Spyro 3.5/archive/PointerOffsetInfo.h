#pragma once
#include "CommonHeader.h"
#include "PointerInfo.h"

class PointerOffsetInfo {
public:
	PointerInfo pointer;
	PointerInfo pointerContent;
	PointerOffsetInfo(char* layoutDataPointer, int offset, vector<int> originalOffsets, vector<MobyData> mobyData);
};


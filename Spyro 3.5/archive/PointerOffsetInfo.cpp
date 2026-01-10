#include "PointerOffsetInfo.h"

PointerOffsetInfo::PointerOffsetInfo(char* layoutDataPointer, int offset, vector<int> originalOffsets, vector<MobyData> mobyData) {
	pointer = PointerInfo(layoutDataPointer, offset, originalOffsets, mobyData);
	pointerContent = PointerInfo(layoutDataPointer, *(int*)(layoutDataPointer + offset), originalOffsets, mobyData);
	return;
}
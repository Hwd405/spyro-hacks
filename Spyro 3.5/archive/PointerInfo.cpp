#include "PointerInfo.h"

PointerInfo::PointerInfo() {
    return;
}

PointerInfo::PointerInfo(char* layoutDataPointer, int offset, vector<int> originalOffsets, vector<MobyData> mobyData) {
    layoutOffset = offset;

    // Determine block
    int j = 0;
    while (true) {
        if (originalOffsets[j + 1] <= layoutOffset) {
            j++;
        }
        else {
            type = (PointerInfo::LayoutBlock)j;
            break;
        }
        if (j + 1 == originalOffsets.size()) {
            type = PointerInfo::LayoutBlock::OTHER;
            break;
        }
    }
    blockOffset = layoutOffset - originalOffsets[type];

    // Mobys
    int mobyBlockEntries = *(int*)(layoutDataPointer + originalOffsets[PointerInfo::LayoutBlock::MOBY] + 4);
    if (type == PointerInfo::LayoutBlock::MOBY) {
        // It's in a moby
        int mobyOffset = blockOffset - 8;
        int mobyNumber = mobyOffset / 0x58; // Hopefully this is less than the number of mobys :)

        localOffset = mobyOffset % 0x58;
        if (mobyNumber >= mobyBlockEntries) {
            allocatedMobysNumber = mobyNumber - mobyBlockEntries;
        }
        else {
            allocatedMobysNumber = -1;
        }
        if (allocatedMobysNumber == -1) {
            uuid = mobyData[mobyNumber].uuid;
        }
    }
    else if (type == PointerInfo::LayoutBlock::TAG) {
        // It's in a tag
        int tagOffset = 0;
        int mobyNumber = 0;
        for (int j = 0; j < mobyBlockEntries; j++) {
            if (mobyData[j].moby.mobyTag <= layoutOffset) {
                tagOffset = mobyData[j].moby.mobyTag;
                mobyNumber = j;
            }
            else {
                break;
            }
        }
        localOffset = layoutOffset - tagOffset;
        uuid = mobyData[mobyNumber].uuid;
        // TODO - TAG UUID
    }
    return;
}
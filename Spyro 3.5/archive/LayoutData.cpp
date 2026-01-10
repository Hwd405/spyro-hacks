#include "CommonHeader.h"
#include "LayoutData.h"

void LayoutData::ParsePointerOffsets(char* layoutDataPointer) {
    for (int i = 0; i < pointerOffsetBlock.size(); i++) {
        PointerOffsetInfo info(layoutDataPointer, pointerOffsetBlock[i], originalOffsets, mobyData);
        pointerOffsetsParsed.push_back(info);
        /*





        PointerInfo info;
        info.layoutOffset = pointerOffsetBlock[i];

        // Determine block
        int j = 0;
        while (true) {
            if (originalOffsets[j + 1] <= info.layoutOffset) {
                j++;
            }
            else {
                info.type = (PointerInfo::LayoutBlock)j;
                break;
            }
            if (j + 1 == originalOffsets.size()) {
                info.type = PointerInfo::LayoutBlock::OTHER;
                break;
            }
        }
        info.blockOffset = info.layoutOffset - originalOffsets[info.type];

        // Mobys
        if (info.type == PointerInfo::LayoutBlock::MOBY) {
            // It's in a moby
            int mobyOffset = info.blockOffset - 8;
            int mobyNumber = mobyOffset / 0x58; // Hopefully this is less than the number of mobys :)
            info.localOffset = mobyOffset % 0x58;
            if (mobyNumber >= mobyBlockEntries) {
                info.allocatedMobysNumber = mobyNumber - mobyBlockEntries;
            }
            else {
                info.allocatedMobysNumber = -1;
            }
            if (info.allocatedMobysNumber == -1) {
                info.uuid = mobyData[mobyNumber].uuid;
            }
        }
        else if (info.type == PointerInfo::LayoutBlock::TAG) {
            // It's in a tag
            int tagOffset = 0;
            int mobyNumber = 0;
            for (int j = 0; j < mobyBlockEntries; j++) {
                if (mobyData[j].moby.mobyTag <= info.layoutOffset) {
                    tagOffset = mobyData[j].moby.mobyTag;
                    mobyNumber = j;
                }
                else {
                    break;
                }
            }
            info.localOffset = info.layoutOffset - tagOffset;
            info.uuid = mobyData[mobyNumber].uuid;
        }
        */
    }
    return;
}

LayoutData::LayoutData(char* layoutDataPointer, int layoutLength) {
    // Header
    originalOffsets.push_back(0);
    layoutHeader = *(LayoutHeader*)layoutDataPointer;
    char* reading = layoutDataPointer + 0x30;

    // Block 1
    originalOffsets.push_back(reading - layoutDataPointer);
    unkBlock1Length = *(int*)reading;
    unkBlock1.assign(reading + 4,reading + unkBlock1Length);
    reading += unkBlock1Length;

    // Block 2
    originalOffsets.push_back(reading - layoutDataPointer);
    unkBlock2Length = *(int*)reading;
    unkBlock2.assign(reading + 4, reading + unkBlock2Length);
    reading += unkBlock2Length;

    // Animated Textures
    originalOffsets.push_back(reading - layoutDataPointer);
    animatedTexturesBlockLength = *(int*)reading;
    animatedTexturesBlock.assign(reading + 4, reading + animatedTexturesBlockLength);
    reading += animatedTexturesBlockLength;

    // World Animations
    originalOffsets.push_back(reading - layoutDataPointer);
    worldAnimationsBlockLength = *(int*)reading;
    worldAnimationsBlock.assign(reading + 4, reading + worldAnimationsBlockLength);
    reading += worldAnimationsBlockLength;

    // Cutscenes
    originalOffsets.push_back(reading - layoutDataPointer);
    cutscenesBlockLength = *(int*)reading;
    cutscenesBlock.assign(reading + 4, reading + cutscenesBlockLength);
    reading += cutscenesBlockLength;

    // Collision Animations
    originalOffsets.push_back(reading - layoutDataPointer);
    collisionAnimationsBlockLength = *(int*)reading;
    collisionAnimationsBlock.assign(reading + 4, reading + collisionAnimationsBlockLength);
    reading += collisionAnimationsBlockLength;

    // Vertex Colour Animations
    originalOffsets.push_back(reading - layoutDataPointer);
    vertexColourAnimationsBlockLength = *(int*)reading;
    vertexColourAnimationsBlock.assign(reading + 4, reading + vertexColourAnimationsBlockLength);
    reading += vertexColourAnimationsBlockLength;

    // Block 8
    originalOffsets.push_back(reading - layoutDataPointer);
    unkBlock8Length = *(int*)reading;
    unkBlock8.assign(reading + 4, reading + unkBlock8Length);
    reading += unkBlock8Length;

    // Help Text
    originalOffsets.push_back(reading - layoutDataPointer);
    helpBlockLength = *(int*)reading;
    helpBlock.assign(reading + 4, reading + helpBlockLength);
    reading += helpBlockLength;

    // Visgroups?
    originalOffsets.push_back(reading - layoutDataPointer);
    if (*(int*)reading < 5) {
        visgroupsPresent = false;
        vgBlock1Length = 0;
        // vgBlock1 empty
    }
    else {
        visgroupsPresent = true;
        vgBlock1Length = *(int*)reading;
        vgBlock1.assign(reading + 4, reading + vgBlock1Length);
        reading += vgBlock1Length;
    }
    originalOffsets.push_back(reading - layoutDataPointer);
    vgBlock2Length = *(int*)reading;
    vgBlock2.assign(reading + 4, reading + vgBlock2Length); // Can I do this with empty vectors? Probably
    reading += vgBlock2Length;

    // Sprite Data
    originalOffsets.push_back(reading - layoutDataPointer);
    spriteDataBlockLength = *(int*)reading;
    spriteDataBlock.assign(reading + 4, reading + spriteDataBlockLength);
    reading += spriteDataBlockLength;

    // Sprite Info - animated sprites? etc
    originalOffsets.push_back(reading - layoutDataPointer);
    spriteInfoBlockLength = *(int*)reading;
    spriteInfoBlock.assign(reading + 4, reading + spriteInfoBlockLength);
    reading += spriteInfoBlockLength;

    // Mobys
    originalOffsets.push_back(reading - layoutDataPointer);
    char* mobyBlockPointer = reading;
    mobyBlockLength = *(int*)reading;
    mobyBlockEntries = *(int*)(reading + 4);
    Moby* mobyArray = (Moby*)(reading + 8);
    for (int i = 0; i < mobyBlockEntries; i++) {
        // Moby
        MobyData moby;
        moby.moby = mobyArray[i];
        
        // UUID TODO - FIX LATER
        // UUID uuid;
        //UuidCreate(&uuid);
        // moby.uuid = uuid;
        
        // Add to vector
        mobyData.push_back(moby);
    }
    reading += mobyBlockLength;

    // 10240 bytes of free space goes here

    // Tags
    originalOffsets.push_back(reading - layoutDataPointer);
    char* tagBlockPointer = reading;
    tagBlockLength = *(int*)reading;
    reading += tagBlockLength; // Currently the end of the block
    for (int i = 0; i < mobyBlockEntries; i++) {
        if (i != mobyBlockEntries - 1) {
            mobyData[i].tag.assign(layoutDataPointer + mobyData[i].moby.mobyTag, layoutDataPointer + mobyData[i + 1].moby.mobyTag);
        }
        else {
            mobyData[i].tag.assign(layoutDataPointer + mobyData[i].moby.mobyTag, reading);
        }
    }
    // TODO - decode all tags

    // Block A - CHECK THIS ACTS LIKE OTHER NEARBY BLOCKS, IT MIGHT NOT!!!
    originalOffsets.push_back(reading - layoutDataPointer);
    char* endOfTagBlockPointer = reading;
    unkBlockALength = *(int*)reading;
    unkBlockA.assign(reading + 4, reading + unkBlockALength);
    reading += unkBlockALength;

    // Pointer Offsets
    originalOffsets.push_back(reading - layoutDataPointer);
    pointerOffsetBlockLength = *(int*)reading; // This is the number of ints, not the total block length, and so does not include the length
    int* offsets = (int*)(reading + 4);
    pointerOffsetBlock.assign(offsets, offsets + pointerOffsetBlockLength);
    ParsePointerOffsets(layoutDataPointer);
    reading = (char*)((int*)reading + 1 + pointerOffsetBlockLength);

    // Demo inputs
    originalOffsets.push_back(reading - layoutDataPointer);
    demoInputsBlockLength = *(int*)reading;
    ControllerState* demoInputs = (ControllerState*)(reading + 4);
    demoInputsBlock.assign(demoInputs, demoInputs + demoInputsBlockLength);

    this->layoutLength = layoutLength; // When building, I should make the full length a multiple of 0x800 - probably don't need to use this variable
    originalOffsets.push_back(layoutLength);
}

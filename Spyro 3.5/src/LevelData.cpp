#include "LevelData.h"

bool sortByNumber(const EggData& a, const EggData& b) {
    return a.number < b.number;
}

#define SQR(x) ((x)*(x))
#define ABS(x) ((x) < 0 ? -(x) : (x))
unsigned long long GetDistance2D(Vector3D* a, Vector3D* b) {
    return (unsigned long long)sqrt(SQR((unsigned long long)ABS(a->x - b->x)) + SQR((unsigned long long)ABS(a->y - b->y)));
}

vector<int> Collision::GetTrisInCylinder(unsigned int h, unsigned int r, Vector3D centre) {

    // probably need to get block centres or something idk, see what happens:
    vector<int> indices;

    for (int i = 0; i < this->collisionHeader->triCount; i++) {
        vector<Vector3D> points = this->triangles[i].GetPoints();
        for (int j = 0; j < 3; j++) {
            if ((GetDistance2D(&(points[j]), &centre) < r) && ABS(points[j].z - centre.z) < h) {
                indices.push_back(i);
                break;
            }
        }
    }
    return indices;

}

vector<int> Collision::GetTrisInCuboid(unsigned int w, unsigned int l, unsigned int h, Vector3D centre) {

    vector<int> indices;

    for (int i = 0; i < this->collisionHeader->triCount; i++) {
        vector<Vector3D> points = this->triangles[i].GetPoints();
        for (int j = 0; j < 3; j++) {
            if ((ABS(points[j].x - centre.x) < w) &&
                (ABS(points[j].y - centre.y) < l) &&
                (ABS(points[j].z - centre.z) < h)) {
                indices.push_back(i);
                break;
            }
        }
    }
    return indices;

}

void LevelData::UpdateEggNames(vector<EggData> eggs) {

    sort(eggs.begin(), eggs.end(), sortByNumber);

    char* blockStart = eggBlockPointer + 4; // the int here is the length of the internal block, make sure this is updated
    int initialLength = *(int*)blockStart;
    EggInfo* offsets = (EggInfo*)(blockStart + 4);
    char* names = blockStart + offsets[0].offset;
    int toUpdate = min(eggs.size(), numberOfEggs); // Just to be safe

    for (int i = 0; i < toUpdate; i++) {

        const char* name = eggs[i].name.c_str();
        int length = eggs[i].name.length();

        memcpy(names, name, length);
        *(names + length) = 0;
        names += (length + 1);

        if (i + 1 < toUpdate) {
            offsets[i + 1].offset = (names - blockStart);
        }
    }

    while ((int)names % 4 != 0) {
        *names = 0;
        names++;
    }

    *(int*)blockStart = names - blockStart;

    while (names - blockStart < initialLength) {
        *names = 0;
        names++;
    }

    return;
}

LevelData::LevelData(char* levelDataPointer, int levelDataLength) {

    char* reading = levelDataPointer;

    // Old implementation, very bad, don't want to fix. sorry
    
    // Block 0 - Textures
    block0Pointer = reading;
    originalOffsets.push_back(reading - levelDataPointer);
    unkBlock0Length = *(int*)reading;
    unkBlock0.assign(reading + 4, reading + unkBlock0Length);
    reading += unkBlock0Length;
    
    // Block 1 - World
    block1Pointer = reading;
    originalOffsets.push_back(reading - levelDataPointer);
    unkBlock1Length = *(int*)reading;
    unkBlock1.assign(reading + 4, reading + unkBlock1Length);
    reading += unkBlock1Length;

    // Block 2
    originalOffsets.push_back(reading - levelDataPointer);
    unkBlock2Length = *(int*)reading;
    unkBlock2.assign(reading + 4, reading + unkBlock2Length);
    reading += unkBlock2Length;

    // Block 3 - Lighting animations?
    originalOffsets.push_back(reading - levelDataPointer);
    unkBlock3Length = *(int*)reading;
    unkBlock3.assign(reading + 4, reading + unkBlock3Length);
    reading += unkBlock3Length;

    // Block 4
    originalOffsets.push_back(reading - levelDataPointer);
    int* block4Pointer = (int*)reading;
    unkBlock4_1 = block4Pointer[0];
    unkBlock4_2 = block4Pointer[1];
    unkBlock4_3 = block4Pointer[2];
    reading = (char*)(block4Pointer + 3);
    
    // Block 5 - Collision
    collision = Collision((unsigned int*)reading);
    originalOffsets.push_back(reading - levelDataPointer);
    unkBlock5Length = *(int*)reading;
    unkBlock5.assign(reading + 4, reading + unkBlock5Length);
    reading += unkBlock5Length;

    // Block 6 - Skybox
    skyboxPointer = reading;
    originalOffsets.push_back(reading - levelDataPointer);
    unkBlock6Length = *(int*)reading;
    unkBlock6.assign(reading + 4, reading + unkBlock6Length);
    reading += unkBlock6Length;
    
    // Block 7 - Portals
    portalBlockPointer = reading;
    originalOffsets.push_back(reading - levelDataPointer);
    portalBlockLength = *(int*)reading;
    portalCount = *(int*)(reading + 4);
    portalInfo.assign((PortalInfo*)(reading + 8), (PortalInfo*)(reading + 8) + portalCount);
    skyboxBlock.assign(reading + 8 + portalCount * sizeof(PortalInfo), reading + portalBlockLength);
    reading += portalBlockLength;
    
    // Block 8
    originalOffsets.push_back(reading - levelDataPointer);
    unkBlock8Length = *(int*)reading;
    unkBlock8.assign(reading + 4, reading + unkBlock8Length);
    reading += unkBlock8Length;

    // Block 9 - Eggs
    eggBlockPointer = reading;
    originalOffsets.push_back(reading - levelDataPointer);
    eggBlockLength = *(int*)reading;
    eggNameBlockLength = *(int*)(reading + 4);
    zeroPadLength = eggBlockLength - eggNameBlockLength - 4;

    EggInfo* eggInfoPointer = (EggInfo*)(reading + 8);
    bool gotAll = false;
    numberOfEggs = 0;
    while (!gotAll) {
        if (eggInfoPointer[numberOfEggs].unk1 % 0x1000000 == 0x808080
            &&
            eggInfoPointer[numberOfEggs].unk2 % 0x1000000 == 0x808080) {
            // This is crude - is there another way of doing this?
            numberOfEggs += 1;
        }
        else {
            gotAll = true;
        }
    }
    eggInfo.assign(eggInfoPointer, eggInfoPointer + numberOfEggs);
    if (numberOfEggs > 0) {
        unkEggInfo.assign((char*)eggInfoPointer + numberOfEggs * sizeof(EggInfo), reading + 4 + eggInfoPointer[0].offset);
        for (int i = 0; i < numberOfEggs; i++) {
            const char* newString = reading + 4 + eggInfoPointer[i].offset;
            string str(newString);
            names.push_back(str);
        }
    }
    reading += eggBlockLength;

    // Block 10 - Speedway Sounds?
    originalOffsets.push_back(reading - levelDataPointer);
    speedwaySoundBlockLength = *(int*)reading;
    speedwaySoundBlock.assign(reading + 4, reading + speedwaySoundBlockLength);
    reading += speedwaySoundBlockLength;

    // Block 11 - Ambient Sounds
    originalOffsets.push_back(reading - levelDataPointer);
    memcpy(ambientSounds, reading, 32);
    reading += 32;

    // Block 12 - SPU Data
    originalOffsets.push_back(reading - levelDataPointer);
    spuDataBlockLength = *(int*)reading;
    memcpy(soundTable, reading + 4, 256);
    reading += 260; // 4 + 256
    spuDataEntries = *(int*)reading;
    spuData.assign((SpuData*)(reading + 4), (SpuData*)(reading + 4) + spuDataEntries);
    
    return;
}
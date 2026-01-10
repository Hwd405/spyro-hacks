#pragma once
#include "CommonHeader.h"

/**
*
* LevelData.h
* Pretty bad name in all honesty
* Very outdated and clunky scene base related things
*
*/

struct CollisionTriangle {
    unsigned int x, y, z;

    void UpdateTri(Vector3D v0, Vector3D v1, Vector3D v2) { // this is not correct, don't use it
        this->x = (v2.x << 23 | v1.x << 14 | v0.x);
        this->y = (v2.y << 23 | v1.y << 14 | v0.y);
        this->z = (v2.z << 24 | v1.z << 16 | v0.z);
        return;
    }
    void UpdateX(int xDiff) {
        unsigned int x = this->x & 0x3FFF;
        x += (xDiff / 16);
        this->x &= 0xFFFFC000;
        this->x |= x;
    }
    void UpdateZ(int zDiff) { // Keeping this implementation because it's used in several places
        unsigned int z = this->z & 0x3FFF; // should be 0xFFFF
        z += (zDiff / 16);
        this->z &= 0xFFFFC000; // this is probably wrong actually - should be 0xFFFF0000
        this->z |= z;
    }
    void UpdateZCorrected(int zDiff) {
        unsigned int z = this->z & 0xFFFF;
        z += (zDiff / 16);
        this->z &= 0xFFFF0000;
        this->z |= z;
    }
    vector<Vector3D> GetPoints() {
        unsigned int x = this->x & 0x3FFF;
        unsigned int y = this->y & 0x3FFF;
        unsigned int z = this->z & 0x3FFF;
        Vector3D v0 = { 16 * x, 16 * y, 16 * z };
        Vector3D v1 = { 16 * (((int)(this->x << 9) >> 23) + x),
                        16 * (((int)(this->y << 9) >> 23) + y),
                        16 * (((int)(this->z << 8) >> 24) + z) };
        Vector3D v2 = { 16 * ((this->x >> 23) + x),
                        16 * ((this->y >> 23) + y),
                        16 * ((this->z >> 24) + z) };
        vector<Vector3D> vecs;
        vecs.push_back(v0);
        vecs.push_back(v1);
        vecs.push_back(v2);
        return vecs;
    }
};

class Collision {
public:
    unsigned int* start;
    unsigned int length;
    struct CollisionHeader {
        unsigned int triCount;
        unsigned int flaggedCount;
        unsigned int unk1;
        unsigned int blockTreeOffset; // offsets are from the start of the data part of the component - AFTER length
        unsigned int blocksOffset;
        unsigned int trianglesOffset;
        unsigned int surfaceTypesOffset;
        unsigned int unk2;
        unsigned int unk3;
    } *collisionHeader;
    unsigned short* blockTree; // offsets are from the start of the data part of the component - AFTER length
    unsigned short* blocks;
    CollisionTriangle* triangles;
    unsigned short* surfaceTypes;

    Collision() {}
    Collision(unsigned int* component) {
        this->start = component;
        this->length = *component;
        this->collisionHeader = (CollisionHeader*)(component + 1);
        this->blockTree = (unsigned short*)((char*)(this->collisionHeader) + collisionHeader->blockTreeOffset);
        this->blocks = (unsigned short*)((char*)(this->collisionHeader) + collisionHeader->blocksOffset);
        this->triangles = (CollisionTriangle*)((char*)(this->collisionHeader) + collisionHeader->trianglesOffset);
        this->surfaceTypes = (unsigned short*)((char*)(this->collisionHeader) + collisionHeader->surfaceTypesOffset);
        return;
    }
    vector<int> GetTrisInCylinder(unsigned int h, unsigned int r, Vector3D centre);
    vector<int> GetTrisInCuboid(unsigned int w, unsigned int l, unsigned int h, Vector3D centre);

};

class LevelData {
public:
    vector<int> originalOffsets; // For each block

    // Block 0 - Textures
    int unkBlock0Length;
    vector<unsigned char> unkBlock0;
    char* block0Pointer;

    // Block 1 - World
    int unkBlock1Length;
    vector<unsigned char> unkBlock1;
    char* block1Pointer;

    // Block 2 - ? short component
    int unkBlock2Length;
    vector<unsigned char> unkBlock2;

    // Block 3 - related to lighting animations?
    int unkBlock3Length;
    vector<unsigned char> unkBlock3;

    // Block 4 - Three ints??
    int unkBlock4_1;
    int unkBlock4_2;
    int unkBlock4_3;

    // Block 5 - Collision
    int unkBlock5Length;
    vector<unsigned char> unkBlock5;
    Collision collision;

    // Block 6 - Skybox
    int unkBlock6Length;
    vector<unsigned char> unkBlock6;
    char* skyboxPointer;

    // Block 7 - Portals
    int portalBlockLength;
    int portalCount;
    vector<PortalInfo> portalInfo;
    vector<unsigned char> skyboxBlock; // Probably splits up further - possibly need some sort of skybox class
    char* portalBlockPointer;

    // Block 8 - Related to portal skyboxes (visgroups?)
    int unkBlock8Length;
    vector<unsigned char> unkBlock8;

    // Block 9 - Eggs
    int eggBlockLength;
    int eggNameBlockLength;
    vector<EggInfo> eggInfo;
    vector<unsigned char> unkEggInfo;
    vector<string> names;
    int zeroPadLength; // Unclear if I'll use this
    int numberOfEggs; // Unclear if I'll use this
    char* eggBlockPointer;

    // Block 10 - Speedway Sounds?
    int speedwaySoundBlockLength;
    vector<unsigned char> speedwaySoundBlock;

    // Block 11 - Ambient Sounds
    char ambientSounds[32];

    // Block 12 - SPU Data
    int spuDataBlockLength;
    char soundTable[256]; // Could create a sound table struct instead
    int spuDataEntries;
    vector<SpuData> spuData;

    // Rest is padded to a mulitple of 0x800

    LevelData(char* levelDataPointer, int levelDataLength); // Fairly straightforward
    void UpdateEggNames(vector<EggData> eggs);
};


#pragma once
#include "CommonHeader.h"
#include "LayoutData.h"
#include "LevelData.h"

class LevelWad {
private:
    int areaCount;
public:
    DataHeader vramHeader;
    DataHeader levelDataHeader;
    AreaHeader areaHeaders[4];

    DataHeader dragonModelHeaders[6];
    ModelPointer modelPointers[64];

    vector<unsigned char> vram;
    
    LevelData levelData;
    
    vector<unsigned char> scenes[4]; //SceneData scenes[4];
    LayoutData layouts[4];
    
    vector<unsigned char> dragonModels[6]; //DragonModel dragonModels[6];

    //LevelWad();
};


#pragma once
#include "CommonHeader.h"
#include "MobyData.h"
#include "PointerOffsetInfo.h"

class LayoutData {
private:
    vector<int> originalOffsets; // For each block
public:
    // Header
    LayoutHeader layoutHeader;
    
    // Block 1
    int unkBlock1Length;
    vector<unsigned char> unkBlock1;
    
    // Block 2
    int unkBlock2Length;
    vector<unsigned char> unkBlock2; // LayoutDataPointers.field_0x4
    
    // Animated Textures
    int animatedTexturesBlockLength;
    vector<unsigned char> animatedTexturesBlock; // LayoutDataPointers.AnimatedTextures
    
    // World Animations
    int worldAnimationsBlockLength;
    vector<unsigned char> worldAnimationsBlock; // LayoutDataPointers.WorldAnimations
    
    // In-level Cutscenes
    int cutscenesBlockLength;
    vector<unsigned char> cutscenesBlock; // LayoutDataPointers.InLevelCustscenes
    
    // Collision Animations
    int collisionAnimationsBlockLength;
    vector<unsigned char> collisionAnimationsBlock; // LayoutDataPointers.CollisionAnimations
    
    // Vertex Colour Animations
    int vertexColourAnimationsBlockLength;
    vector<unsigned char> vertexColourAnimationsBlock; // LayoutDataPointers.VertexColourAnimations
    
    // Block 8
    int unkBlock8Length;
    vector<unsigned char> unkBlock8; // LayoutDataPointers.field_0x1c
    
    // Help Menu
    int helpBlockLength;
    vector<unsigned char> helpBlock;
    
    // Visgroups?
    bool visgroupsPresent;
    int vgBlock1Length; // 0 if visgroups not present
    vector<unsigned char> vgBlock1;
    int vgBlock2Length; // 0x04 if visgroups not present
    vector<unsigned char> vgBlock2;
    
    // Sprite "Data"
    int spriteDataBlockLength; // Sprite data etc, consider renaming
    vector<unsigned char> spriteDataBlock;
    
    // Sprite "Info"
    int spriteInfoBlockLength; // This is probably the sprite info block - animated sprites etc, consider renaming
    vector<unsigned char> spriteInfoBlock;
    
    // Mobys and Tags
    int mobyBlockLength; // mobys.size() * size(Moby) + 8;
    int mobyBlockEntries; // mobys.size();
    int tagBlockLength; // Comes after the moby block obvs
    vector<MobyData> mobyData; // Moby block is followed by 0x10240 bytes of free space, then by tag block - Mobys could be initialised first with empty tag before tags added
    
    // Block A
    int unkBlockALength;
    vector<unsigned char> unkBlockA; // tags + 1 - pods? god knows
    
    // Pointer Offsets
    int pointerOffsetBlockLength; // In this case, does not include the block length itself! It's the number of INTs
    vector<int> pointerOffsetBlock;
    vector<PointerOffsetInfo> pointerOffsetsParsed;
    
    // Demo Inputs
    int demoInputsBlockLength; // In this case, does not include the block length itself! It's the number of LONGs (inputs)
    vector<ControllerState> demoInputsBlock; // Change to an input type
    
    // Various
    //int emptyBlockLength; // zeroes - I suspect the full block length needs to to be a multiple of 0x800 - probably don't really need this
    int layoutLength; // Could be set when building the layout data? but might be better to have on the wad layer instead

    LayoutData(char* layoutDataPointer, int layoutLength);
    void ParsePointerOffsets(char* layoutDataPointer);
};


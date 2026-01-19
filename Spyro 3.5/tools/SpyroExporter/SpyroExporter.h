#pragma once
//#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <string>

using namespace std;

typedef unsigned char u8;
typedef char i8;
typedef unsigned short u16;
typedef short i16;
typedef unsigned int u32;
typedef int i32;
typedef unsigned long ULONG;

typedef struct {
    i32 x, y, z;
} Vector3D;

typedef struct {
    u8 x, y, z;
} Angle;

typedef struct {
    u8 id;
    u8 nextId;
    u8 frame;
    u8 nextFrame;
} Animation;

typedef struct {
    u32 mobyTag; // 0x00 void* (4 byte)
    u32 nextCollision; // 0x04 Moby* (4 byte)
    u32 unknownCollision; // 0x08
    Vector3D position; // 0x0C
    u32 damageFlags; // 0x18
    u8 unknown[24]; // 0x1C
    u16 collisionBlock; // 0x34
    u16 mobyClass; // 0x36
    u16 unknown2[2]; // 0x38
    Animation animationState; // 0x3C
    u8 animationProgress; // 0x40
    u8 unknown3[2]; // 0x41
    u8 animationRun; // 0x43
    Angle angle; // 0x44
    u8 unknown4; // 0x47
    u8 state; // 0x48
    u8 substate; // 0x49
    u8 unknown5[5]; // 0x4A
    u8 scaleDown; // 0x4F
    u8 unka; // 0x50
    u8 unkb; // 0x51
    u8 levelSectorIndex; // 0x52
    u8 unkc; // 0x53
    u32 colour; // 0x54
} Moby;

typedef struct {
    // 0x800722C0 on Sep14

    // 0
    int levelData;

    // 1
    int scene;

    // 2
    int layout;

    // 3
    int unk;

    // ...
} DataPointers;

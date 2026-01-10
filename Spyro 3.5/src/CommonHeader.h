#pragma once
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <rpc.h>
//#include <rpcdce.h>

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

typedef struct Moby {
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
	u8 unknown5[2]; // 0x4A
	u8 lowDrawDistance; // 0x4C, if 0 moby is invisible
	u8 drawn; // 0x4D, 0-2
	u8 highDrawDistance; // 0x4E
	u8 scaleDown; // 0x4F
	u8 levelSectorIndex; // 0x50
	u8 unkb; // 0x51
	u8 subtype; // 0x52
	u8 unkc; // 0x53
	u32 colour; // 0x54
} Moby;

typedef struct {
	unsigned int start, length;
} DataHeader;

typedef struct {
	int offset;
	int length;
} WadEntry;

typedef struct {
	WadEntry entries[256];
} Wad;

typedef struct {
	Vector3D pos;
	Angle angle;
} PosInfo;

typedef struct {
	char number;
	bool toUpdate; // false if this egg should be left alone
	char levelNumber;
	char area;
	Vector3D position;
	char yaw;
	string name;
	string atlasName;
} EggData;

typedef struct {
	DataHeader scene;
	DataHeader layout;
} AreaHeader;

typedef struct {
	short mobyClass;
	int offset;
} ModelPointer;

typedef struct {
	int clut;
	int texture;
} VramPosition;

typedef struct {
	Vector3D spawnPosition;
	Angle spawnRotation;
	char zero; // unless a use is found for this
	int mobyOffset;
	int unk2;
	VramPosition flameTexture;
	VramPosition unkTexture1;
	VramPosition unkTexture2;
} LayoutHeader;

/*
typedef struct {
	int id; // deprecated probably?
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
		OTHER = 19 // Error
	} type;
	int postMobysNumber; // deprecated, remove

	UUID uuid; // e.g. for that of the corresponding Moby
	int layoutOffset; // doesn't really need to be used except when building
	int blockOffset; // Inside block, for calculation
	int localOffset; // e.g. inside Moby or Tag
	int allocatedMobysNumber; // i.e. if it's larger than the number of mobys, otherwise make it -1

	// Need the whole thing duplicated as a "content" field too I think

	// NEED:
	// Explicit info
		// Block type
		// UUID (e.g. for mobys) of the object it's pointing at
		// tagPointerUUID in tags
		// allocatedMobysNumber
	// Offsets
		// layoutOffset
		// blockOffset
		// localOffset (inside moby or tag)
	// Content data
		// All of the above
	// Could have "PointerInfo" class + "PointerOffsetInfo" class containing two copies of this

} PointerInfo; // May want to make this a class instead
*/

typedef struct {
	int stickState;
	enum ButtonState : int { L2 = 0x1, R2 = 0x2, L1 = 0x4, R1 = 0x8, TRI = 0x10, O = 0x20, X = 0x40, SQU = 0x80, SEL = 0x100, STA = 0x800, U = 0x1000, R = 0x2000, D = 0x4000, L = 0x8000 } buttonState;
} ControllerState;

typedef enum {
	S1_D1    = 11052500,
	S1_JUN15 = 11061500,
	// Missing versions
	// Missing versions
	S3_D2    = 33072700,
	S3_D3    = 33080000,
	S3_D4    = 33080001,
	S3_D5    = 33080002,
	S3_D6    = 33080003,
	S3_LNUM  = 33080004,
	S3_EREV  = 33090000,
	S3_SEP04 = 33090400,
	S3_SEP14 = 33091400,
	S3_SEP29 = 33092900,
	S3_OCT24 = 33102400,
	S3_OCT31 = 33103100
} GameVersion;

typedef struct {
	Vector3D polygonPoints[5];
	u32 skyboxPointer;
	u8 renderDistance;
	u8 sidedness;
	u8 levelId;
	u8 animationState;
} PortalInfo;

typedef struct {
	int offset; // In-game this is replaced with a pointer to the name
	int unk1;
	int unk2;
} EggInfo;

typedef struct {
	unsigned int   unk1;
	unsigned short unk2;
	unsigned short unk3;
	unsigned short unk4;
	unsigned short unk5;
	unsigned short unk6;
	unsigned short unk7;
	unsigned char  unk8;
	unsigned char  unk9;
	unsigned short unk10;
	unsigned short unk11;
} SpuData;

typedef struct {
	int startLba; // Relative to 90000, the start of SPEECH.STR
	unsigned short lengthLba;
	unsigned short unk; // Possibly the channel, so these should be 00 - 08 with the upper byte 00 (possible that the upper byte is something else)
} XaData;

typedef struct {
	Vector3D pos;
	int widthX;
	int widthY;
	int angleZ;
} BoundingBox;

typedef struct {
	BoundingBox box; // entrance
	// int unknown[12];
	// int spawnAngle;
	// Vector3D spawnPos;
	// The spawn position is in the critter's tag!
	// Spawn angle - 0x48
	// Spawn pos - 0x4c
} Moby991Tag;

typedef struct {
	BoundingBox box; // entrance
	BoundingBox unknown;
	int linkedMoby; // both should be deleted when removing sublevels
	int toArea;
	int entranceZWidth;
	int musicZWidth;
	int fromArea;
} Moby1022Tag;

typedef struct {
	unsigned int animationCount;
	char sounds[16];
	unsigned int collisionModels[8];
	unsigned int dataOffset; // ???
	unsigned int faces; // The address immediately after all of the headers - colour offsets are from here
	unsigned int animations[1]; // length animationCount, each of the animations (ModelAnimHeader) immediately follow, offset is from the start of the model
} ModelHeader;

typedef struct {
	unsigned short blockOffset; // offset of block start (in blocks/ *2 bytes)
	unsigned char blockCount; // number of blocks
	unsigned char startFlags; // flags
	unsigned int shadowData; // ?
} ModelAnimFrameInfo;

typedef struct {
	// There seems to be more than 1 type of anim header so let's start with this one
	unsigned char frameCount, colourCount, vertScale, unkc;
	unsigned char vertCount, unkd, unke, unkf;
	unsigned int unkg;
	unsigned int verts; // offset
	unsigned int faces; // offset
	unsigned int colours; // offset
	unsigned int lpFaces; // offset
	unsigned int lpColours; // offset
	unsigned short data;
	short empty; // ?
	ModelAnimFrameInfo frames[1]; // length frameCount?
} ModelAnimHeader;

typedef struct {
	unsigned char R, G, B;
	unsigned char empty;
} RGB;

typedef struct {
	char typ, msg;
	unsigned short unk;
	unsigned int length;
	unsigned int start; // relative to SPEECH.STR / LBA 90000
} DialogueData;

typedef struct {
	short value;
	short paid;
} MoneybagsEntry;

extern Wad wadHeader;
extern vector<vector<EggData>> eggData;
extern Moby sampleEgg;
extern vector<char> sampleEggTag;
extern int gameVersion;
extern bool musicFilesPresent;
extern unsigned int overlayOffsets[2];
extern bool stormyMode;
extern bool explorationMode;
extern string subversion;

extern int getCRC(unsigned char* buffer, ULONG bufsize);
extern void UpdateLevels(char* wadPath, char* exePath);
extern void UpdateExe(char* exePath);
extern bool fileExists(const char* fileName);
extern void UpdateOverlays(char* exePath, char* wadPath);
//extern unsigned long long GetDistance(Vector3D* a, Vector3D* b);

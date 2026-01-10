#pragma once
#include "CommonHeader.h"

/*
==Tags==
1
2		Undefined
3
4
5





248		Fixed

950		Fixed

988		Fixed

1022
1023
*/

typedef struct {
	Vector3D OriginalPosition; //Probably in case it falls in the lava
	int Empty; //0
	int FallTimer; //Starts at 0, decreases to -0xC0 while falling when dropped
	short PickupTimer; //Possibly relative rotation, 0 when it stops rotating
	short PadId; //0 if a weight, 1 or 2 if a pad
	int WeightIndex1; //-1 if a weight, index of a weight moby if a pad
	int WeightIndex2; //The other weight
} Moby248Tag;

typedef struct {
	int EggRequirement;
	int unk1;
	int unk2;
	int unk3;
} Moby950Tag;

typedef struct {
	int LevelId; //Used for the name that appears, not the destination
	int TwoSided; //If 1, the name appears on both sides
	int EggReq;
	int InteractionCheck; //If non-negative, checks that this interaction has been set, if it hasn't the portal remains closed
	int unk3;
} Moby988Tag;
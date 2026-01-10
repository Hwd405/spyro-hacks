#pragma once
#include "CommonHeader.h"
#include "LevelData.h"
#include "component.h"

/**
*
* SceneData.h
* Outdated area scene related formats
* Understanding of these is a bit better now - these are area overwrites to the scene
*
*/

class vramImage {
public:
	bool hasVram;
	int* start;
	int x;
	int y;
	int w;
	int h;
	Component<char> preVramBlock;
	short* vram;
	vramImage(Reader* r) {
		this->start = (int*)r->getPos();
		this->x = r->read<int>();
		this->y = r->read<int>();
		this->w = r->read<int>();
		this->h = r->read<int>();
		if (r->peek<int>() == -1) {
			this->hasVram = false;
			return;
		}
		this->hasVram = true;
		this->preVramBlock = Component<char>(r);
		this->vram = (short*)r->getPos();
	}
	vramImage() {}
};

class SceneData {
private:
	int sublevel; // this is probably useful
public:

	// Area 0
	Component<unsigned char> unknown01;
	vramImage vram;
	//int* unknown02; // 4 ints
	//Component<unsigned char> blockBeforeVram; // ?
	//unsigned char* vram; // 0xC000, each 0x200 block is placed at the start of each 0x400 block from vram+0x60000

	// Areas 1 - 3
	Component<unsigned char> mobyModels; // the first int inside the data seems to be the number of models minus 1 (i.e. if there's only Spyro, it'd be 0)?
	int* modelOffsets; // 64 int, seem to be relative to scene + 1 int, the last one is the length of the model pointers (one more entry than the classes)
	short* modelClasses; // 64 short
	Component<unsigned char> unknown11;
	Component<unsigned char> unknown12;
	Component<unsigned char> unknown13;
	Component<unsigned char> unknown14;

	Component<unsigned char> unknown15;
	Collision collision; // this is unknown15, but I've implemented it awkwardly, so
	
	 //int* unknown02; // 4 ints
	//Component<unsigned char> blockBeforeVram; //?
	//unsigned char* vram; // 0xC000 again

	SceneData(int sublevel, unsigned char* buffer, int size);
	SceneData();
	void UpdateVram(const char* vramPath);

};


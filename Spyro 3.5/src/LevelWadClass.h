#pragma once
#include "CommonHeader.h"
#include "LayoutDataClass.h"


class LevelWadClass {
public:
	int sublevelCount;
	DataHeader levelDataHeader;
	DataHeader sceneHeaders[4];
	DataHeader layoutHeaders[4];
	int* modelOffsets;
	short* modelClasses;
	char* sceneDataBlocks[4];
	char* layoutDataBlocks[4];
	char* levelDataBlock;
	char* vramDataBlock;
	vector<LayoutDataClass> layoutClasses;
	int levelNumber;
	int wadNumber;
	WadEntry wadEntry;
	fstream wad;

	LevelWadClass(char* wadPath, int levelNumber);
	void UpdateLayout(int sublevelNumber);
	void UpdateScene(int sublevelNumber);
	void UpdateLayouts();
	void UpdateData();
	void UpdateVram();
	void UpdateEgg(EggData egg, vector<vector<int>> presentEggs);
	void UpdateEggs(vector<EggData> eggs);
	void UpdateEggNames(int offset, int levelNumber, int eggCount);
	void UpdateSublevelExit(int from, int to, PosInfo pos);
	ModelHeader* GetModelPointer(int area, int mobyClass);
	~LevelWadClass();
};


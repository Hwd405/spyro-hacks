#pragma once
#include "CommonHeader.h"

/**
* 
* LayoutDataClass.h
* Outdated layout related formats
* 
*/

typedef struct {
	int length; // Length to tag block
	int count;
	Moby mobys;
} MobyBlock;

class LayoutDataClass {
private:
	int originalMobyBlockLength;
	int originalMobyCount;
	int originalTagBlockLength;
public:
	char* layoutData;
	int offset;
	int length;
	MobyBlock* mobyBlock;
	Moby* mobys;
	char* tagBlock; // (char*)mobyBlock + mobyBlock.length
	int* offsetBlock; // length (in ints), starting from the int after offsetBlock
	LayoutDataClass(char* layoutDataPointer, DataHeader layoutDataHeader);
	void DeleteMoby(int index);
	void AddMoby(Moby moby, vector<char> mobyTag);
	int GetTagLength(int index);
	MobyBlock* GetMobyBlock();
	int GetMobyIndexFromNpcType(short mobyClass, char npcType);
	vector<int> GetMobyIndexFromClass(short mobyClass);
	int GetClosestMobyIndexFromClass(short mobyClass, Vector3D position);
	void UpdateNpcPositionByType(short mobyClass, char npcType, Vector3D position);
	void UpdateNpcPositionByType(short mobyClass, char npcType, PosInfo position);
	void UpdateMobyPosition(int index, Vector3D position);
	void UpdateMobyPosition(int index, PosInfo position);
	void UpdateMobyPositions(vector<int> indices, vector<Vector3D> positions);
	void UpdateMobyPositions(vector<int> indices, vector<PosInfo> positions);
	void UpdateMobyPositionsByClass(short mobyClass, vector<Vector3D> positions);
	void UpdateMobyPositionsByClass(short mobyClass, vector<PosInfo> positions);
	void UpdatePointerOffsetBlock(int pointerOffset);
	void DuplicateMoby(int index);
	void DuplicateMoby(int index, PosInfo posinfo);
	void DuplicateMoby(short mobyClass, int index);
	void DuplicateMoby(short mobyClass, int index, PosInfo posinfo);
	Moby GetMoby(int index);
	void SetMoby(int index, Moby moby);
	vector<char> GetMobyTag(int index, int length);
	void SetEggNumber(int index, unsigned int newValue);
	int GetEggNumber(int index);
	void PrintMobyInfoByClass(short mobyClass);
	int GetEggIndexByNumber(int number);
	char* GetTagPointer(int index);
};


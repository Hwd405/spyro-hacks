#include "CommonHeader.h"
#include "LayoutDataClass.h"

char* LayoutDataClass::GetTagPointer(int index) {
	return layoutData + mobys[index].mobyTag;
}

int LayoutDataClass::GetEggIndexByNumber(int number) {
	return GetMobyIndexFromNpcType(132, (char)number);
}

void LayoutDataClass::PrintMobyInfoByClass(short mobyClass) {
	cout << "-----" << endl;
	cout << "Class " << (int)mobyClass << endl << endl;
	vector<int> indices = GetMobyIndexFromClass(mobyClass);
	for (int i = 0; i < indices.size(); i++) {
		cout << "Moby " << dec << i << endl;
		Moby moby = GetMoby(indices[i]);
		for (int j = 0; j < 0x58; j++) {
			cout << hex << (int)*(((unsigned char*)&moby) + j) << " ";
		}
		cout << endl;
		cout << "Tag " << dec << i << endl;
		int len = GetTagLength(indices[i]);
		vector<char> tag = GetMobyTag(indices[i], len);
		for (int j = 0; j < len; j++) {
			cout << hex << (int)(unsigned char)tag[j] << " ";
		}
		cout << endl;
		if (i != indices.size() - 1) {
			cout << endl;
		}
	}
	cout << "-----" << endl;
	return;
}

void LayoutDataClass::UpdatePointerOffsetBlock(int pointerOffset) {
	int offsetBlockLength = *offsetBlock;
	int* endOfBlock = offsetBlock + 1 + offsetBlockLength;

	if (*endOfBlock == 0) {
		*endOfBlock = pointerOffset;
		*offsetBlock += 1;
	}
	else {
		if (*endOfBlock < 0x1000) {
			memcpy(endOfBlock + 2,endOfBlock,8*(*endOfBlock - 1) + 4/*length*/);
			*endOfBlock = pointerOffset;
			*offsetBlock += 1;
			*(endOfBlock + 1) = 0;
		}
		else {
			printf("Warning - demo inputs block too long, not updating pointer offsets block.\n");
		}
	}
	return;
}

void LayoutDataClass::SetMoby(int index, Moby moby) {
	if (index < 0) {
		printf("Warning - index passed to SetMoby is negative.\n");
	}
	mobys[index] = moby;
	return;
}

Moby LayoutDataClass::GetMoby(int index) {
	if (index < 0) {
		printf("Warning - index passed to GetMoby is negative.\n");
	}
	return mobys[index];
}

vector<char> LayoutDataClass::GetMobyTag(int index, int length) {
	char* reading = layoutData + mobys[index].mobyTag;
	vector<char> tag;
	while (reading < layoutData + mobys[index].mobyTag + length) {
		tag.push_back(*reading);
		reading++;
	}
	return tag;
}

int LayoutDataClass::GetEggNumber(int index) {
	if (mobys[index].mobyClass != 132) {
		printf("Error - moby passed to GetEggNumber is not an egg.\n");
		return -1;
	}
	return *(int*)(layoutData + mobys[index].mobyTag);
}

void LayoutDataClass::SetEggNumber(int index, unsigned int newValue) {
	if (mobys[index].mobyClass != 132) {
		printf("Error - moby passed to SetEggNumber is not an egg.\n");
		return;
	}
	if (newValue >= 6) {
		printf("Error - egg number passed to SetEggNumber is too large.\n");
		return;
	}
	*(int*)(layoutData + mobys[index].mobyTag) = newValue;
	return;
}

int LayoutDataClass::GetMobyIndexFromNpcType(short mobyClass, char npcType) {
	// Only first is obtained because this is what I need in practice
	for (int i = 0; i < mobyBlock->count; i++) {
		if (mobys[i].mobyClass == mobyClass) {
			if (*(layoutData + mobys[i].mobyTag) == npcType) {
				return (int)i;
			}
		}
	}
	printf("Error - no NPC found with class %d and type %d.\n",mobyClass,npcType);
	return -1;
}

vector<int> LayoutDataClass::GetMobyIndexFromClass(short mobyClass) {
	vector<int> indices;
	for (int i = 0; i < mobyBlock->count; i++) {
		if (mobys[i].mobyClass == mobyClass) {
			indices.push_back(i);
		}
	}
	if (indices.empty()) {
		//printf("Error - no moby found with class %d.\n", mobyClass); // this spits out loads of egg related messages
	}
	return indices;
}

#define SQR(x) ((x)*(x))
#define ABS(x) ((x) < 0 ? -(x) : (x))
unsigned long long GetDistance(Vector3D* a, Vector3D* b) {
	// return (unsigned long long)sqrt(SQR((double)ABS(a->x - b->x)) + SQR((double)ABS(a->y - b->y)) + SQR((double)ABS(a->z - b->z)));
	return (unsigned long long)sqrt(SQR((unsigned long long)ABS(a->x - b->x)) + SQR((unsigned long long)ABS(a->y - b->y)) + SQR((unsigned long long)ABS(a->z - b->z)));
	// return sqrt(SQR(ABS(a->x - b->x)) + SQR(ABS(a->y - b->y)) + SQR(ABS(a->z - b->z)));
}

int LayoutDataClass::GetClosestMobyIndexFromClass(short mobyClass, Vector3D position) {
	vector<int> indices = GetMobyIndexFromClass(mobyClass);
	if (indices.empty()) {
		printf("Error - class passed to GetClosestMobyIndexFromClass not found.\n");
		return -1;
	}
	unsigned long long closestDist = 0xffffffffffffffff;
	unsigned long long calcDist;
	int closestIndex = indices[0];
	for (int i = 0; i < indices.size(); i++) {
		calcDist = GetDistance(&(mobys[indices[i]].position), &position);
		if (calcDist < closestDist) {
			closestDist = calcDist;
			closestIndex = indices[i];
		}
	}
	return closestIndex;
}

void LayoutDataClass::UpdateNpcPositionByType(short mobyClass, char npcType, Vector3D position) {
	int npcIndex = GetMobyIndexFromNpcType(mobyClass, npcType);
	UpdateMobyPosition(npcIndex, position);
	return;
}

void LayoutDataClass::UpdateNpcPositionByType(short mobyClass, char npcType, PosInfo position) {
	int npcIndex = GetMobyIndexFromNpcType(mobyClass, npcType);
	UpdateMobyPosition(npcIndex, position);
	return;
}

void LayoutDataClass::UpdateMobyPosition(int index, Vector3D position) {
	if (index == -1) {
		printf("Error - invalid index passed to UpdateMobyPosition,\n");
		return;
	}
	mobys[index].position = position;
	return;
}

void LayoutDataClass::UpdateMobyPosition(int index, PosInfo position) {
	if (index == -1) {
		printf("Error - invalid index passed to UpdateMobyPosition,\n");
		return;
	}
	mobys[index].position = position.pos;
	mobys[index].angle = position.angle;
	return;
}

void LayoutDataClass::UpdateMobyPositions(vector<int> indices, vector<Vector3D> positions) {
	// Smallest list is used
	int len = min(indices.size(), positions.size());
	if (len == 0) {
		printf("Error - array of length 0 passed to UpdateMobyPositions.\n");
		return;
	}
	for (int i = 0; i < len; i++) {
		UpdateMobyPosition(indices[i], positions[i]);
	}
	return;
}

void LayoutDataClass::UpdateMobyPositions(vector<int> indices, vector<PosInfo> positions) {
	// Smallest list is used
	int len = min(indices.size(), positions.size());
	if (len == 0) {
		printf("Error - array of length 0 passed to UpdateMobyPositions.\n");
		return;
	}
	for (int i = 0; i < len; i++) {
		UpdateMobyPosition(indices[i], positions[i]);
	}
	return;
}

void LayoutDataClass::UpdateMobyPositionsByClass(short mobyClass, vector<Vector3D> positions) {
	vector<int> indices = GetMobyIndexFromClass(mobyClass);
	UpdateMobyPositions(indices, positions);
	return;
}

void LayoutDataClass::UpdateMobyPositionsByClass(short mobyClass, vector<PosInfo> positions) {
	vector<int> indices = GetMobyIndexFromClass(mobyClass);
	UpdateMobyPositions(indices, positions);
	return;
}

void LayoutDataClass::DeleteMoby(int index) {
	memset((char*)(mobys + index),0,0x58);
	return;
}

void LayoutDataClass::AddMoby(Moby moby, vector<char> mobyTag) {
	int remainingMemory = mobyBlock->length - (mobyBlock->count * 0x58 + 8);
	if (remainingMemory < 0x58 + mobyTag.size()) {
		printf("ERROR - Unable to add moby - not enough memory.\n");
		return;
	}
	mobys[mobyBlock->count] = moby;
	mobyBlock->count++;
	*(int*)(tagBlock - mobyTag.size()) = *(int*)tagBlock + mobyTag.size();
	memcpy(tagBlock + 4 - mobyTag.size(),(char*)&(mobyTag[0]), mobyTag.size());
	tagBlock -= mobyTag.size();
	mobyBlock->length -= mobyTag.size();
	mobys[mobyBlock->count - 1].mobyTag = ((tagBlock + 4) - layoutData);
	UpdatePointerOffsetBlock((char*)&(mobys[mobyBlock->count - 1].mobyTag) - layoutData);
	return;
}

void LayoutDataClass::DuplicateMoby(int index) {

	Moby moby = GetMoby(index);
	int len = GetTagLength(index);
	vector<char> tag = GetMobyTag(index, len);
	AddMoby(moby, tag);
	return;

}

void LayoutDataClass::DuplicateMoby(int index, PosInfo posinfo) {

	Moby moby = GetMoby(index);
	moby.position = posinfo.pos;
	moby.angle = posinfo.angle;
	int len = GetTagLength(index);
	vector<char> tag = GetMobyTag(index, len);
	AddMoby(moby, tag);
	return;

}

void LayoutDataClass::DuplicateMoby(short mobyClass, int index, PosInfo posinfo) {

	vector<int> indices = GetMobyIndexFromClass(mobyClass);
	if (index >= indices.size()) {
		printf("ERROR - Invalid index given to DuplicateMoby.\n");
		return;
	}

	DuplicateMoby(indices[index], posinfo);
	return;

}

void LayoutDataClass::DuplicateMoby(short mobyClass, int index) {

	vector<int> indices = GetMobyIndexFromClass(mobyClass);
	if (index >= indices.size()) {
		printf("ERROR - Invalid index given to DuplicateMoby.\n");
		return;
	}

	DuplicateMoby(indices[index]);
	return;

}

int LayoutDataClass::GetTagLength(int index) {
	// If less than last usual moby, diff with next moby tag
	// If last usual moby, diff with tag block end
	// If one more than last usual moby, diff with original tag block start + 4
	// If any more than that, diff with previous
	if (index < originalMobyCount - 1) {
		return mobys[index + 1].mobyTag - mobys[index].mobyTag;
	}
	else if (index == originalMobyCount - 1) {
		return (tagBlock + *(int*)tagBlock) - (layoutData + mobys[index].mobyTag);
	}
	else if (index == originalMobyCount) {
		return originalMobyBlockLength - mobys[index].mobyTag;
	}
	else {
		return mobys[index - 1].mobyTag - mobys[index].mobyTag;
	}
}

MobyBlock* LayoutDataClass::GetMobyBlock() {
	// Locate the moby block
	char* currentlyReading = layoutData + 0x30;
	bool checksFailed = false;

	for (int k = 0; k < 9; k++) {
		// Confirm we're not jumping into anywhere we don't want to be
		if (currentlyReading + *(int*)currentlyReading >= layoutData + length) {
			checksFailed = true;
			break;
		}
		// Confirm we're not null-jumping
		if (*(int*)currentlyReading == 0) {
			checksFailed = true;
			break;
		}
		currentlyReading += *(int*)currentlyReading;
	}
	if (checksFailed) {
		return 0; // This is to be checked afterwards
	}
	char hasVisgroups = 0;
	if (!(*currentlyReading < 5)) {
		hasVisgroups = 1;
	}
	for (int k = 0; k < 3 + hasVisgroups; k++) {
		// Confirm we're not jumping into anywhere we don't want to be
		if (currentlyReading + *(int*)currentlyReading >= layoutData + length) {
			checksFailed = true;
			break;
		}
		// Confirm we're not null-jumping
		if (*(int*)currentlyReading == 0) {
			checksFailed = true;
			break;
		}
		currentlyReading += *(int*)currentlyReading;
	}
	if (checksFailed) {
		return 0;
	}

	return (MobyBlock*)currentlyReading;
}

LayoutDataClass::LayoutDataClass(char* layoutDataPointer, DataHeader layoutDataHeader) {
	layoutData = layoutDataPointer;
	offset = layoutDataHeader.start;
	length = layoutDataHeader.length; // immediately after this is the sublevel skybox
	
	mobyBlock = GetMobyBlock();
	mobys = &(mobyBlock->mobys);
	tagBlock = (char*)mobyBlock + mobyBlock->length;

	originalMobyBlockLength = mobyBlock->length;
	originalMobyCount = mobyBlock->count;
	originalTagBlockLength = *(int*)tagBlock;

	char* reading = tagBlock + *(int*)tagBlock;
	reading = reading + *(int*)reading;
	offsetBlock = (int*)reading;
}

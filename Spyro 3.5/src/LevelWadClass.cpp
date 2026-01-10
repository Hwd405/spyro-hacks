#include "LevelWadClass.h"

bool compareByNumber(const EggData& a, const EggData& b) {
	return a.number < b.number;
}


void LevelWadClass::UpdateEggNames(int offset, int levelNumber, int eggCount) {

	// Offset is to start of name block, not egg block
	int* eggNameBlockLengthPointer = (int*)(levelDataBlock + offset);
	EggInfo* eggInfoPointer = (EggInfo*)(levelDataBlock + offset + 4);
	vector<int> offsets;
	int firstNameOffset = eggInfoPointer[0].offset;
	int nameBlockLength = 0;

	vector<EggData> currentEggData = eggData[levelNumber];
	sort(currentEggData.begin(), currentEggData.end(), compareByNumber);

	for (int i = 0; i < eggCount; i++) {
		offsets.push_back(nameBlockLength); // These are offsets from the first string
		nameBlockLength += (currentEggData[i].name).length();
		nameBlockLength = nameBlockLength - (nameBlockLength % 4) + 4;
	}
	char* newBlock = (char*)malloc(nameBlockLength);
	for (int i = 0; i < nameBlockLength; i++) {
		newBlock[i] = 0; // Initialise this entire block for copying
	}
	for (int i = 0; i < eggCount; i++) {
		string str = currentEggData[i].name;
		str.copy(newBlock + offsets[i], str.length());
		eggInfoPointer[i].offset = firstNameOffset + offsets[i];
	}
	memcpy(levelDataBlock + offset + firstNameOffset, newBlock, nameBlockLength);
	free(newBlock);
	return;

}

void LevelWadClass::UpdateData() {
	wad.seekp(wadEntry.offset + levelDataHeader.start);
	wad.write(levelDataBlock, levelDataHeader.length);
	return;
}

void LevelWadClass::UpdateVram() {
	wad.seekp(wadEntry.offset + 0x800);
	wad.write(vramDataBlock, 0x80000);
	return;
}

void LevelWadClass::UpdateScene(int sublevelNumber) {
	wad.seekp(wadEntry.offset + sceneHeaders[sublevelNumber].start);
	wad.write(sceneDataBlocks[sublevelNumber], sceneHeaders[sublevelNumber].length);
	return;
}

void LevelWadClass::UpdateLayout(int sublevelNumber) {
	wad.seekp(wadEntry.offset + layoutHeaders[sublevelNumber].start);
	wad.write(layoutDataBlocks[sublevelNumber], layoutHeaders[sublevelNumber].length);
	return;
}

void LevelWadClass::UpdateLayouts() {
	for (int i = 0; i < sublevelCount; i++) {
		UpdateLayout(i);
	}
	return;
}

void LevelWadClass::UpdateEgg(EggData egg, vector<vector<int>> presentEggs) {
	// This is rather convoluted and could probably be more easily done in non-move cases by just passing in an example egg moby / tag

	if (!egg.toUpdate) {
		return; // Do not update this egg
	}

	if (egg.area >= presentEggs.size()) {
		printf("Error - invalid area number passed to UpdateEgg.\n");
		return;
	}

	vector<vector<int>> indices; // All egg indices per area
	for (int i = 0; i < sublevelCount; i++) {
		indices.push_back(layoutClasses[i].GetMobyIndexFromClass(132)); // These should line up with the order in presentEggs realistically!
	}

	// Check if egg is already in sublevel
	if (find(presentEggs[egg.area].begin(), presentEggs[egg.area].end(), egg.number) != presentEggs[egg.area].end()) {
		// Update the corresponding egg's position and yaw - later on may need a state update too
		for (int i = 0; i < presentEggs[egg.area].size(); i++) {
			if ((presentEggs[egg.area])[i] == egg.number) {
				PosInfo position;
				position.pos = egg.position;
				position.angle = { 0, 0, (u8)egg.yaw };
				layoutClasses[egg.area].UpdateMobyPosition((indices[egg.area])[i], position);
				break;
			}
		}
	}
	else {
		bool present = false;
		int area;
		for (int i = 0; i < sublevelCount; i++) {
			if (find(presentEggs[i].begin(), presentEggs[i].end(), egg.number) != presentEggs[i].end()) {
				present = true;
				area = i;
				break;
			}
		}
		if (present) {
			// Egg is present in level in area i
			Moby copyMoby;
			vector<char> tag;
			for (int i = 0; i < presentEggs[area].size(); i++) {
				if ((presentEggs[area])[i] == egg.number) {
					copyMoby = layoutClasses[area].GetMoby((indices[area])[i]);
					copyMoby.position = egg.position;
					copyMoby.angle.z = egg.yaw;
					tag = layoutClasses[area].GetMobyTag((indices[area])[i], 24); // Egg tag is 24 bytes long
					layoutClasses[egg.area].AddMoby(copyMoby, tag);
					layoutClasses[area].DeleteMoby((indices[area])[i]);
					break;
				}
			}
		}
		else {
			// Egg is not present in level and needs to be added
			bool foundNonEmpty = false;
			for (int i = 0; i < sublevelCount; i++) {
				if (!indices[i].empty()) {
					foundNonEmpty = true;
					area = i;
					break;
				}
			}
			if (foundNonEmpty) {
				Moby copyMoby = layoutClasses[area].GetMoby((indices[area])[0]);
				copyMoby.position = egg.position;
				copyMoby.angle.z = egg.yaw;
				vector<char> tag = layoutClasses[area].GetMobyTag((indices[area])[0], 24);
				//*(int*)&tag = egg.number;
				tag[0] = egg.number;
				layoutClasses[egg.area].AddMoby(copyMoby, tag);
			}
			else {
				//printf("Error - cannot place egg as no valid eggs are present.\n");
				//printf("If this is a boss level, consider updating this function to add in a sample egg or do it elsewhere.\n");
				// use sampleEggTag
				Moby newMoby = sampleEgg;
				newMoby.position = egg.position;
				newMoby.angle.z = egg.yaw;
				vector<char> tag = sampleEggTag;
				tag[0] = egg.number;
				layoutClasses[egg.area].AddMoby(newMoby, tag);
			}
		}
	}
	return;
}

void LevelWadClass::UpdateEggs(vector<EggData> eggs) {
	// Check that there's not too many eggs
	if (eggs.size() > 6) {
		printf("Error - too many eggs passed to UpdateEggs.\n");
		return;
	}
	// Should really validate that it's with the usual egg limits too I think

	// Validate level and sublevel
	for (int i = 0; i < eggs.size(); i++) {
		if (eggs[i].levelNumber != levelNumber) {
			printf("Error - invalid level number passed to UpdateEggs.\n");
			return;
		}
		if (eggs[i].area >= sublevelCount) {
			printf("Error - invalid area number passed to UpdateEggs.\n");
			return;
		}
	}

	for (int i = 0; i < eggs.size(); i++) {
		// Recalculate this with each update or shit breaks
		vector<vector<int>> presentEggs;
		for (int i = 0; i < sublevelCount; i++) {
			vector<int> indices = layoutClasses[i].GetMobyIndexFromClass(132);
			vector<int> numbers;
			for (int j = 0; j < indices.size(); j++) {
				if (layoutClasses[i].GetEggNumber(indices[j]) != -1) {
					numbers.push_back(layoutClasses[i].GetEggNumber(indices[j]));
				}
			}
			presentEggs.push_back(numbers);
		}
		UpdateEgg(eggs[i], presentEggs);
	}

	return;
}

void LevelWadClass::UpdateSublevelExit(int from, int to, PosInfo pos) {
	vector<int> indices = layoutClasses[from].GetMobyIndexFromClass(1022);
	for (int i = 0; i < indices.size(); i++) {
		Moby* moby = &(layoutClasses[from].mobys[indices[i]]);
		Moby1022Tag* tag = (Moby1022Tag*)(layoutClasses[from].layoutData + moby->mobyTag);
		if (tag->fromArea == from && tag->toArea == to) {
			layoutClasses[from].UpdateMobyPosition(tag->linkedMoby, pos);
			break;
		}
	}
	return;
}

ModelHeader* LevelWadClass::GetModelPointer(int area, int mobyClass) {

	if (area == 0) {
		for (int i = 0; i < 64; i++) {
			if (modelClasses[i] == mobyClass) return (ModelHeader*)(sceneDataBlocks[area] + (modelOffsets[i] - sceneHeaders[area].start));
		}
		cout << "ERROR - invalid mobyClass passed to GetModelPointer" << endl;
		return 0;
	}
	else {
		cout << "ERROR - non-zero area passed to GetModelPointer, but this is not currently supported" << endl;
	}
	return 0;
}

LevelWadClass::~LevelWadClass() {
	for (int i = 0; i < sublevelCount; i++) {
		if (layoutDataBlocks[i] != 0) {
			free(layoutDataBlocks[i]);
			//free(sceneDataBlocks[i]);
		}
	}
	//free(levelDataBlock);
	//free(vramDataBlock);
	//free(modelOffsets);
	//free(modelClasses);
	wad.close();
	return;
}

LevelWadClass::LevelWadClass(char* wadPath, int levelNumber) {
	this->levelNumber = levelNumber;
	wadNumber = 97 + levelNumber * 2; // Late S3 only
	wadEntry = wadHeader.entries[wadNumber];
	wad.open(wadPath, ios_base::in | ios_base::out | ios_base::binary);

	// Get the number of sublevels
	char* header = (char*)malloc(16 * 6);
	wad.seekg(wadEntry.offset);
	wad.read(header, 16 * 6);
	sublevelCount = 0;
	for (int i = 0; i < 4; i++) {
		sceneHeaders[i] = *(DataHeader*)(header + 0x10 + 0x10 * i);
		layoutHeaders[i] = *(DataHeader*)(header + 0x18 + 0x10 * i);
		layoutDataBlocks[i] = 0;
		if (layoutHeaders[i].start != 0) {
			sublevelCount++;
		}
	}
	levelDataHeader = *(DataHeader*)(header + 8);

	// Models
	modelOffsets = (int*)malloc(64 * 4);
	modelClasses = (short*)malloc(64 * 2);
	wad.seekg(wadEntry.offset + 0xA0);
	wad.read((char*)modelOffsets, 64 * 4);
	wad.seekg(wadEntry.offset + 0x1A0);
	wad.read((char*)modelClasses, 64 * 2);

	// Allocate layouts
	for (int i = 0; i < sublevelCount; i++) {
		layoutDataBlocks[i] = (char*)malloc(layoutHeaders[i].length);
		wad.seekg(wadEntry.offset + layoutHeaders[i].start);
		wad.read(layoutDataBlocks[i],layoutHeaders[i].length);
		layoutClasses.push_back(LayoutDataClass(layoutDataBlocks[i], layoutHeaders[i]));

		sceneDataBlocks[i] = (char*)malloc(sceneHeaders[i].length);
		wad.seekg(wadEntry.offset + sceneHeaders[i].start);
		wad.read(sceneDataBlocks[i], sceneHeaders[i].length); // the actual usage of this as a scene object will be done somewhere else
	}
	levelDataBlock = (char*)malloc(levelDataHeader.length);
	wad.seekg(wadEntry.offset + levelDataHeader.start);
	wad.read(levelDataBlock, levelDataHeader.length);

	vramDataBlock = (char*)malloc(0x80000);
	wad.seekg(wadEntry.offset + 0x800);
	wad.read(vramDataBlock, 0x80000);

	return;
}
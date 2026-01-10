#include "CommonHeader.h"
#include "LevelWadClass.h"
#include "TagFormats.h"
#include "LevelData.h"
#include "SceneData.h"

Moby sampleGem;
vector<char> sampleGemTag;

int GetStringLength(char* str, int headerLen) {
	int len = string(str).length() + headerLen; // include null
	len >>= 2; // Divide by 4, remove remainder
	len += 1; // Add 1 (this will become 4)
	len *= 4;
	return len - headerLen;
}

int GetStringLength(string str, int headerLen) {
	int len = str.length() + headerLen; // include null
	len >>= 2; // Divide by 4, remove remainder
	len += 1; // Add 1 (this will become 4)
	len *= 4;
	return len - headerLen;
}

void MoveDialogueStringForwards(LevelWadClass* levelWad, int area, int index, int message, int count) {

	char* tag = levelWad->layoutClasses[area].GetTagPointer(index);
	int* dialoguePointers = (int*)(tag + 16); // 0xC is name, 0x10 is msg 0
	int lastLine = (int)tag[3]; // one less than the msg count

	if (message > lastLine || message < 0) {
		printf("ERROR - moving dialogue failed - invalid message number passed to MoveDialogueStringForwards.\n");
		return;
	}

	if (count % 4 != 0) {
		printf("ERROR - moving dialogue failed - count passed to MoveDialogueStringForwards must be a multiple of 4.\n");
		return;
	}

	// Check lengths
	char* messagePointer = levelWad->layoutClasses[area].layoutData + dialoguePointers[message];
	int headerLen = *messagePointer;
	char* headerPointer = messagePointer;
	messagePointer += headerLen;

	if ((message == lastLine) && (GetStringLength(messagePointer, headerLen) < count)) { // note that if there's extra padding at the end this won't be accounted for
		printf("ERROR - moving dialogue failed - count is greater than the length of the old line. Count is %d characters longer.\n", count - GetStringLength(messagePointer, headerLen));
		return;
	}
	else if (message < lastLine) {
		int over = count - (dialoguePointers[message + 1] - (dialoguePointers[message] + headerLen)); // latter term is the "true" space for the string
		if (0 < over) {
			printf("ERROR - moving dialogue failed - count is greater than the length of the old line. Count is %d characters longer.\n", over);
			return;
		}
	}

	memmove(headerPointer + count, headerPointer, headerLen);
	dialoguePointers[message] += count;

	int i = 0;
	while (i < count) {
		*headerPointer = 0;
		headerPointer++;
		i++;
	}

	return;

}

void MoveDialogueStringForwards(LevelWadClass* levelWad, int area, int mobyClass, int type, int message, int count) {

	int index = levelWad->layoutClasses[area].GetMobyIndexFromNpcType(mobyClass, type);
	if (index == -1) {
		printf("ERROR - moving dialogue failed - area %d, mobyClass %d, type %d.\n", area, mobyClass, type);
		return;
	}
	MoveDialogueStringForwards(levelWad, area, index, message, count);
	return;

}

void MoveDialogueStringBackwards(LevelWadClass* levelWad, int area, int index, int message, int count) {

	char* tag = levelWad->layoutClasses[area].GetTagPointer(index);
	int* dialoguePointers = (int*)(tag + 16); // 0xC is name, 0x10 is msg 0
	int lastLine = (int)tag[3]; // one less than the msg count

	if (message > lastLine || message < 0) {
		printf("ERROR - moving dialogue failed - invalid message number passed to MoveDialogueStringBackwards.\n");
		return;
	}
	else if (message == 0) {
		printf("ERROR - moving dialogue failed - cannot move first line backwards.\n");
		return;
	}

	if (count % 4 != 0) {
		printf("ERROR - moving dialogue failed - count passed to MoveDialogueStringBackwards must be a multiple of 4.\n");
		return;
	}

	// Check lengths
	char* messagePointer = levelWad->layoutClasses[area].layoutData + dialoguePointers[message];
	char* earlierMessagePointer = levelWad->layoutClasses[area].layoutData + dialoguePointers[message - 1];
	int headerLen = *messagePointer;

	if (earlierMessagePointer + *earlierMessagePointer + 1 + count + 1 > messagePointer) { // let there be at least one character and one nullspace character in the earlier message
		printf("ERROR - moving dialogue failed - count is greater than the length of the earlier line. Count is %d characters longer.\n", (int)((earlierMessagePointer + *earlierMessagePointer + 2 + count) - messagePointer));
		return;
	}

	memmove(messagePointer - count - 1, messagePointer - 1, headerLen + 1);
	dialoguePointers[message] -= count;

	char* spaces = messagePointer + headerLen - count;

	while (spaces < messagePointer + headerLen) {
		*spaces = 0x20; // space
		spaces++;
	}

	return;

}

void MoveDialogueStringBackwards(LevelWadClass* levelWad, int area, int mobyClass, int type, int message, int count) {

	int index = levelWad->layoutClasses[area].GetMobyIndexFromNpcType(mobyClass, type);
	if (index == -1) {
		printf("ERROR - moving dialogue failed - area %d, mobyClass %d, type %d.\n", area, mobyClass, type);
		return;
	}
	MoveDialogueStringBackwards(levelWad, area, index, message, count);
	return;

}

void UpdateDialogueFromIndex(LevelWadClass* levelWad, int area, int index, int message, string line) {

	char* tag = levelWad->layoutClasses[area].GetTagPointer(index);
	int* dialoguePointers = (int*)(tag + 16); // 0xC is name, 0x10 is msg 0
	int lastLine = (int)tag[3]; // one less than the msg count

	if (message > lastLine || message < 0) {
		printf("ERROR - updating dialogue failed - invalid message number passed to UpdateDialogueFromIndex.\n");
		return;
	}

	// Check lengths
	char* messagePointer = levelWad->layoutClasses[area].layoutData + dialoguePointers[message];
	int headerLen = *messagePointer;
	messagePointer += headerLen; // Pass over dialogue header

	if ((message == lastLine) && (GetStringLength(messagePointer, headerLen) < GetStringLength(line, headerLen))) { // note that if there's extra padding at the end this won't be accounted for
		printf("ERROR - updating dialogue failed - new dialogue line is longer than the old line. New message is %d characters longer.\n", GetStringLength(line, headerLen) - GetStringLength(messagePointer, headerLen));
		return;
	}
	else if (message < lastLine) {
		int over = GetStringLength(line, headerLen) - (dialoguePointers[message + 1] - (dialoguePointers[message] + headerLen)); // latter term is the "true" space for the string
		if (0 < over) {
			printf("ERROR - updating dialogue failed - new dialogue line is longer than the old line. New message is %d characters longer.\n", over);
			return;
		}
	}

	memcpy(messagePointer, (char*)line.c_str(), line.length());
	*(messagePointer + line.length()) = 0; // Null-terminate
	return;

}

void UpdateDialogue(LevelWadClass* levelWad, int area, int mobyClass, int message, string line) {

	// Get the first one
	int index = levelWad->layoutClasses[area].GetMobyIndexFromClass(mobyClass)[0];
	if (index == -1) {
		printf("ERROR - updating dialogue failed - area %d, mobyClass %d.\n", area, mobyClass);
		return;
	}
	UpdateDialogueFromIndex(levelWad, area, index, message, line);
	return;

}

void UpdateDialogue(LevelWadClass* levelWad, int area, int mobyClass, int type, int message, string line) {

	int index = levelWad->layoutClasses[area].GetMobyIndexFromNpcType(mobyClass, type);
	if (index == -1) {
		printf("ERROR - updating dialogue failed - area %d, mobyClass %d, type %d.\n", area, mobyClass, type);
		return;
	}
	UpdateDialogueFromIndex(levelWad, area, index, message, line);
	return;

}

void UpdateName(LevelWadClass* wad, int area, int index, string name) {
	char* tag = wad->layoutClasses[area].GetTagPointer(index);
	char* oldName = wad->layoutClasses[area].layoutData + *(int*)(tag + 12);

	int oldLen = 4 * ((strlen(oldName) / 4) + 1);
	int newLen = 4 * ((name.length() / 4) + 1);

	if (newLen > oldLen) {
		printf("ERROR - cannot update name, new string too long.\n");
		return;
	}

	char* newName = (char*)calloc(oldLen, 1);
	memcpy(newName, &(name[0]), name.length());
	memcpy(oldName, newName, oldLen);
	free(newName);
	return;
}

void UpdateName(LevelWadClass* wad, int area, int mobyClass, int type, string name) {
	int index = wad->layoutClasses[area].GetMobyIndexFromNpcType(mobyClass, type);
	if (index == -1) {
		printf("ERROR - updating name failed - area %d, mobyClass %d, type %d.\n", area, mobyClass, type);
		return;
	}
	UpdateName(wad, area, index, name);
	return;
}

void UpdateMessageOffset(LevelWadClass* wad, int area, int index, int from, int to) {
	/* WARNING - DO NOT USE THIS BEFORE UPDATING DIALOGUE MESSAGES */
	char* tag = wad->layoutClasses[area].GetTagPointer(index);
	int* messages = (int*)(tag + 16);

	// CHECK NUMBER OF MESSAGES TO SEE IF FROM AND TO ARE VALID!!!
	int lastMessage = (int)tag[3];
	if (from < 0 || to < 0 || from > lastMessage || to > lastMessage) {
		printf("ERROR - invalid message numbers passed to UpdateMessageOffset.\n");
		return;
	}

	messages[from] = messages[to];
	return;
}

void UpdateMessageOffset(LevelWadClass* wad, int area, int mobyClass, int type, int from, int to) {
	/* WARNING - DO NOT USE THIS BEFORE UPDATING DIALOGUE MESSAGES */
	int index = wad->layoutClasses[area].GetMobyIndexFromNpcType(mobyClass, type);
	if (index == -1) {
		printf("ERROR - updating dialogue offset - area %d, mobyClass %d, type %d.\n", area, mobyClass, type);
		return;
	}
	UpdateMessageOffset(wad, area, index, from, to);
	return;
}

void UpdateSkybox(const char* skyPath, char* skyPointer) {

	if (!fileExists(skyPath)) {
		printf("ERROR - updating a skybox failed. Provided file could not be found - '%s'.\n",skyPath);
		return;
	}

	// Open file
	fstream openSky(skyPath, ios_base::in | ios_base::out | ios_base::binary);
	streampos fileSize;
	openSky.seekg(0, ios::end);
	fileSize = openSky.tellg();
	openSky.seekg(0, ios::beg);

	// First int is the length of the skybox - only do the min of what's loaded just in case
	int origLen = *(int*)skyPointer;

	// SpyroEdit skyboxes are 4 bytes too big (I think?)
	int newLen = min(origLen, (int)fileSize);

	// Probably best to just ignore the first int, because that tells us the length of this block
	openSky.seekg(4);
	openSky.read(skyPointer + 4, newLen - 4);

	// Correct the offsets because they're all off by 4 (decrease by 4 please)
	int* offsetPointer = (int*)skyPointer + 2;
	int sectors = *offsetPointer;
	for (int i = 0; i < sectors; i++) {
		offsetPointer[i + 1] -= 4;
	}

	openSky.close();

	return;
}

void UpdateVram(const char* vramPath, char* vramPointer) {

	if (!fileExists(vramPath)) {
		printf("ERROR - updating VRAM failed. Provided file could not be found - '%s'.\n", vramPath);
		return;
	}

	// Open file
	fstream openVram(vramPath, ios_base::in | ios_base::out | ios_base::binary);
	openVram.seekg(0, ios::beg);
	openVram.read(vramPointer, 0x80000);
	openVram.close();

	return;
}

void UpdateVram(short* vram, const char* filename, int x, int y, int w, int h) {

	if (!fileExists(filename)) {
		printf("ERROR - updating VRAM failed. Provided file could not be found - '%s'.\n", filename);
		return;
	}

	// Open file
	short* vramFromFile = (short*)malloc(0x80000);
	fstream openVram(filename, ios_base::in | ios_base::out | ios_base::binary);
	openVram.seekg(0, ios::beg);
	openVram.read((char*)vramFromFile, 0x80000);
	openVram.close();

	// Copy from file
	for (int i = 0; i < h; i++) {
		memcpy(
			(char*)(vram + (0x200 * (y + i)) + x),
			(char*)(vramFromFile + (0x200 * (y + i)) + x),
			w * 2
		);
	}

	free(vramFromFile);
	return;

}

void UpdateBlock(const char* blockPath, char* blockPointer) { // e.g. textures

	if (!fileExists(blockPath)) {
		printf("ERROR - updating data block failed. Provided file could not be found - '%s'.\n", blockPath);
		return;
	}

	// Open file
	fstream openFile(blockPath, ios_base::in | ios_base::out | ios_base::binary);
	streampos fileSize;
	openFile.seekg(0, ios::end);
	fileSize = openFile.tellg();
	openFile.seekg(0, ios::beg);

	// First int is the length of the block - only do the min of what's loaded just in case
	int origLen = *(int*)blockPointer - 4; // Not including the block length itself
	int newLen = min(origLen, (int)fileSize);

	openFile.seekg(0, ios::beg);
	openFile.read(blockPointer + 4, newLen);
	openFile.close();

	return;
}

void UpdateLighting(const char* blockPath, char* blockPointer) {

	if (!fileExists(blockPath)) {
		printf("ERROR - updating lighting failed. Provided file could not be found - '%s'.\n", blockPath);
		return;
	}

	// Open file
	fstream openFile(blockPath, ios_base::in | ios_base::out | ios_base::binary);
	streampos fileSize;
	openFile.seekg(0, ios::end);
	fileSize = openFile.tellg();
	openFile.seekg(0, ios::beg);

	// First int is the length of the block - only do the min of what's loaded just in case
	int origLen = *(int*)blockPointer - 4; // Not including the block length itself
	int newLen = min(origLen, (int)fileSize);

	char* buffer = (char*)malloc(newLen);

	openFile.seekg(0, ios::beg);
	openFile.read(buffer, newLen);
	openFile.close();

	int initialBlockLength = *(int*)buffer;
	int count = *(int*)(buffer + initialBlockLength);
	int* pointers = (int*)(buffer + initialBlockLength + 4);
	unsigned int offset = pointers[0] - *(int*)(blockPointer + initialBlockLength + 8); // relative to blockpointer, 0 is len, 4 is initial block [length], 4+blockLen is count
	
	for (int i = 0; i < count; i++) {
		pointers[i] -= offset;
	}

	memcpy(blockPointer + 4, buffer, newLen);

	return;
}

void UpdateMobyOffset(LevelWadClass* wad, int area, int count) {
	// This should really have been done in AddMoby, but unfortunately I put taht method on the layout rather than the wad
	// Area is the area that a moby has been added to, so we increment all areas after it
	int i = 1;
	while (area + i < wad->sublevelCount) {
		auto header = (LayoutHeader*)(wad->layoutClasses[area + i].layoutData);
		(header->mobyOffset) += count;
		i++;
	}
	if (((LayoutHeader*)(wad->layoutClasses[wad->sublevelCount - 1].layoutData))->mobyOffset + wad->layoutClasses[wad->sublevelCount - 1].mobyBlock->count > 256) {
		cout << "WARNING - there may be too many mobys in a level updated with UpdateMobyOffset." << endl;
	}
	return;
}

void AddSpecialGem(LevelWadClass* wad, int area, Vector3D position, int superSecretSpecialVersion) {

	Moby gem = sampleGem; // rather than duplicating anything - check that this doesn't mean I have any rendering issues etc.
	gem.levelSectorIndex = superSecretSpecialVersion ? 27 : 26;
	gem.position = position;
	wad->layoutClasses[area].AddMoby(gem, sampleGemTag);

	if (area + 1 < wad->sublevelCount) {
		int offset = wad->layoutClasses[area].mobyBlock->count - ((LayoutHeader*)(wad->layoutClasses[area + 1].layoutData))->mobyOffset;
		UpdateMobyOffset(wad, area, offset);
	}
	
	return;

}

void DebugPrintPosition(LevelWadClass* wad, int area, int index) {
	cout << "DEBUG - REMOVE LATER - moby at ";
	cout << wad->layoutClasses[0].mobys[index].position.x << ", ";
	cout << wad->layoutClasses[0].mobys[index].position.y << ", ";
	cout << wad->layoutClasses[0].mobys[index].position.z << endl;
	return;
}

void UpdateLevels(char* wadPath, char* exePath) {
	Moby sampleEgg;
	vector<char> sampleEggTag;

#ifdef _DEBUG
	cout << endl << "== DEBUG - Stormy mode " << (stormyMode ? "ACTIVE" : "INACTIVE") << " ==" << endl;
#endif
	
	for (int levelNo = 0; levelNo < 37; levelNo++) {

#ifdef _DEBUG
		//cout << endl << "== DEBUG - Updating Level " << levelNo << " ==" << endl;
#endif

		LevelWadClass LevelWad(wadPath, levelNo);
		LevelData LevelData(LevelWad.levelDataBlock, LevelWad.levelDataHeader.length);

#ifdef _DEBUG
		/*
		for (int i = 0; i < LevelWad.sublevelCount; i++) {
			vector<int> indices = LevelWad.layoutClasses[i].GetMobyIndexFromClass(132);
			for (int j = 0; j < indices.size(); j++) {
				cout << "Level " << levelNo << " - area " << i << " - egg found! ";
				cout << (int)LevelWad.layoutClasses[i].mobys[j].unka << " / "; // 
				cout << (int)LevelWad.layoutClasses[i].mobys[j].unkb << " / "; // always 0
				cout << (int)LevelWad.layoutClasses[i].mobys[j].levelSectorIndex << " / "; // 0 or 1, seems random?
				cout << (int)LevelWad.layoutClasses[i].mobys[j].unkc << endl; // usually 7, 6 for the Take it to the Bank egg in Dino Mines?
			}
		}
		*/
#endif

		if (levelNo < eggData.size()) {
			LevelWad.UpdateEggs(eggData[levelNo]);
			LevelData.UpdateEggNames(eggData[levelNo]); // TODO FIX THIS - it might work now, previously had the egg block end too early, check A9L Name 2
		}
		// update dragon names etc:
		// LevelWad.UpdateEggNames(LevelData.originalOffsets[9] + 4, levelNo, LevelData.numberOfEggs);
		if (levelNo == 0) {

			// Get a basic egg for later
			int eggIndex = (LevelWad.layoutClasses[0].GetMobyIndexFromClass(132))[0];
			sampleEgg = LevelWad.layoutClasses[0].GetMoby(eggIndex);
			sampleEggTag = LevelWad.layoutClasses[0].GetMobyTag(eggIndex, 24);
			sampleEggTag[0] = 0; // Dragon number set to 0

			// Also gem
			int gemIndex = (LevelWad.layoutClasses[0].GetMobyIndexFromClass(1))[0];
			sampleGem = LevelWad.layoutClasses[0].GetMoby(gemIndex);
			int len = LevelWad.layoutClasses[0].GetTagLength(gemIndex);
			sampleGemTag = LevelWad.layoutClasses[0].GetMobyTag(gemIndex, len);
			sampleGem.levelSectorIndex = 1; // Make its value 1

			// Egg reqs
			vector<int> portals = LevelWad.layoutClasses[0].GetMobyIndexFromClass(988);
			for (int i = 0; i < portals.size(); i++) {
				Moby988Tag* portalTag = (Moby988Tag*)(LevelWad.layoutClasses[0].GetTagPointer(portals[i]));
				if (portalTag->LevelId == 13) {
					portalTag->EggReq = (explorationMode ? 1 : 8);
				}
				else if (portalTag->LevelId == 14) {
					portalTag->EggReq = (explorationMode ? 1 : 11);
				}
				else if (portalTag->LevelId == 15) {
					portalTag->EggReq = 16;
				}
			}

			// Move rock (561)
			Vector3D position = { 87174, 26825, 26688 }; // old 86624, 26825, 26638
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(561, { position });

			// Life jar
			vector<Vector3D> positions = {
				{  73043, 37110, 27351 },
				{ 112187, 56367, 24390 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(563, positions);

			// Vases
			positions = {
				{ 67926, 62896, 24278 },
				{ 69051, 63818, 24279 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(201, positions);

			// Zoes
			vector<Vector3D> from = {
				{ 88422, 52070, 22272 },
				{ 28723, 43397, 20224 }
			};
			vector<PosInfo> to = {
				{ { 51515, 48826, 28684 }, { 0, 0, 0xED } },
				{ { 46665, 16335, 21312 }, { 0, 0, 0x59 } }
			};
			for (int i = 0; i < from.size(); i++) {
				int index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(272, from[i]);
				//cout << "Level 10 Zoe:" << endl;
				//DebugPrintPosition(&LevelWad, 0, index);
				LevelWad.layoutClasses[0].UpdateMobyPosition(index, to[i]);
			}

			Moby gem = sampleGem;
			gem.position = { 42776, 54904, 27810 };
			gem.levelSectorIndex = 25;
			LevelWad.layoutClasses[0].AddMoby(gem, sampleGemTag);

			// Moneybags - dialogue - class 198 type 3 msg 5
			string line = "Spyro! You're looking a little... uhm... different since we beat the Sorceress together last time. Did you get a tan? It looks... ahem... well... Anyway, the Sorceress was generous enough to hire me again. I have some new items for sale, so be sure to come talk to me when you see me around.";
			UpdateDialogue(&LevelWad, 0, 198, 3, 5, line);

			// Bianca - class 425 type 1 msg 0
			line = "Spyro, it's such a relief to see you. I don't know how she did it, but the Sorceress has returned and she's furious that you defeated her all those years ago. I've never seen her like this. I don't think she'll stop until she takes over every world in this realm! She's stolen another batch of dragon eggs to fuel her magic and hidden them in all new places. These places are... unimaginable. More difficult! More perilous! More secret! This is serious. What do you say, Spyro? Can you save the dragon kingdom one more time?";
			UpdateDialogue(&LevelWad, 0, 425, 1, 0, line);

			// Zoe - class 272 type 10 msg 0
			line = "Try gliding out of bounds behind me and then turning around to face the water below us. If you charge the edge of the water or headbash somewhere in the middle with good timing, you'll be able to swim in the air! I've heard that there are similar spots in some of the other worlds.";
			UpdateDialogue(&LevelWad, 0, 272, 10, 0, line);

			// Zoe - class 272 type 2 msg 0
			line = "You can get extra height with superfly by flapping your wings with the < button, holding down, then charging all in quick succession.";
			UpdateDialogue(&LevelWad, 0, 272, 2, 0, line);

			// Zoe - class 272 type 11
			MoveDialogueStringForwards(&LevelWad, 0, 272, 11, 1, 56);
			line = "Did you know you can charge jump from some surfaces that are too steep to stand on? The trick is that you need to be facing downhill when you touch the ground to be able to get a jump.";
			UpdateDialogue(&LevelWad, 0, 272, 11, 0, line);
			MoveDialogueStringForwards(&LevelWad, 0, 272, 11, 2, 16);
			line = "Try reaching the 25 gem on top of that tree using the technique.&&I'm up for the challenge.&I'll just swim there."; // TODO - is this good enough?
			UpdateDialogue(&LevelWad, 0, 272, 11, 1, line);
			MoveDialogueStringForwards(&LevelWad, 0, 272, 11, 3, 76);
			line = "Perfect! Be sure to hold < and > together before you make contact with the ground.";
			UpdateDialogue(&LevelWad, 0, 272, 11, 2, line);
			line = "Okay then. Get Active Mode'd, nerd.";
			UpdateDialogue(&LevelWad, 0, 272, 11, 3, line);

			// Hunter - class 416 type 0
			MoveDialogueStringBackwards(&LevelWad, 0, 416, 0, 4, 40);
			UpdateDialogue(&LevelWad, 0, 416, 0, 0, "AAAAUUUGGH!!");
			UpdateDialogue(&LevelWad, 0, 416, 0, 1, "AAAAUUUGGH!!");
			UpdateDialogue(&LevelWad, 0, 416, 0, 2, "IT'S A M-MONSTER! AAAAUUUGGH!! DON'T HURT ME!");
			UpdateDialogue(&LevelWad, 0, 416, 0, 3, "IT'S A M-MONSTER! AAAAUUUGGH!! DON'T HURT ME!");
			UpdateDialogue(&LevelWad, 0, 416, 0, 4, "Oh, it's just you, Spyro. I didn't recognize you at first. Here's some crap.");

			// Delete this
			/*
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1);
			for (int i = 0; i < indices.size(); i++) {
				//cout << "State: " << (int)LevelWad.layoutClasses[0].mobys[indices[i]].state << "; ";
				//cout << "subtate: " << (int)LevelWad.layoutClasses[0].mobys[indices[i]].substate << "; ";
				//cout << "animid: " << (int)LevelWad.layoutClasses[0].mobys[indices[i]].animationState.id << "; ";
				//cout << "animframe: " << (int)LevelWad.layoutClasses[0].mobys[indices[i]].animationState.frame << "; ";
				//cout << "r: " << (int)(LevelWad.layoutClasses[0].mobys[indices[i]].colour % 256) << "; ";
				//cout << "subtype: " << (int)LevelWad.layoutClasses[0].mobys[indices[i]].subtype << "; ";
				//cout << "sector: " << (int)LevelWad.layoutClasses[0].mobys[indices[i]].levelSectorIndex << endl;
				LevelWad.layoutClasses[0].mobys[indices[i]].levelSectorIndex = 26;
			}
			printf("WARNING - Make sure Sunrise Spring doesn't contain tonnes of gems in the release build.\n");
			*/
			// 26 IS THE NEW GEM!

		}
		else if (levelNo == 1) {
			// 1. The egg in the water can currently be collected by hopping into the fountain and having Spyro touch the ground before the buoyancy takes effect and brings him back up. This, I think, can be solved by raising the water level or by lowering the ground level and the egg - whatever is easier.

			// Move EOL NPC (249,3)
			Vector3D position = { 53487, 51808, 22998 };
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(249, 3, position);

			// Deprecated - Hunter removed
			/*
			// Hunter - change messages 0 and 1 to point at messages 8 and 7 instead (experimental)
			//int hunterIndex = LevelWad.layoutClasses[2].GetMobyIndexFromClass(313)[0];
			//char* sparxTag = LevelWad.layoutClasses[0].GetTagPointer(hunterIndex);
			//int* dialoguePointers = (int*)(sparxTag + 16); // 0xC is name, 0x10 is msg 0
			//dialoguePointers[0] = dialoguePointers[8];
			//dialoguePointers[1] = dialoguePointers[7];
			*/

			// Whirlwind
			int index = (LevelWad.layoutClasses[0].GetMobyIndexFromClass(1023))[0];
			Moby whirlwind = LevelWad.layoutClasses[0].GetMoby(index);
			int tagLen = LevelWad.layoutClasses[0].GetTagLength(index);
			vector<char> tag = LevelWad.layoutClasses[0].GetMobyTag(index, tagLen);
			whirlwind.position = { 38214, 47597, 13785 };
			whirlwind.angle = { 0, 0, 0x50 };
			*(int*)&(tag[0]) = 12000; // height
			LevelWad.layoutClasses[0].AddMoby(whirlwind, tag);

			// Councillor George and Zoe
			index = LevelWad.layoutClasses[0].GetMobyIndexFromNpcType(249, 0);
			position = LevelWad.layoutClasses[0].mobys[index].position;
			position.z += 1577;
			int rot = LevelWad.layoutClasses[0].mobys[index].angle.z;
			LevelWad.layoutClasses[0].DeleteMoby(index);
			index = LevelWad.layoutClasses[0].GetMobyIndexFromNpcType(272, 12);
			PosInfo posInfo = { position, { 0, 0, rot } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posInfo);

			// Zoe
			index = LevelWad.layoutClasses[0].GetMobyIndexFromNpcType(272, 11);
			posInfo = { { 86629, 63605, 21503 }, { 0, 0, 0x78 } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posInfo);

			// Rapunzel
			index = (LevelWad.layoutClasses[1].GetMobyIndexFromClass(256))[0];
			LevelWad.layoutClasses[1].DeleteMoby(index);

			// Move some gems
			vector<Vector3D> positions = {
				{ 173156, 11875, 26872 },
				{ 179631, 10476, 26888 },
				{ 157672, 38763, 26710 }
			};
			LevelWad.layoutClasses[2].UpdateMobyPositionsByClass(1, positions);
			positions = {
				{ 186791, 15844, 26306 },
				{ 189092, 21691, 26678 },
				{ 165078, 38586, 23258 }
			};
			LevelWad.layoutClasses[2].UpdateMobyPositionsByClass(200, positions);
			vector<int> indices = LevelWad.layoutClasses[2].GetMobyIndexFromClass(1);
			LevelWad.layoutClasses[2].mobys[indices[2]].levelSectorIndex = 25;

			// Hunter
			index = LevelWad.layoutClasses[2].GetMobyIndexFromClass(313)[0];
			LevelWad.layoutClasses[2].DeleteMoby(index);

			// Zoe - dialogue - class 272 type 11 msgs 0/1
			string line = "Hi, Spyro! Be sure to read signs outside of side areas and talk to anyone you see inside. There's often useful information that can be learned that way.";
			MoveDialogueStringForwards(&LevelWad, 0, 272, 11, 1, 104);
			UpdateDialogue(&LevelWad, 0, 272, 11, 0, line);
			UpdateDialogue(&LevelWad, 0, 272, 11, 1, "!!"); // zeroed out this line of dialogue
			UpdateDialogue(&LevelWad, 0, 272, 12, 0, "You can download the 'Exploration Build' for easier progress if you get stuck.");

			// Citizen Horace - 249 5 0/1
			MoveDialogueStringForwards(&LevelWad, 0, 249, 5, 1, 8);
			line = "P-yoo! Yuck! You stink!";
			UpdateDialogue(&LevelWad, 0, 249, 5, 0, line);
			line = "Your scales are filthy! You should find somewhere to rinse off.";
			UpdateDialogue(&LevelWad, 0, 249, 5, 1, line);

			// Mayor Leo (249,3)
			UpdateName(&LevelWad, 0, 249, 3, "Leonidas");

			// Marco
			line = "There's nothing in here. Look somewhere else!";
			UpdateDialogue(&LevelWad, 1, 249, 1, 0, line);

			// Sign
			MoveDialogueStringBackwards(&LevelWad, 0, 683, 110, 2, 24);
			UpdateDialogue(&LevelWad, 0, 683, 110, 0, line);
			UpdateDialogue(&LevelWad, 0, 683, 110, 1, line);
			UpdateDialogue(&LevelWad, 0, 683, 110, 2, line);

			// Experimental - lower the effective height of the egg
			// int bathEgg = LevelWad.layoutClasses[0].GetEggIndexByNumber(4);
			// LevelWad.layoutClasses[0].mobys[bathEgg].unknown2[0] = 0x300;
			// Doesn't work - Spyro's headbash (~15910) goes lower than the base of the fountain (15974) anyway

			//cout << "Collision block has length " << LevelData.collision.length << " in Sunny Villa." << endl;
			//cout << "Collision block has " << LevelData.collision.collisionHeader->triCount << " polygons in Sunny Villa." << endl;

			// Fountain floor
			//cout << "Found " << LevelData.collision.GetTrisInCylinder(300, 1500, { 22865, 35764, 15577 }).size() << " polygons in cylinder." << endl; // 256, 1720
			vector<int> polys = LevelData.collision.GetTrisInCylinder(300, 1500, { 22865, 35764, 15577 });
			for (int i = 0; i < polys.size(); i++) {
				LevelData.collision.triangles[polys[i]].UpdateZ(-208);
			}

			// Fountain water
			//cout << "Found " << LevelData.collision.GetTrisInCylinder(300, 1700, { 22865, 35764, 16000 }).size() << " polygons in cylinder." << endl; // 256, 1720
			polys = LevelData.collision.GetTrisInCylinder(300, 1700, { 22865, 35764, 16000 });
			for (int i = 0; i < polys.size(); i++) {
				LevelData.collision.triangles[polys[i]].UpdateZ(96);
			}

			UpdateSkybox("resources/11.sky", LevelData.skyboxPointer);
			UpdateLighting("resources/11.lighting", LevelData.block1Pointer);
		}
		else if (levelNo == 2) { // Cloud Spires

			// Move EOL NPC (222,3), Cirrus (222, 2) and //Nimbus (222, 1)
			PosInfo posinfo = { { 36518, 57587, 29205 }, { 0, 0, 0x8D } };
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(222, 3, posinfo);
			posinfo = { { 43126, 48108, 5079 }, { 0, 0, 0xCC } };
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(222, 2, posinfo);
			//posinfo = { { 23093, 45854, 10762 }, { 0, 0, 0x35 } };
			//LevelWad.layoutClasses[0].UpdateNpcPositionByType(222, 1, posinfo);

			// Zoes
			posinfo = { { 32595, 70526, 11775 }, { 0, 0, 0x00 } };
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(272, 7, posinfo);
			posinfo = { { 56779, 28514, 11844 }, { 0, 0, 0xD7 } }; // 56779, 28514, 11854, D7 // { { 56779, 28514, 11854 }, { 0, 0, 0xD5 } } // { { 69255, 33251, 13626 }, { 0, 0, 0x6F } } // { { 46068, 71234, 19471 }, { 0, 0, 0x8E } };
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(272, 6, posinfo);

			// Move Moneybags (198)
			posinfo = { { 54980, 49122, 16424 }, { 0, 0, 0x2C } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(198, { posinfo });

			// Life jar
			posinfo = { { 85740, 97498, 19721 }, { 0xF7, 0xF1, 0x16 } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(563, { posinfo });

			// EOL portal
			posinfo = { { 34012, 55394, 29206 }, { 0, 0, 0x36 } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(182, { posinfo });

			// Nimbus things - the tag is huge and so there's tag offsets inside the tag
			//int nimbus = LevelWad.layoutClasses[0].GetMobyIndexFromNpcType(222, 1);
			//char* tag = LevelWad.layoutClasses[0].GetTagPointer(nimbus);
			//Vector3D* vec = (Vector3D*)(LevelWad.layoutClasses[0].layoutData + *(int*)(LevelWad.layoutClasses[0].layoutData + *(int*)(tag + 0x58) + 12)); // yikes
			//cout << vec->x << ", " << vec->y << ", " << vec->z << endl;
			//*vec = { 23094, 45853, 10762 }; // 48181, 71670, 31551

			// Fluffy things
			int fluffy = LevelWad.layoutClasses[0].GetMobyIndexFromNpcType(222, 3);
			char* tag = LevelWad.layoutClasses[0].GetTagPointer(fluffy);
			Vector3D* vec = (Vector3D*)(LevelWad.layoutClasses[0].layoutData + *(int*)(LevelWad.layoutClasses[0].layoutData + *(int*)(tag + 0x58) + 12)); // yikes again
			//cout << vec->x << ", " << vec->y << ", " << vec->z << endl;
			*vec = { 34012, 55394, 29206 }; // EOL portal

			// Zoe render distance
			int index = LevelWad.layoutClasses[0].GetMobyIndexFromNpcType(272, 6);
			LevelWad.layoutClasses[0].mobys[index].lowDrawDistance = 0;
			LevelWad.layoutClasses[0].mobys[index].highDrawDistance = 0;

			// Whirlwind
			Vector3D pos = { 45957, 48343, 5632 }; // needs to match the egg on the pedestal, whichever that is
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(1023, pos);
			posinfo = { { 84833, 111691, 8152 }, { 0, 0, 0xCB } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posinfo);
			*(int*)(LevelWad.layoutClasses[0].layoutData + LevelWad.layoutClasses[0].mobys[index].mobyTag) = 8298; // something like this

			// Exploration Whirlwind
			if (explorationMode) {
				PosInfo whPos = { { 37642, 44859, 9174 }, { 0, 0, 0x53 } };
				LevelWad.layoutClasses[0].DuplicateMoby(1023, 0, whPos);
				int* whTag = (int *)(LevelWad.layoutClasses[0].GetTagPointer(LevelWad.layoutClasses[0].mobyBlock->count - 1));
				whTag[0] = 23000; // height
				whTag[2] = 0; // type
			}

			// MobyTag + 0x1d is -1 for all unlinked big rhynocs (188), and is the index of the eggs otherwise
			// Get all big Rhynocs and find the ones linked to eggs
			// Get the index of "Guarded Twice" egg (dragon 5)
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(188);
			int guardedEgg = LevelWad.layoutClasses[0].GetEggIndexByNumber(5);

			// Move linked Rhynoc into the appropriate spot, otherwise disable the link
			for (int i = 0; i < indices.size(); i++) {
				int len = LevelWad.layoutClasses[0].GetTagLength(indices[i]);
				if (4 * 0x1e <= len) {
					int* link = ((int*)(LevelWad.layoutClasses[0].layoutData + LevelWad.layoutClasses[0].mobys[indices[i]].mobyTag) + 0x1d);
					if (*link != -1) {
						if (*link == guardedEgg) {
							// We're switching the links - make this one unlinked, because this one usually despawns on low difficulties
							*link = -1;
						}
						else {
							// Make this one match the guarded egg position
							*link = guardedEgg;
							LevelWad.layoutClasses[0].mobys[indices[i]].position = { 67500, 76452, 14297 };
							LevelWad.layoutClasses[0].mobys[indices[i]].angle.z = 0x81;
							LevelWad.layoutClasses[0].mobys[indices[i]].lowDrawDistance *= 2; // need these, otherwise it can appear during the egg animation
							LevelWad.layoutClasses[0].mobys[indices[i]].highDrawDistance *= 2;
							//printf("DEBUG - Cloud Spires enemy move successful.\n");
						}
					}
				}
			}

			// Winged Rhynocs
			vector<Vector3D> from = {
				{ 74629, 108933, 9216 },
				{ 73390, 97075, 10240 }
			};
			vector<PosInfo> to = {
				{ { 35918, 55441, 29206 }, { 0, 0, 0x5E } },
				{ { 33101, 57146, 29209 }, { 0, 0, 0x17 } }
			};
			for (int i = 0; i < from.size(); i++) {
				int index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(188, from[i]);
				LevelWad.layoutClasses[0].UpdateMobyPosition(index, to[i]);
			}

			// Spirits
			vector<PosInfo> spiritPositions = {
				{ { 146708,  77741,  3608 }, { 0, 0, 0x5A } },
				{ { 129005,  96464,  8500 }, { 0, 0, 0xDB } },
				{ { 141318, 106932, 14172 }, { 0, 0, 0xC1 } },
				{ { 117742, 107226, 15194 }, { 0, 0, 0x0C } },
				{ { 136705,  74407, 16650 }, { 0, 0, 0x1B } },
				{ { 105471,  80897,  6337 }, { 0, 0, 0xDD } }
			};
			// Centre of spirits actually at (Vector3D*)(MobyTag + 0x34), planar angle seems to be at 0x30 but I'm not sure what it represents exactly
			// 0x44 seems to be related to the height
			vector<int> spiritIndices = LevelWad.layoutClasses[2].GetMobyIndexFromClass(638);
			int total = min(spiritIndices.size(), spiritPositions.size());
			for (int i = 0; i < total; i++) {
				char* mobyTag = (LevelWad.layoutClasses[2].layoutData + LevelWad.layoutClasses[2].mobys[spiritIndices[i]].mobyTag);
				*(Vector3D*)(mobyTag + 0x34) = spiritPositions[i].pos;
				*(int*)(mobyTag + 0x30) = spiritPositions[i].angle.z;
			}

			// Cloud
			index = LevelWad.layoutClasses[2].GetMobyIndexFromClass(223)[0];
			LevelWad.layoutClasses[2].mobys[index].scaleDown = 15;
			LevelWad.layoutClasses[2].mobys[index].position.z -= 800;

			// Gems
			from = { // just reusing this
				{ 87548, 106902, 14175 },
				{ 85981, 103273, 14917 },
				{ 42587,  71662, 25974 }, // 10
				{ 51422,  71650, 25974 }, // 10
				{ 47052,  71625, 29974 }, // 25
				{ 39052, 116931, 20158 },
				{ 40793, 116835, 19993 },
				{ 73099,  35318, 16277 },
				{ 71505,  34193, 16393 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, from);
			from = {
				{ 126619, 71274, 16007 },
				{ 124379, 72263, 16157 }
			};
			LevelWad.layoutClasses[2].UpdateMobyPositionsByClass(1, from);

			indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1);
			LevelWad.layoutClasses[0].mobys[indices[2]].levelSectorIndex = 10;
			LevelWad.layoutClasses[0].mobys[indices[3]].levelSectorIndex = 10;
			LevelWad.layoutClasses[0].mobys[indices[4]].levelSectorIndex = 25;

			// Baskets
			vector<PosInfo> posinfos = {
				{ { 36247, 116716, 21826 }, { 0xF8, 0xF5, 0xE5 } },
				{ { 68506, 42977, 15622 }, { 0, 0, 0 } }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(200, posinfos);

			// Vases
			from = {
				{ 67977, 46302, 14294 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(201, from);

			// Move sublevel entrance
			indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1022);
			int visualIndex = -1;
			PosInfo portalPos = { { 48046, 13427, 13200 }, { 0, 0x40, 0 } };
			for (int i = 0; i < indices.size(); i++) {
				Moby* moby = LevelWad.layoutClasses[0].mobys + indices[i];
				Moby1022Tag* tag = (Moby1022Tag*)(LevelWad.layoutClasses[0].GetTagPointer(indices[i]));
				if (tag->fromArea == 0 && tag->toArea == 1) { // Sun Seeds
					// Entrance
					visualIndex = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(3, moby->position);
					moby->position = portalPos.pos;
					tag->box.pos = moby->position;
					tag->unknown.pos = moby->position;
					tag->entranceZWidth = 600;
				}
			}
			if (visualIndex != -1) {
				//cout << "Moved a portal visual in Cloud Spires!" << endl;
				LevelWad.layoutClasses[0].UpdateMobyPosition(visualIndex, portalPos);
			}

			// Area 1 exit
			posinfo = { { 56461, 27141, 10031 }, { 0, 0, 0x3D } }; // (briefly 56562, 27167, 10430, but this spawned Spyro too low!) // { { 56461, 27141, 10031 }, { 0, 0, 0x3D } } // { { 57211, 27394, 10009 }, { 0xFF, 0x0B, 0x4E } } // { { 67673, 33771, 12049 }, { 0, 0, 0xF8 } }
			LevelWad.UpdateSublevelExit(1, 0, posinfo);

			// Cirrus - 222 2 0 
			string msg = "Oh, please help us, Spyro! Our city has been overrun by rhynocs! You can have this egg if you defeat some of the big, scary, winged ones.";
			UpdateDialogue(&LevelWad, 0, 222, 2, 0, msg);

			// Moneybags (198,0)
			MoveDialogueStringBackwards(&LevelWad, 0, 198, 0, 1, 4);
			MoveDialogueStringForwards(&LevelWad, 0, 198, 0, 3, 68);
			MoveDialogueStringForwards(&LevelWad, 0, 198, 0, 2, 100);
			msg = "Well, well, if it isn't my favorite dragon! I have a secret to tell you. The Sorceress told me not to say anything, but maybe I could be persuaded to let it slip... for the right price. &Pay 50 gems to learn a secret? &Sure, whatever. &No way, you doofus."; // ^^^ changed to 50
			UpdateDialogue(&LevelWad, 0, 198, 0, 0, msg);
			msg = "Alright, Spyro. Look where I'm looking... a little bit to the left... a little bit more. There you go! Don't say I never gave you a good deal. It looks like you can glide right to that egg over there. Remember, you didn't hear it from me.";
			UpdateDialogue(&LevelWad, 0, 198, 0, 1, msg);
			msg = "What? NO? What else are you going to do with all those gems? Buy flying lessons?";
			UpdateDialogue(&LevelWad, 0, 198, 0, 2, msg);
			msg = "This cloud isn't really big enough for two people. I prefer not to have... reptiles... so close to me, especially when they don't have many gems. &[A helpful clue costs ^^^ gems."; // ^^^ NOT changed to 50
			UpdateDialogue(&LevelWad, 0, 198, 0, 3, msg);

			// Zoe
			msg = "Hi, Spyro! I'm worried one of those armored rhynocs will overhear us. We'll talk more inside the Charmed Ridge portal's tower. It's a little more private.";
			UpdateDialogue(&LevelWad, 0, 272, 6, 0, msg);
			msg = "If you bait that rhynoc to attack you, dive into the abyss, then bonk the corner while the wind is over you, something cool might happen.";
			UpdateDialogue(&LevelWad, 0, 272, 7, 0, msg);

			// Delete sun seeds NPC
			index = LevelWad.layoutClasses[1].GetMobyIndexFromClass(222)[0];
			LevelWad.layoutClasses[1].DeleteMoby(index);

			UpdateSkybox((stormyMode ? "resources/12s.sky" : "resources/12.sky"), LevelData.skyboxPointer);
			UpdateLighting((stormyMode ? "resources/12s.lighting" : "resources/12.lighting"), LevelData.block1Pointer);

			// Print large rhynoc moby data and tags
			// LevelWad.layoutClasses[0].PrintMobyInfoByClass(188);
		}
		else if (levelNo == 3) { // Molten Crater
			// Move EOL NPC (229,0)
			Vector3D position = { 59430, 74133, 21098 };
			if (!explorationMode) { // challenge mode only
				LevelWad.layoutClasses[0].UpdateNpcPositionByType(229, 0, position);
			}

			// Remove the red thief
			vector<int> indices = LevelWad.layoutClasses[2].GetMobyIndexFromClass(88);
			LevelWad.layoutClasses[2].DeleteMoby(indices[1]);
			// TODO - is this even the correct one?

			// Move chest
			PosInfo posinfo = { { 27750, 44374, 14009 }, { 0, 0, 0x09 } }; // 0x19BCB is about half way in the wall // old - { { 0xF451, 0x19B4B, 10407 }, { 0, 0, 0xC0 } }
			int chestIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(205)[0];
			LevelWad.layoutClasses[0].UpdateMobyPosition(chestIndex, posinfo);

			// Move key
			Vector3D keyPos = { 50505, 83333, 13292 };
			int keyIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(212)[0];
			LevelWad.layoutClasses[0].UpdateMobyPosition(keyIndex, keyPos);

			// Move heads (85, (int*)tag[7] == -1
			indices = LevelWad.layoutClasses[1].GetMobyIndexFromClass(85);
			int placed = 0;
			vector<PosInfo> positions = {
				{ { 39546, 52645, 12422 }, { 0, 0, 0x0B } },
				{ { 39986, 60350, 14469 }, { 0, 0, 0xF2 } },
				{ { 46413, 65857, 10783 }, { 0, 0, 0xC8 } },
				{ { 54582, 61032, 16496 }, { 0, 0, 0x97 } },
				{ { 53467, 49493, 16501 }, { 0, 0, 0x5A } }
			};
			for (int i = 0; i < indices.size(); i++) {
				int* tag = (int*)(LevelWad.layoutClasses[1].layoutData + LevelWad.layoutClasses[1].mobys[indices[i]].mobyTag);
				if (tag[7] == -1) {
					LevelWad.layoutClasses[1].UpdateMobyPosition(indices[i], positions[placed]);
					placed++;
				}
				if (placed == 5) {
					break;
				}
			}

			// Tiki
			int index = LevelWad.layoutClasses[1].GetMobyIndexFromNpcType(229, 1);
			LevelWad.layoutClasses[1].DeleteMoby(index);

			// Zoe
			index = LevelWad.layoutClasses[0].GetMobyIndexFromNpcType(272, 11);
			posinfo = { { 28499, 47017, 0 }, { 0, 0, 0xE8 } };
			posinfo.pos.z = LevelWad.layoutClasses[0].mobys[index].position.z;
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posinfo);

			// Life Jar
			position = { 42535, 86184, 24667 };
			LevelWad.layoutClasses[2].UpdateMobyPositionsByClass(563, { position });

			// Armoured chest
			position = { 24344, 64295, 14151 };
			LevelWad.layoutClasses[2].UpdateMobyPositionsByClass(203, { position });

			// Stoney
			posinfo = { { 27021, 83343, 20616 }, { 0, 0, 0x21 } };
			LevelWad.layoutClasses[2].UpdateNpcPositionByType(229, 4, posinfo);

			// We should have the egg around 0x675 behind the chest, maybe more
			// Note that positive y is in direction 0x40 (opposite to the egg direction)
			// i.e. if the egg is facing in direction t radians and is at position E, and the chest is at position C,
			// |E - C| = 0x675 and arg(E - C) = t + 2*pi

			//UpdateSkybox("resources/13-old.sky", LevelData.skyboxPointer);
			//UpdateLighting("resources/13-old.lighting", LevelData.block1Pointer);

			// Area 1 exit
			posinfo = { { 65989, 75226, 20417 }, { 0x0E, 0xFC, 0x89 } }; // { { 66075, 75023, 20505 }, { 0x0E, 0xFC, 0x99 } };
			LevelWad.UpdateSublevelExit(1, 0, posinfo);

			// Zoe
			string msg = "I think there's something unusual inside that chest, Spyro. After opening the lock, try charging straight forward until you bonk your head against the wall.";
			UpdateDialogue(&LevelWad, 0, 272, 11, 0, msg);

			// Clay
			msg = (!explorationMode)
				? "Oh, you're looking for Rocky? Try charge jumping at one of the wild boars, gliding, then charging again as it tries to headbutt you. With any luck, you'll get catapulted high enough to figure out where he is."
				: "Oh, you're looking for Rocky? I saw him go into the penguin sublevel by climbing on top of it. He probably exited right away to use the secret teleport though.";
			MoveDialogueStringForwards(&LevelWad, 0, 229, 2, 1, 88);
			UpdateDialogue(&LevelWad, 0, 229, 2, 0, msg);
			UpdateDialogue(&LevelWad, 0, 229, 2, 1, "...");

			// Sign
			msg = "Nothing here but some floating heads.";
			UpdateDialogue(&LevelWad, 0, 684, 120, 0, msg);
			UpdateDialogue(&LevelWad, 0, 684, 120, 1, msg);
			UpdateDialogue(&LevelWad, 0, 684, 120, 2, msg);

			// Cpt. Byrd
			UpdateName(&LevelWad, 1, 89, 10, "Cpt. Byrd");

			UpdateSkybox("resources/13.sky", LevelData.skyboxPointer);
			UpdateVram("resources/13.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/13.texture", LevelData.block0Pointer);
			UpdateLighting("resources/13.lighting", LevelData.block1Pointer);

			// sublevels don't have a scene block here, even area 0
			//auto Scene = SceneData(0, (unsigned char*)LevelWad.sceneDataBlocks[0], LevelWad.sceneHeaders[0].length);
			//Scene.UpdateVram("resources/13.vram");
			//LevelWad.UpdateScene(0);
			// Do other sublevels
			// Seems that area 0 may not have a scene block anyway

		}
		else if (levelNo == 4) { // Seashell Shore
			
			// Move EOL NPC (271,7)
			PosInfo position = { { 48423, 56181, 18136 } , { 0, 0, 0x55 } };
			if (!explorationMode) {
				LevelWad.layoutClasses[0].UpdateNpcPositionByType(271, 7, position);
			}

			// Attempt to mess with the octopus attached to the NPC
			vector<int> octoIndices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(45);
			int sealIndex = LevelWad.layoutClasses[0].GetMobyIndexFromNpcType(271, 7);
			position = { { 48423, 56181, 21000 }, { 0, 0, 0x57 } };
			for (int i = 0; i < octoIndices.size(); i++) {
				int len = LevelWad.layoutClasses[0].GetTagLength(octoIndices[i]);
				if (len < 12) {
					// Tag isn't long enough
					continue;
				}
				int* link = ((int*)(LevelWad.layoutClasses[0].layoutData + LevelWad.layoutClasses[0].mobys[octoIndices[i]].mobyTag) + 2);
				if (*link != -1 && *link == sealIndex && !explorationMode) {
					// *link = -1;
					// LevelWad.layoutClasses[0].mobys[sealIndex].state = 0x18;
					LevelWad.layoutClasses[0].UpdateMobyPosition(octoIndices[i], position);
					//printf("DEBUG - Seashell Shore enemy move should be successful.\n");
				}
			}

			// Update state of tunnel egg, now in Bluto area (2)
			vector<int> indices = LevelWad.layoutClasses[2].GetMobyIndexFromClass(132);
			if (indices.size() != 0) {
				LevelWad.layoutClasses[2].mobys[indices[0]].state = 0;
			}

			// Make Mollie (dragon 2, now in area 0) invisible
			int index = LevelWad.layoutClasses[0].GetEggIndexByNumber(2);
			LevelWad.layoutClasses[0].mobys[index].lowDrawDistance = 0;

			// Delete NPC in tunnel sublevel
			index = LevelWad.layoutClasses[3].GetMobyIndexFromNpcType(271, 9);
			LevelWad.layoutClasses[3].DeleteMoby(index);

			// Move some cannons in area 1 (deprecated)
			/*
			indices = LevelWad.layoutClasses[1].GetMobyIndexFromClass(420);
			for (int i = 0; i < indices.size(); i++) {
				LevelWad.layoutClasses[1].mobys[indices[i]].position.y += 10000;
				LevelWad.layoutClasses[1].mobys[indices[i]].position.z += 4000;
			}
			indices = LevelWad.layoutClasses[1].GetMobyIndexFromClass(422);
			for (int i = 0; i < indices.size(); i++) {
				LevelWad.layoutClasses[1].mobys[indices[i]].position.y += 10000;
				LevelWad.layoutClasses[1].mobys[indices[i]].position.x += 4000;
			}
			*/

			// Move a few sand blocks - this should move the cannons
			indices = LevelWad.layoutClasses[1].GetMobyIndexFromClass(421);
			for (int i = 0; i < indices.size(); i++) {
				if (LevelWad.layoutClasses[1].mobys[indices[i]].position.x < 77000) {
					LevelWad.layoutClasses[1].mobys[indices[i]].position.x -= 20000;
				}
			}

			// Shovel Rhynoc
			position = { { 90752, 70957, 24940 }, { 0, 0, 0xD5 } };
			index = LevelWad.layoutClasses[1].GetClosestMobyIndexFromClass(429, { 89641, 60959, 18542 });
			LevelWad.layoutClasses[1].UpdateMobyPosition(index, position);

			// Seashell Rhynoc (295)
			position = { { 116633, 111578, 25019 }, { 0, 0, 0xCE } };
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(295, { 109578, 109476, 22005 }); // originally 109259, 109813, 21960, test this
			Vector3D* tag = (Vector3D*)LevelWad.layoutClasses[0].GetTagPointer(index);
			*tag = position.pos; // Destination / home position?
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, position);

			// Basket
			Vector3D pos = { 104571, 71506, 23987 };
			LevelWad.layoutClasses[1].UpdateMobyPositionsByClass(200, { pos });

			// Vase
			pos = { 84376, 93099, 22472 };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(201, { pos });

			// Headbash crate
			pos = { 129373, 75120, 23239 };
			LevelWad.layoutClasses[1].UpdateMobyPositionsByClass(202, { pos });

			// Gems
			pos = { 118697, 104110, 28566 }; // 10
			index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1)[0];
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, pos);
			LevelWad.layoutClasses[0].mobys[index].levelSectorIndex = 10;

			// Zoes
			vector<Vector3D> from = {
				{ 96522, 49664, 22526 },
				{ 116787, 109240, 26602 },
				{ 116900, 95683, 23536 }
			};
			vector<PosInfo> to = {
				{ { 90274, 94409, 24047 }, { 0, 0, 0x29 } },
				{ { 107089, 113037, 23954 }, { 0, 0, 0xFC } },
				{ { 60885, 89742, 28612 }, { 0, 0, 0xC0 } }
			};
			for (int i = 0; i < from.size(); i++) {
				index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(272, from[i]);
				//cout << "Zoe - Seashell Shore: ";
				//DebugPrintPosition(&LevelWad, 0, index);
				LevelWad.layoutClasses[0].UpdateMobyPosition(index, to[i]);
			}

			// Zoe
			string msg = "If you get stuck between or inside objects, sometimes you get pushed away with a lot of speed. Try walking behind that rhynoc toward the vase until he grabs a barrel.";
			UpdateDialogue(&LevelWad, 0, 272, 12, 0, msg);
			msg = "To reach ledges that are just a little too high, face away from them, jump, and spin more than 90 degrees toward the ledge while you're in the air. Try it here.";
			UpdateDialogue(&LevelWad, 0, 272, 11, 0, msg);

			UpdateSkybox("resources/14.sky", LevelData.skyboxPointer);
			UpdateLighting("resources/14.lighting", LevelData.block1Pointer);
		}
		else if (levelNo == 5) {
			// Sparx - change message 2 to point at message 0 instead (experimental)
			int sparxIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(144)[0]; // Speedway Sparx is 144
			char* sparxTag = LevelWad.layoutClasses[0].GetTagPointer(sparxIndex);
			int* dialoguePointers = (int*)(sparxTag + 16); // 0xC is name, 0x10 is msg 0
			dialoguePointers[2] = dialoguePointers[0];

			// Sparx - change msg 0 string - old implementation, very yucky
			char* messagePointer = LevelWad.layoutClasses[0].layoutData + dialoguePointers[0];
			messagePointer += *messagePointer; // Pass over dialogue header
			const char* newMessage = "Which challenge would you like to play? &Choose a challenge: &Time Attack &Can you repeat that? &Exit"; // Shorter than original
			memcpy(messagePointer, (char*)newMessage, strlen(newMessage));
			*(messagePointer + strlen(newMessage)) = 0; // Null-terminate

			// Hunter - disable
			int hunterIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(377)[0];
			LevelWad.layoutClasses[0].DeleteMoby(hunterIndex); // This is crude - maybe I just need to put him in an uninteractable state instead

			UpdateSkybox("resources/15.sky", LevelData.skyboxPointer);
			UpdateLighting("resources/15.lighting", LevelData.block1Pointer);
		}
		else if (levelNo == 6) {

			// Mushroom huts
			vector<PosInfo> positions = {
				{ { 73809, 35788, 25303 }, { 0, 0, 0x87 } },
				{ { 59251, 34852, 22490 }, { 0, 0, 0x80 } },
				{ { 79212, 57771, 23254 }, { 0, 0, 0x62 } },
				{ { 28690, 53380, 23511 }, { 0, 0, 0x41 } }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(154, positions);

			// Headbash crates - there are 6 by default
			for (int i = 0; i < 8; i++) {
				LevelWad.layoutClasses[0].DuplicateMoby(202, 0);
			}
			vector<Vector3D> posns = {
				{ 47784, 53621, 32602 },
				{ 36491, 51267, 29488 },
				{ 56940, 86420, 20700 },
				{ 51609, 53751, 34780 },
				{ 40047, 57460, 24800 },
				{ 76800, 58368, 23296 },
				{ 76800, 58368, 23696 },
				{ 76800, 58368, 24096 },
				{ 76800, 58368, 23496 },
				{ 40990, 51153, 29488 },
				{ 42399, 56166, 26660 },
				{ 57964, 89957, 20700 },
				{ 62464, 89958, 20700 },
				{ 65484, 86622, 20700 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(202, posns);

			// Goats
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(126);
			for (int i = 0; i < indices.size(); i++) {
				char* tag = LevelWad.layoutClasses[0].layoutData + LevelWad.layoutClasses[0].mobys[indices[i]].mobyTag;
				int goat = *(int*)(tag + 0x70);

				// 0 - Billy (delete)
				// 1 - Bobby
				// 2 - Pete (delete)
				if (goat != 1) {
					LevelWad.layoutClasses[0].DeleteMoby(indices[i]);
					// cout << "DEBUG - Deleted a goat (" << goat << ")" << endl;
				}
			}

			UpdateSkybox("resources/16.sky", LevelData.skyboxPointer);
			UpdateVram("resources/16.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/16.texture", LevelData.block0Pointer);
			UpdateLighting("resources/16.lighting", LevelData.block1Pointer);
		}
		else if (levelNo == 8) {

			// Deprecated changes
			/*
			// Remove the Zoe closest to a specific spot
			Vector3D position = { 187425, 57765, 10225 };
			int index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(289, position); // Tutorial Zoe is 289
			//LevelWad.layoutClasses[0].DeleteMoby(index);
			PosInfo posinfo = { { 97261, 60195, 16872 }, { 0, 0, 0x80 } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posinfo);
			*/

			// Change spawn point
			LayoutHeader* header = (LayoutHeader*)LevelWad.layoutClasses[0].layoutData;
			header->spawnPosition = { 28559, 75131, 10625 };
			header->spawnRotation = { 0, 0, 0x40 };

			// Keys
			int b = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(339, { 96481,  85791, 10547 });
			int g = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(339, { 76052, 109947, 10547 });
			int r = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(339, { 55562,  81275, 10547 });
			LevelWad.layoutClasses[0].UpdateMobyPosition(b, LevelWad.layoutClasses[0].mobys[g].position);
			Vector3D position = { 43303, 85305, 10547 };
			LevelWad.layoutClasses[0].UpdateMobyPosition(g, position);
			position = { 66193, 85464, 10547 };
			LevelWad.layoutClasses[0].UpdateMobyPosition(r, position);

			// Zoe
			int index = LevelWad.layoutClasses[0].GetMobyIndexFromNpcType(289, 3); // Tutorial Zoe is class 289
			PosInfo posInfo = { { 73886, 47469, 16900 }, { 0, 0, 0xC0 } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posInfo);

			// Zoe
			string msg = "Wow! I can't believe you made it here, Sparx! Congratulations! This is top secret, so don't tell anyone other than Spyro, OK?... There are secret eggs in Buzz's Dungeon, Spike's Arena, and Scorch's Pit. If you can find them, you can collect them and use them to move on to the next world instead of challenging the boss.";
			UpdateDialogue(&LevelWad, 0, 289, 3, 0, msg);

		}
		else if (levelNo == 9) { // Midday Garden Home

			// Egg reqs
			vector<int> portals = LevelWad.layoutClasses[0].GetMobyIndexFromClass(988);
			for (int i = 0; i < portals.size(); i++) {
				Moby988Tag* portalTag = (Moby988Tag*)(LevelWad.layoutClasses[0].GetTagPointer(portals[i]));
				if (portalTag->LevelId == 23) {
					portalTag->EggReq = (explorationMode ? 1 : 20);
				}
				else if (portalTag->LevelId == 24) {
					portalTag->EggReq = (explorationMode ? 1 : 24);
				}
				else if (portalTag->LevelId == 25) {
					portalTag->EggReq = 29;
				}
			}

			// Move flowerpots
			vector<PosInfo> positions = {
				{ { 54320, 33441,  5940 }, { 0, 0, 0xE1 } },
				{ { 58035, 29783,  5940 }, { 0, 0, 0x5F } },
				{ { 27973, 37836, 15471 }, { 0, 0, 0 } },
				{ { 84125, 33388, 11648 }, { 0, 0, 0 } }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(509, positions);
			/*TODO - For the pots that end up under the bridge, can you rotate them so they block the bridge swim-in-air?*/
			// For the last two pots, I should try reducing their render distances by half
			vector<int> potIndices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(509);
			LevelWad.layoutClasses[0].mobys[potIndices[2]].lowDrawDistance = 2;
			LevelWad.layoutClasses[0].mobys[potIndices[2]].highDrawDistance = 2;
			LevelWad.layoutClasses[0].mobys[potIndices[3]].lowDrawDistance = 8;
			LevelWad.layoutClasses[0].mobys[potIndices[3]].highDrawDistance = 8;

			// Move a basket (DEPRECATED)
			// Vector3D position = { 60919, 22099, 7126 };
			// int index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(200, position);
			// LevelWad.layoutClasses[0].UpdateMobyPosition(index, position);
			/*Move a vase to around 60919, 22099, 7526 (Find one a short distance away and move it there perhaps)*/

			// Delete thief
			int index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(519)[0];
			LevelWad.layoutClasses[0].DeleteMoby(index);

			// Move a gem
			Vector3D position = { 53559, 16502, 13395 };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, { position });

			// Sparx sign
			PosInfo posinfo = { { 21402, 26294, 13996 }, { 0, 0, 0xCF } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(325, { posinfo });

			// Hunter
			posinfo = { { 52568, 73721, 8152 }, { 0, 0, 0xD8 } };
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(499, 8, posinfo);

			// Move Zoe and make her draw distance zero
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(272, { 39383, 40141, 13312 });
			LevelWad.layoutClasses[0].mobys[index].lowDrawDistance = 0;
			LevelWad.layoutClasses[0].mobys[index].highDrawDistance = 0;
			posinfo = { { 97914, 47238, 11167 }, { 0, 0, 0x00 } }; // formerly { { 73442, 10162, 8703 }, { 0, 0, 0x20 } }
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posinfo);

			// Bianca
			string msg = "Good job so far, Spyro. The Sorceress can't be happy that you've been able to get this far in one piece. I overheard a rhynoc in Icy Peak bragging that you wouldn't even get out of Sunrise Spring alive this time. Ha! You showed him! Even so, I'm sure the Sorceress has more tricks up her sleeve. Be careful.";
			UpdateDialogue(&LevelWad, 0, 425, 10, 0, msg);

			// Hunter 499 8 0
			msg = "Someone's been putting my flower pots up in trees! What kind of sick prank is that? My poor begonias!";
			UpdateDialogue(&LevelWad, 0, 499, 8, 0, msg);

			// Zoe
			msg = "You found me! I have something important to tell you, but here's not the place to do it. There's a portal right above you when you enter Cloud City. Enter that, then look for me when you leave, OK? I might be in a wall.";
			string altmsg = "You found me! I have something important to tell you, but here's not the place to do it. There's a portal right above you when you enter Stormy Spires. Enter that, then look for me when you leave, OK? I might be in a wall.";
			UpdateDialogue(&LevelWad, 0, 272, 2, 0, (stormyMode ? altmsg : msg));

			// Sign
			msg = "Cool, now do it without swimming.";
			UpdateDialogue(&LevelWad, 0, 325, 150, 0, msg);

			// Cpt. Byrd
			UpdateName(&LevelWad, 0, 70, 7, "Cpt. Byrd");
			UpdateName(&LevelWad, 0, 669, 7, "Cpt. Byrd");

			UpdateSkybox("resources/20.sky", LevelData.skyboxPointer);
			UpdateVram("resources/20.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/20.texture", LevelData.block0Pointer);
			UpdateLighting("resources/20.lighting", LevelData.block1Pointer);

		}
		else if (levelNo == 10) { // Icy Peak

			// Move EOL NPC (317,1)
			PosInfo position = { { 60499, 39505, 16854 } , { 0, 0, 0 } };
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(317, 1, position);

			// Ice sculpture
			// This didn't work, I just can't seem to find the collision polygons here
			// I think they're maybe all moved downwards
			/*
			cout << "Found " << LevelData.collision.GetTrisInCylinder(200, 100, { 100512, 40832, 17664 }).size() << " polygons in cylinder." << endl; // 256, 1720
			vector<int> polys = LevelData.collision.GetTrisInCylinder(200, 100, { 100512, 40832, 17664 });
			for (int i = 0; i < polys.size(); i++) {
				//cout << "Primary X: " << LevelData.collision.triangles[polys[i]].GetPoints()[0].x << endl;
				LevelData.collision.triangles[polys[i]].UpdateX(-128);
			}
			cout << "Found " << LevelData.collision.GetTrisInCylinder(500, 800, { 100840, 41600, 2164 }).size() << " polygons in cylinder." << endl; // 256, 1720
			polys = LevelData.collision.GetTrisInCylinder(500, 800, { 100840, 41600, 2164 });
			for (int i = 0; i < polys.size(); i++) {
				cout << "Primary X: " << LevelData.collision.triangles[polys[i]].GetPoints()[0].x << endl;
				LevelData.collision.triangles[polys[i]].UpdateX(-160);
			}
			*/

			// Headbash crate
			position = { { 100370, 42275, 17402 }, { 0x40, 0xC0, 0x20 } };
			int index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(202)[0];
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, position);
			LevelWad.layoutClasses[0].mobys[index].lowDrawDistance = 0;
			LevelWad.layoutClasses[0].mobys[index].highDrawDistance = 0;

			// Whirlwind - widen by 300
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(1023, position.pos);
			int* tag = (int*)LevelWad.layoutClasses[0].GetTagPointer(index);
			tag[1] += 300;

			// Other whirlwind
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(1023, { 30669, 22467, 17664 });
			tag = (int*)LevelWad.layoutClasses[0].GetTagPointer(index);
			*tag = 17378;
			LevelWad.layoutClasses[0].mobys[index].angle.z = 0x13;

			// Gems
			vector<Vector3D> positions = {
				{ 68829, 42815, 22771 },
				{ 30601, 22516, 17622 }, // 25
				{ 71607, 28131, 19483 }, // 25
				{ 79636, 31487, 23016 }, // 2
				{ 79383, 32494, 24079 }  // 1
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1);
			LevelWad.layoutClasses[0].mobys[indices[1]].levelSectorIndex = 25;
			LevelWad.layoutClasses[0].mobys[indices[2]].levelSectorIndex = 25;
			LevelWad.layoutClasses[0].mobys[indices[3]].levelSectorIndex = 2;
			LevelWad.layoutClasses[0].mobys[indices[4]].levelSectorIndex = 1;

			// Baskets
			vector<PosInfo> posinfos = {
				{ { 65683, 39327, 22879 }, { 0xF5, 0xF7, 0x95 } },
				{ { 88359, 36200, 21165 }, { 0xFA, 0x09, 0x98 } }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(200, posinfos);

			// Mess with the egg links
			// These eggs should have moved out of this area so this should be fair game
			indices = LevelWad.layoutClasses[1].GetMobyIndexFromClass(230);
			for (int i = 0; i < indices.size(); i++) {
				tag = (int*)(LevelWad.layoutClasses[1].GetTagPointer(indices[i]));
				Moby* linkedMoby = LevelWad.layoutClasses[1].mobys + tag[10];
				//cout << "Class " << linkedMoby->mobyClass << endl;
				linkedMoby->state = 0x80;

				// The above didn't really work, so just delete them
				LevelWad.layoutClasses[1].DeleteMoby(indices[i]);
			}

			// Doug 317 1 0/1
			MoveDialogueStringBackwards(&LevelWad, 0, 317, 1, 1, 16);
			string msg = "Aw, geez. I finally caught a break! I've been stuck in here for 2 weeks for Cripes sake! Believe you me, that's the last time I go on a date with a figure skater.";
			UpdateDialogue(&LevelWad, 0, 317, 1, 0, msg);
			msg = "Ya, hey dere, Spyro. Ya know, I better head up Nort and hide out for a while.";
			UpdateDialogue(&LevelWad, 0, 317, 1, 1, msg);

			// Moneybags
			MoveDialogueStringForwards(&LevelWad, 0, 198, 3, 1, 52);
			msg = "For some reason, Nancy is still angry with me for something I \"accidentally\" financed years ago. When my good friend, Tonya, said \"clubbing\", I thought she wanted to dance! Anyway, I promised her I'd keep this door shut to make amends. &[Love is priceless.";
			UpdateDialogue(&LevelWad, 0, 198, 3, 3, msg);
			msg = "Thanks for understanding, Spyro. I wouldn't want to get in the way of young love.";
			UpdateDialogue(&LevelWad, 0, 198, 3, 2, msg);
			msg = "Well, when you put it that way... in you go!";
			UpdateDialogue(&LevelWad, 0, 198, 3, 1, msg);
			msg = "Nancy asked me to keep this door shut so she and Doug could have some quality time together. I suppose it's better than just leaving a sock on the door heh heh. Oh, to be young again... Sorry, Spyro. I can't let you in. &Reply with what? &I'll give you 100 gems. &Understood.";
			UpdateDialogue(&LevelWad, 0, 198, 3, 0, msg);
			
			// Nancy
			MoveDialogueStringForwards(&LevelWad, 2, 333, 2, 5, 60);
			MoveDialogueStringForwards(&LevelWad, 2, 333, 2, 4, 220);
			MoveDialogueStringForwards(&LevelWad, 2, 333, 2, 3, 304);
			MoveDialogueStringForwards(&LevelWad, 2, 333, 2, 2, 212);
			MoveDialogueStringForwards(&LevelWad, 2, 333, 2, 1, 212);

			MoveDialogueStringBackwards(&LevelWad, 2, 333, 2, 7, 48);
			MoveDialogueStringBackwards(&LevelWad, 2, 333, 2, 8, 48);

			msg = "OK, you listen to me. If I step outside this ice rink, and Doug isn't there waiting for me with a bouquet of roses and a box of chocolates, you had better start praying. Do you see these ice skates? They're pretty sharp, y'know... and I'm not afraid to use them. &Run away? &Sorry, I wasn't listening. &Uhh... I'd better go.";
			UpdateDialogue(&LevelWad, 2, 333, 2, 0, msg);
			UpdateDialogue(&LevelWad, 2, 333, 2, 1, ".");
			msg = "Oh, dragon, you're not going anywhere. If my Dougie's not here anymore, then you will take his place. If you dare walk into that portal, then I will make you regret it. Believe me, I will find you and I will bring you back. Don't you dare leave me...";
			UpdateDialogue(&LevelWad, 2, 333, 2, 2, msg);
			UpdateDialogue(&LevelWad, 2, 333, 2, 3, ".");
			UpdateDialogue(&LevelWad, 2, 333, 2, 4, ".");
			UpdateDialogue(&LevelWad, 2, 333, 2, 5, ".");
			UpdateDialogue(&LevelWad, 2, 333, 2, 6, "Hmph.");
			UpdateDialogue(&LevelWad, 2, 333, 2, 7, ".");
			msg = "How did you get in here? You're not my Dougie... That bear opened the door for you... What do I even pay him for? And you! You made Doug leave, didn't you!? He. Wanted. To. Stay. How hard is that for you to understand!? How could you do this to me, dragon? You ruined everything!";
			UpdateDialogue(&LevelWad, 2, 333, 2, 8, msg);

			UpdateMessageOffset(&LevelWad, 2, 333, 2, 1, 0);

			UpdateSkybox("resources/21.sky", LevelData.skyboxPointer);
			UpdateLighting("resources/21.lighting", LevelData.block1Pointer);
		}
		else if (levelNo == 11) {
			/*TODO - First of all, I would like to completely remove the possibility of using Sgt. Byrd in this level.
			I think this can be achieved simply by freezing the surface that blocks the area portal that's there until Sgt. Byrd's level is completed.
			I might want to keep skateboarding blocked as well.
			[hwd - I could probably just kill the portal mobys]*/

			LevelWad.layoutClasses[0].DuplicateMoby(202, 0); // add one
			vector<Vector3D> cratePositions = {
				{ 73329, 24714, 39999 }, // old 68579, 26201, 35489
				{ 68526, 26193, 34844 }, // old 67343, 22565, 37629
				{ 67363, 25153, 35803 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(202, cratePositions);

			// Move lava frogs
			// Positions
			vector<PosInfo> lavaFrogPositions = {
				{ { 61225, 25312, 16611 }, { 0, 0, 0x71 } },
				{ { 76812, 20777, 22606 }, { 0, 0, 0xAD } },
				{ { 70506, 23427, 24024 }, { 0, 0, 0x00 } },
				{ { 68337, 28422, 26507 }, { 0, 0, 0x19 } },
				{ { 78944, 25758, 30386 }, { 0, 0, 0xC0 } },
				{ { 73356, 21795, 39977 }, { 0, 0, 0x98 } },
				{ { 70650, 26679, 39990 }, { 0, 0, 0xEE } },
				{ { 89484, 52443, 24554 }, { 0, 0, 0xC0 } }
			};
			// Bone targets
			vector<int> frogs = LevelWad.layoutClasses[0].GetMobyIndexFromClass(571);
			for (int i = 0; i < frogs.size(); i++) {

				/*
				cout << "Frog " << i << ": " << LevelWad.layoutClasses[0].mobys[frogs[i]].position.x << ", ";
				cout << LevelWad.layoutClasses[0].mobys[frogs[i]].position.y << ", ";
				cout << LevelWad.layoutClasses[0].mobys[frogs[i]].position.z << "; ";
				cout << "rot " << (int)LevelWad.layoutClasses[0].mobys[frogs[i]].angle.z << " -> ";
				cout << lavaFrogPositions[i].pos.x << ", ";
				cout << lavaFrogPositions[i].pos.y << ", ";
				cout << lavaFrogPositions[i].pos.z << endl;
				*/

				int* tag = (int*)LevelWad.layoutClasses[0].GetTagPointer(frogs[i]);
				//cout << tag[0] << " " << tag[1] << " " << tag[2] << " " << tag[3] << " " << tag[4] << " " << tag[5] << " " << tag[6] << " " << tag[7] << endl;
				int* linked = (int*)(LevelWad.layoutClasses[0].layoutData + tag[3] + 0xC);
				linked = (int*)(LevelWad.layoutClasses[0].layoutData + *linked);
				//cout << "Linked pos: " << linked[0] << ", " << linked[1] << ", " << linked[2] << endl;

				Vector3D* linkedPos = (Vector3D*)linked;
				linkedPos->x = lavaFrogPositions[i].pos.x + (int)(2000 * cos((3.142 * lavaFrogPositions[i].angle.z) / 128));
				linkedPos->y = lavaFrogPositions[i].pos.y + (int)(2000 * sin((3.142 * lavaFrogPositions[i].angle.z) / 128));
				linkedPos->z = lavaFrogPositions[i].pos.z;

			}
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(571, lavaFrogPositions);

			// Experimental - move a rocket (204) to [some positions] // OLD { 72069, 58268, 20882 } and another rocket to { 44637, 72287, 21561 }, 0x02 for testing
			vector<PosInfo> rocketPositions = {
				{ {  45322, 72448, 21161 }, { 0, 0, 0xBB } }, // 0x02 previously, but rotation doesn't actually seem to affect it anyway
				{ {  39152, 39985, 21465 }, { 0, 0, 0x19 } }
			};
			if (!explorationMode) {
				rocketPositions.push_back({ { 144960, 58261, 20501 }, { 0, 0, 0x78 } });
			}
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(204, rocketPositions);

			// EXPERIMENTAL - final rocket render distance
			int rocketIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(204)[2];
			LevelWad.layoutClasses[0].mobys[rocketIndex].lowDrawDistance *= 2;
			LevelWad.layoutClasses[0].mobys[rocketIndex].highDrawDistance *= 2;

			// Move Sgt. Byrd (89) to { 40169, 46004, 23578 }, (char)0xF8 // OLD { 74908, 23536, 40184 }, (char)0x80
			int byrdIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(89)[0];
			LevelWad.layoutClasses[0].UpdateMobyPosition(byrdIndex, { { 40169, 46004, 23578 }, { 0, 0, 0xF8 } });

			// Headbash crate
			LevelWad.layoutClasses[0].DuplicateMoby(202, 0, { { 65718, 49040, 18327 }, { 0, 0, 0x00 } });

			// Whirlwind
			LevelWad.layoutClasses[0].DuplicateMoby(1023, 0, { { 65718, 49040, 18327 }, { 0, 0, 0xCA } });
			int* whirlwindTag = (int*)(LevelWad.layoutClasses[0].GetTagPointer(LevelWad.layoutClasses[0].mobyBlock->count - 1));
			whirlwindTag[0] = 32673; // height
			whirlwindTag[2] = 5; // type
			whirlwindTag[5] = LevelWad.layoutClasses[0].mobyBlock->count - 2; // linked to new headbash crate

			// Hunter
			int index = LevelWad.layoutClasses[1].GetMobyIndexFromClass(560)[0];
			LevelWad.layoutClasses[1].DeleteMoby(index);
			// also delete 728?
			// also delete 544?

			// Sublevel exits
			PosInfo position = { { 43108, 72384, 21237 }, { 0, 0, 0 } };
			LevelWad.UpdateSublevelExit(2, 0, position);
			position = { { 75785, 28371, 24024 }, { 0, 0, 0x38 } };
			LevelWad.UpdateSublevelExit(1, 0, position);

			// Cpt. Byrd
			UpdateName(&LevelWad, 0, 89, 7, "Cpt. Byrd");

			// This one isn't based on the level itself but it should be smaller than it
			UpdateSkybox("resources/22.sky", LevelData.skyboxPointer);
			UpdateLighting("resources/22.lighting", LevelData.block1Pointer);
			// May want to figure out how to change the sublevel lighting here too
		}
		else if (levelNo == 12) {

			// EOL NPC
			Vector3D position = { 68919, 41619, 14778 };
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(20, 3, position);

			// Moneybags
			PosInfo posinfo = { { 62880, 74204, 7895 }, { 0, 0, 0x75 } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(180, { posinfo });

			// Move chest
			posinfo = { { 38789, 86971, 7896 }, { 0, 0, 0xE0 } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(205, { posinfo });

			// Move key
			posinfo = { { 87243, 35731, 6840 }, { 0, 0, 0x15 } }; // moved up by 200
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(212, { posinfo });

			// Piranha Signs
			vector<PosInfo> signPositions = {
				{ { 57940, 48806, 15320 }, { 0, 0, 0 } },
				{ { 54818, 45596, 15317 }, { 0, 0, 0 } },
				{ { 51078, 45371, 15319 }, { 0, 0, 0 } },
				{ { 47755, 48702, 15317 }, { 0, 0, 0 } },
				{ { 47597, 52422, 15321 }, { 0, 0, 0 } },
				{ { 50916, 55779, 15319 }, { 0, 0, 0 } },
				{ { 54552, 55822, 15321 }, { 0, 0, 0 } },
				{ { 58072, 52460, 15321 }, { 0, 0, 0 } }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(129, signPositions);

			// Sheila sublevel add crates // moving the previous ones instead
			/*
			int index = LevelWad.layoutClasses[2].GetMobyIndexFromClass(202)[0];
			Moby crate = LevelWad.layoutClasses[2].GetMoby(index);
			int tagLen = LevelWad.layoutClasses[2].GetTagLength(index);
			vector<char> tag = LevelWad.layoutClasses[2].GetMobyTag(index, tagLen);
			LevelWad.layoutClasses[2].AddMoby(crate, tag);
			LevelWad.layoutClasses[2].AddMoby(crate, tag);
			LevelWad.layoutClasses[2].AddMoby(crate, tag);
			*/
			
			// Sheila Sublevel things to move
			vector<Vector3D> cratePos = {
				// { 101023,  98021,  9686 }, // deprecated
				// { 105695,  97148,  9686 }, // deprecated
				// { 109973,  96220,  9686 }, // deprecated
				{ 109739, 126794, 12100 },
				{ 115046, 111903, 12150 },
				{ 117130,  98479, 10200 }
			};
			vector<Vector3D> vasePos = { { 99381, 98225, 4392 } };
			vector<Vector3D> basketPos = {
				{ 97101, 118873, 3861 },
				{ 96638, 135050, 4374 },
				{ 97013, 118864, 3861 },
				{ 93196, 104410, 4392 }
			};
			//LevelWad.layoutClasses[2].UpdateMobyPositionsByClass(202, cratePos); // deprecated
			LevelWad.layoutClasses[2].UpdateMobyPositionsByClass(201, vasePos);
			LevelWad.layoutClasses[2].UpdateMobyPositionsByClass(200, basketPos);

			// Add mushroom
			int index = LevelWad.layoutClasses[2].GetMobyIndexFromClass(157)[0];
			Moby mushroom = LevelWad.layoutClasses[2].GetMoby(index);
			int tagLen = LevelWad.layoutClasses[2].GetTagLength(index);
			vector<char> tag = LevelWad.layoutClasses[2].GetMobyTag(index, tagLen);
			mushroom.lowDrawDistance = 0;
			mushroom.highDrawDistance = 0;
			basketPos = { // reusing this variable
				{ 101045, 98114, 7108 },
				{ 102123, 97893, 7108 },
				{ 103506, 97610, 7108 },
				{ 104889, 97326, 7108 },
				{ 106272, 97043, 7108 },
				{ 107655, 96760, 7108 },
				{ 109038, 96476, 7108 },
				{ 110421, 96193, 7108 },
				{ 111804, 95910, 7108 }
			};
			for (int i = 0; i < basketPos.size(); i++) {
				mushroom.position = basketPos[i];
				LevelWad.layoutClasses[2].AddMoby(mushroom, tag);
			}

			// Update state of Sheila eggs, now in Sleepyhead area (1) and main area (0)
			vector<int> indices = LevelWad.layoutClasses[1].GetMobyIndexFromClass(132);
			for (int i = 0; i < indices.size(); i++) {
				LevelWad.layoutClasses[1].mobys[indices[i]].state = 0;
			}
			indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(132);
			for (int i = 0; i < indices.size(); i++) {
				LevelWad.layoutClasses[0].mobys[indices[i]].state = 0;
			}

			// Gems
			indices = LevelWad.layoutClasses[2].GetMobyIndexFromClass(1);
			basketPos = { // reusing this variable
				{ 83280, 106710, 4348 }, // 1
				{ 80214, 106552, 5072 }, // 2
				{ 76772, 106733, 10184 } // 5
			};
			LevelWad.layoutClasses[2].UpdateMobyPositionsByClass(1, basketPos);
			LevelWad.layoutClasses[2].mobys[indices[0]].levelSectorIndex = 1;
			LevelWad.layoutClasses[2].mobys[indices[1]].levelSectorIndex = 2;
			LevelWad.layoutClasses[2].mobys[indices[2]].levelSectorIndex = 5;

			// Moneybags - 180 2 n
			string msg = "\\              I have a secret.\\       You won't know unless you pay.\\           On sale, just for you. &Pay ^^^ gems for a secret? &Sure, just shut up. &No, your haiku poetry stinks.";
			UpdateDialogue(&LevelWad, 0, 180, 2, 0, msg);
			msg = "\\          Mushroom with a vase.\\    High-jump toward the egg you see.\\             Invisible path.\\           Also, so you know:\\    Key chests might have eggs inside.\\        Turn the key and charge.";
			UpdateDialogue(&LevelWad, 0, 180, 2, 1, msg);
			msg = "\\         I wish I could help.\\       I know of a secret path.\\      Come back with more cash. &[^^^ gems needed for a secret";
			UpdateDialogue(&LevelWad, 0, 180, 2, 3, msg);

			// Bubba the Firefly - Class 20, Type 5
			UpdateName(&LevelWad, 0, 20, 5, "Bart the Firefly");
			msg = "\\             Ay, caramba, man!\\       Bart Simpson, at your service.\\           Who the hell are you?";
			UpdateDialogue(&LevelWad, 0, 20, 5, 0, msg);
			msg = "\\       I will not show off.\\ I will not scream for ice cream.\\      I will not waste chalk.";
			UpdateDialogue(&LevelWad, 0, 20, 5, 1, msg);
			msg = "Eat my shorts!";
			UpdateDialogue(&LevelWad, 0, 20, 5, 2, msg);
			UpdateDialogue(&LevelWad, 0, 20, 5, 3, msg);

			UpdateSkybox("resources/23.sky", LevelData.skyboxPointer);
			UpdateLighting("resources/23.lighting", LevelData.block1Pointer);
		}
		else if (levelNo == 13) { // Bamboo Terrace
			// EOL NPC
			PosInfo position = { { 115690, 38081, 21493 }, { 0, 0, 0xC1 } }; // formerly { { 118123, 38268, 24757 }, { 0, 0, 0xC0 } };
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(11, 2, position);

			// Ling Ling
			position = { { 122009, 21997, 19417 }, { 0, 0, 0xC8 } };
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(11, 3, position);

			// Another NPC move
			position = { { 41416, 85456, 18390 }, { 0, 0, 0x2C } };
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(11, 7, position);

			// Baskets
			vector<Vector3D> positions = {
				{ 84751, 69580, 21209 },
				{ 92929, 73614, 21209 },
				{ 96658, 60553, 20182 },
				{ 93274, 58862, 20182 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(200, positions);

			// Disable thief
			vector<int> thiefIndices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(582);
			if (thiefIndices.empty()) {
				printf("ERROR - Bamboo Terrace - no thief found.\n");
			}
			else {
				LevelWad.layoutClasses[0].DeleteMoby(thiefIndices[0]);
			}

			// Area 2 entrance? EXPERIMENTAL
			// position = { { 176312, 59249, 12247 }, { 0, 0, 0x40 } };
			// LevelWad.UpdateSublevelExit(0, 2, position);
			// This sets BENTLEY'S spawn point when you start the actual sublevel - the initial spawn point (Bentley's room) is somewhere else!

			// Delete Bentley - check that Spyro spawns in the desired spot
			int bentleyIndex = LevelWad.layoutClasses[2].GetMobyIndexFromClass(15)[0];
			LevelWad.layoutClasses[2].DeleteMoby(bentleyIndex);

			// Delete area 1 NPC
			vector<int> npcIndices = LevelWad.layoutClasses[1].GetMobyIndexFromClass(61); // keeping this for now, unless told otherwise
			if (npcIndices.empty()) {
				printf("ERROR - Bamboo Terrace - no NPC (61) found.\n");
			}
			else {
				LevelWad.layoutClasses[1].DeleteMoby(npcIndices[0]);
			}
			int index = LevelWad.layoutClasses[1].GetMobyIndexFromNpcType(61, 1);
			if (index == -1) {
				printf("ERROR - Bamboo Terrace - no NPC (61, type 1) found.\n");
			}
			else {
				LevelWad.layoutClasses[1].DeleteMoby(index);
			}

			UpdateSkybox("resources/24.sky", LevelData.skyboxPointer);
			UpdateVram("resources/24.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/24.texture", LevelData.block0Pointer);
			UpdateLighting("resources/24.lighting", LevelData.block1Pointer);

			for (int i = 0; i < 3; i++) {
				auto Scene = SceneData(i, (unsigned char*)LevelWad.sceneDataBlocks[i], LevelWad.sceneHeaders[i].length);
				Scene.UpdateVram("resources/24.vram"); // not ideal, some area 1 and 2 textures are wrong
				LevelWad.UpdateScene(i);
			}

		}
		else if (levelNo == 14) {

			// Rings (329)
			vector<PosInfo> positions = {
				{ { 108453,  59405, 22215 }, { 0,    0, 0x25 } },
				{ {  82483,  97601, 14100 }, { 0, 0x40,    0 } },
				{ {  65256, 100785, 10363 }, { 0,    0, 0x52 } },
				{ {  77512, 125655, 16344 }, { 0,    0, 0x04 } },
				{ {  85149, 126559, 16344 }, { 0,    0, 0x03 } },
				{ {  59907, 127893, 23151 }, { 0,    0, 0x40 } },
				{ {  72397, 140305, 23000 }, { 0, 0x40,    0 } },
				{ { 121283,  75607, 18169 }, { 0,    0, 0x24 } }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(329, positions);

			// Boost Star
			int starIndex = 1; // this one seems to work, but, just in case there's a difference between versions, check
			vector<int> stars = LevelWad.layoutClasses[0].GetMobyIndexFromClass(338);
			//cout << stars.size() << " stars." << endl;
			for (int i = 0; i < stars.size(); i++) {
				int* tag = (int*)(LevelWad.layoutClasses[0].GetTagPointer(stars[i]));
				if (tag[1] == 0) {
					starIndex = i;
					break;
				}
			}
			LevelWad.layoutClasses[0].DuplicateMoby(338, starIndex, { { 108453, 59405, 22215 }, { 0, 0, 0x25 } });
			// specifically, we want to copy one intended for the time trial mode, not the race, and I don't want to worry about version differences in moby ordering here

			// Change render distance of a couple of the eggs
			int index = LevelWad.layoutClasses[0].GetEggIndexByNumber(1);
			LevelWad.layoutClasses[0].mobys[index].lowDrawDistance = 4;
			LevelWad.layoutClasses[0].mobys[index].highDrawDistance = 4;

			index = LevelWad.layoutClasses[0].GetEggIndexByNumber(2);
			LevelWad.layoutClasses[0].mobys[index].lowDrawDistance = 0;
			LevelWad.layoutClasses[0].mobys[index].highDrawDistance = 0;

			// Sparx - change message 2 to point at message 0 instead (experimental)
			int sparxIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(144)[0]; // Speedway Sparx is 144
			char* sparxTag = LevelWad.layoutClasses[0].GetTagPointer(sparxIndex);
			int* dialoguePointers = (int*)(sparxTag + 16); // 0xC is name, 0x10 is msg 0
			dialoguePointers[2] = dialoguePointers[0];

			// Sparx - change msg 0 string [old implementation :zany: probably shouldn't mix the new implementation with the offset updates]
			char* messagePointer = LevelWad.layoutClasses[0].layoutData + dialoguePointers[0];
			messagePointer += *messagePointer; // Pass over dialogue header
			const char* newMessage = "Which challenge would you like to play? &Choose a challenge: &Time Attack &...What? &Quit"; // Shorter than original
			memcpy(messagePointer, (char*)newMessage, strlen(newMessage));
			*(messagePointer + strlen(newMessage)) = 0; // Null-terminate

			// Hunter - disable - deprecated
			//int hunterIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(653)[0];
			//LevelWad.layoutClasses[0].DeleteMoby(hunterIndex); // This is crude - maybe I just need to put him in an uninteractable state instead

			// Hunter
			MoveDialogueStringBackwards(&LevelWad, 0, 653, 1, 1, 76);
			MoveDialogueStringForwards(&LevelWad, 0, 653, 1, 3, 100);
			MoveDialogueStringForwards(&LevelWad, 0, 653, 1, 2, 112);

			string msg = "Okay, this is embarrassing, but I lost your Christmas present, Spyro.";
			UpdateDialogue(&LevelWad, 0, 653, 1, 0, msg);
			msg = "There I was - dressed like Santa Claws on the roof with a bag of awesome presents. Yours was definitely in there. But, by the time I got down here, it was gone. I looked everywhere! I have no idea where it could be. Let me know if you find it, OK? &Help Hunter out? &Huh? &OK, I'll let you know.";
			UpdateDialogue(&LevelWad, 0, 653, 1, 1, msg);
			UpdateDialogue(&LevelWad, 0, 653, 1, 2, ".");
			UpdateDialogue(&LevelWad, 0, 653, 1, 3, "OK.");

			UpdateMessageOffset(&LevelWad, 0, 653, 1, 2, 1);

		}
		else if (levelNo == 15) {
			
			// Change spawn point
			if (!explorationMode) { // standard spawn in exploration
				LayoutHeader* header = (LayoutHeader*)LevelWad.layoutClasses[0].layoutData;
				header->spawnPosition = { 45158, 43991, 10627 };
				header->spawnRotation = { 0, 0, 0x4E };
			}

			// Disable portal?
			int index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(991)[0];
			BoundingBox* tag = (BoundingBox*)(LevelWad.layoutClasses[0].layoutData + LevelWad.layoutClasses[0].mobys[index].mobyTag);
			tag->pos = { 0, 0, 0 };
			index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(3)[0];
			LevelWad.layoutClasses[0].DeleteMoby(index);
			// Vector3D pos = LevelWad.layoutClasses[0].mobys[index].position;
			// int portalIndex = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(991, pos);
			// LevelWad.layoutClasses[0].DeleteMoby(portalIndex);

			// Balloon
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(208);
			vector<Vector3D> positions = {
				{ 39839, 48256, 8298 },
				{ 42906, 51421, 8298 },
				{ 47198, 51419, 8298 }
			};
			for (int i = 0; i < 3; i++) {
				LevelWad.layoutClasses[0].UpdateMobyPosition(indices[i], positions[i]);
				LevelWad.layoutClasses[0].mobys[indices[i]].state = 3;
			}

			// Byrd
			PosInfo posInfo = { { 50735, 70388, 14322 }, { 0, 0, 0xD4 } };
			if (!explorationMode) {
				LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(89, { posInfo });
			}

			// Weight
			Vector3D  pos = { 50565, 74322, 18434 };
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(248, pos);
			pos = { 52211, 33167, 6103 };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, pos);

			// Bombs
			pos = { 58415, 77036, 18418 };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(104, { pos });

			// Life jar
			pos = { 15314, 10629, 6088 };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(563, { pos });

			// Hummingbird cages - cages 55, hummingbirds 167
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(55, { 11653, 9482, 6646 });
			posInfo = { { 15802, 39431, 18390 }, { 0, 0, 0x44 } }; // at one point was 0xC4
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posInfo);
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(55, { 53893, 76093, 13312 });
			LevelWad.layoutClasses[0].mobys[index].angle.z = 0x05; // at one point was 0x85

			// Gems
			positions = {
				{ 42710, 41183, 28207 },
				{ 39838, 44501, 28208 },
				{ 40108, 48306, 28207 },
				{ 43372, 51281, 28199 },
				{ 49155, 34026, 28224 },
				{ 49122, 62632, 28217 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);

			// EOL enemy
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(161, { 27320, 52572, 8192 });
			//cout << "DEBUG - REMOVE LATER - EOL Enemy at ";
			//cout << LevelWad.layoutClasses[0].mobys[index].position.x << ", ";
			//cout << LevelWad.layoutClasses[0].mobys[index].position.y << ", ";
			//cout << LevelWad.layoutClasses[0].mobys[index].position.z << endl;
			posInfo = { { 13516, 56064, 18392 }, { 0, 0, 0xE1 } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posInfo);

			// Move Sgt. Byrd spawn point (both versions)
			index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(89)[0];
			int* byrdTag = (int*)(LevelWad.layoutClasses[0].layoutData + LevelWad.layoutClasses[0].mobys[index].mobyTag);
			index = byrdTag[22];
			posInfo = { { 67361, 61916, 20145 }, { 0, 0, 0x6F } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posInfo);
			// This won't cause warping issues because we've disabled this portal! :)

			// Cpt. Byrd
			UpdateName(&LevelWad, 0, 89, 10, "Cpt. Byrd");

			// Gabrielle
			string msg = "You need to find the Captain! He's the only one who can liberate the rhynoc prisoners and retake our stronghold. Try using the balloons behind me to get to the upper hallway. Charge jump toward one, then press < while still charging right as you enter it for a bounce.";
			UpdateDialogue(&LevelWad, 0, 167, 0, 0, msg);
			msg = "You just learned the 'Dead Body Proxy' technique. Try it in other worlds too.";
			UpdateDialogue(&LevelWad, 0, 167, 0, 1, msg);
			msg = "One carefully placed flame should take this door down, private.";
			UpdateDialogue(&LevelWad, 0, 167, 0, 2, msg);

		}
		else if (levelNo == 16) { // Spike's Arena

			// Cpt. Byrd
			UpdateName(&LevelWad, 0, 536, 0, "Cpt. Byrd");

			UpdateSkybox("resources/27.sky", LevelData.skyboxPointer);
			UpdateVram("resources/27.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/27.texture", LevelData.block0Pointer);

		}
		else if (levelNo == 17) { // Spider Town
			
			// Delete Sparx
			int index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(368)[0];
			LevelWad.layoutClasses[0].DeleteMoby(index);

			// Gems (1, becomes 381 in-game?)
			vector<Vector3D> positions = {
				{ 82367, 61798, 9175 },
				{ 77973, 61798, 9175 },
				{ 70561, 73166, 8149 },
				{ 67861, 72698, 9175 },
				{ 57016, 72647, 9175 },
				{ 57002, 76786, 9174 },
				{ 57045, 80899, 9175 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);

			// Key
			Vector3D position = { 112845, 38707, 4403 };
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(339, position);
			position = { 95529, 57140, 4403 };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, position);

			// Spawner
			position = { 61293, 64489, 4054 };
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(644, position);
			PosInfo posinfo = { { 52300, 71930, 8149 }, { 0, 0, 0x40 } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posinfo);

			// Fire-breathing spiders
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(600);
			for (int i = 0; i < indices.size(); i++) {
				LevelWad.layoutClasses[0].DeleteMoby(indices[i]);
			}

			// Special Gems
			AddSpecialGem(&LevelWad, 0, { 79900, 24595, 4056 }, false);
			AddSpecialGem(&LevelWad, 0, { 110694, 36960, 4056 }, false);
			AddSpecialGem(&LevelWad, 0, { 110167, 49383, 4056 }, false);
			AddSpecialGem(&LevelWad, 0, { 25715, 76823, 4055 }, true);

		}
		else if (levelNo == 18) { // Evening Lake

			// Egg reqs
			vector<int> portalMobys = LevelWad.layoutClasses[0].GetMobyIndexFromClass(988);
			for (int i = 0; i < portalMobys.size(); i++) {
				Moby988Tag* portalTag = (Moby988Tag*)(LevelWad.layoutClasses[0].GetTagPointer(portalMobys[i]));
				if (portalTag->LevelId == 33) {
					portalTag->EggReq = (explorationMode ? 1 : 40);
				}
				else if (portalTag->LevelId == 34) {
					portalTag->EggReq = (explorationMode ? 1 : 47);
				}
				else if (portalTag->LevelId == 35) {
					portalTag->EggReq = (explorationMode ? 1 : 52); // because of bentley, etc.
				}
			}

			// Stooby - Dragon 2
			int index = LevelWad.layoutClasses[0].GetEggIndexByNumber(2);
			if (index != -1) {
				LevelWad.layoutClasses[0].mobys[index].lowDrawDistance = 0;
			}
			else {
				printf("ERROR - Evening Lake - Unable to update Stooby.\n");
			}

			// EXPERIMENTAL - dragons 0 and 4 - attempt to increase their render distances
			// If this works then this section of code could be cleared up by just getting all the dragons and working on them on a case by case basis
			index = LevelWad.layoutClasses[0].GetEggIndexByNumber(0);
			if (index != -1) {
				LevelWad.layoutClasses[0].mobys[index].lowDrawDistance *= 3;
				LevelWad.layoutClasses[0].mobys[index].highDrawDistance *= 3;
				//LevelWad.layoutClasses[0].mobys[index].levelSectorIndex = 255;
			}
			else {
				printf("ERROR - Evening Lake - Unable to update Ted.\n");
			}
			index = LevelWad.layoutClasses[0].GetEggIndexByNumber(4);
			if (index != -1) {
				LevelWad.layoutClasses[0].mobys[index].lowDrawDistance *= 3;
				LevelWad.layoutClasses[0].mobys[index].highDrawDistance *= 3;
				//LevelWad.layoutClasses[0].mobys[index].levelSectorIndex = 255;
			}
			else {
				printf("ERROR - Evening Lake - Unable to update Stuart.\n");
			}

			// Switch Bentley and Honey portal destinations
			PortalInfo* portals = (PortalInfo*)(LevelWad.levelDataBlock + LevelData.originalOffsets[7] + 8);
			int portalCount = LevelData.portalCount;
			for (int i = 0; i < portalCount; i++) {
				if (portals[i].levelId == 35) {
					portals[i].levelId = 36;
				}
				else if (portals[i].levelId == 36) {
					portals[i].levelId = 35;
				}
			}
			// Would be funny to switch the loading screen transitions too so that you think you're going into the right one
			// This has been done in UpdateExe

			// Zoe
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(272, { 57651, 62290, 35328 });
			PosInfo posInfo = { { 49787, 46917, 34000 }, { 0, 0, 0x35 } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posInfo);

			// Bianca
			string msg = "I can feel the Sorceress's magic weakening with every dragon you save, but she still has much more power than me. You need to hurry, Spyro! If she completes her immortality ritual, all hope will be lost. She used an invisibility enchantment on some of the eggs here. I've written clues in your Atlas that might help you find them.";
			UpdateDialogue(&LevelWad, 0, 425, 20, 0, msg);

			// Zoe
			msg = "Tell Sparx to meet me in Crawdad Farm on top of the wall by the boss room. He can use the boss's attack to boost himself up and then climb the wall with his strafe.";
			UpdateDialogue(&LevelWad, 0, 272, 12, 0, msg);

			// Moneybags 198 0 n
			MoveDialogueStringBackwards(&LevelWad, 0, 198, 0, 3, 16);
			msg = "Hello, Spyro! Do you want to play a game? Here's how it works: if you give me gems, you win! Sounds like fun, right? &Pay ^^^ gems to play? &OK. &No deal, dirtbag.";
			UpdateDialogue(&LevelWad, 0, 198, 0, 0, msg);
			msg = "Yes?? Ha! Even I didn't think that was going to work! Dragons really are so gullible.";
			UpdateDialogue(&LevelWad, 0, 198, 0, 1, msg);
			msg = "Unlucky, Spyro. You just lost the game.";
			UpdateDialogue(&LevelWad, 0, 198, 0, 2, msg);
			msg = "Don't insult me, Spyro. You need more gems than that if you want to play my little game. Come back when your pockets are a little deeper and see if you can beat me. &[^^^ gems needed to play.";
			UpdateDialogue(&LevelWad, 0, 198, 0, 3, msg);

			UpdateSkybox("resources/30.sky", LevelData.skyboxPointer);
			UpdateLighting("resources/30.lighting", LevelData.block1Pointer);

		}
		else if (levelNo == 19) { // Frozen Altars

			// Get NPC positions
			PosInfo eugenePos; // 119 1
			PosInfo ernestPos; // 119 3
			PosInfo eustacePos; // 119 4
			
			int index = LevelWad.layoutClasses[0].GetMobyIndexFromNpcType(119, 1);
			eugenePos = { LevelWad.layoutClasses[0].mobys[index].position, LevelWad.layoutClasses[0].mobys[index].angle };
			index = LevelWad.layoutClasses[0].GetMobyIndexFromNpcType(119, 3);
			ernestPos = { LevelWad.layoutClasses[0].mobys[index].position, LevelWad.layoutClasses[0].mobys[index].angle };
			index = LevelWad.layoutClasses[0].GetMobyIndexFromNpcType(119, 4);
			eustacePos = { LevelWad.layoutClasses[0].mobys[index].position, LevelWad.layoutClasses[0].mobys[index].angle };

			// Move EOL NPC (119,1)
			PosInfo position = { { 90687, 90198, 17978 }, { 0, 0, 0xE0 } };
			if (!explorationMode) { // only in challenge mode
				LevelWad.layoutClasses[0].UpdateNpcPositionByType(119, 1, position);
			}

			// Move Ernest (3) to Eugene (1) original position
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(119, 3, eugenePos);
			if (explorationMode) { // moves in cave, i.e. Eustace and Ernest just switch
				LevelWad.layoutClasses[0].UpdateNpcPositionByType(119, 3, eustacePos);
			}

			// Move Eustace (4) to Ernest (3) original position
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(119, 4, ernestPos);

			// Moneybags
			position = { { 46241, 108169, 28118 }, { 0, 0, 0xE0 } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(198, { position });

			// Gems
			vector<Vector3D> positions = {
				{ 103638,  99269, 33402 }, // 10
				{ 109218, 105453, 31400 }, // 25
				{  94685,  92863, 34870 }  // 5
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);

			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1);
			LevelWad.layoutClasses[0].mobys[indices[0]].levelSectorIndex = 10;
			LevelWad.layoutClasses[0].mobys[indices[1]].levelSectorIndex = 25;
			LevelWad.layoutClasses[0].mobys[indices[2]].levelSectorIndex = 5;

			// Baskets
			positions = {
				{ 89040, 57337, 28630 },
				{ 89034, 53534, 28632 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(200, positions);

			// Move mammoths
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(62, { 71111, 31895, 19813 });
			position = { { 47500, 54000, 28710 }, { 0, 0, 0x40 } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, position);
			int* mobyTag = (int*)(LevelWad.layoutClasses[0].GetTagPointer(index));
			mobyTag = (int*)(LevelWad.layoutClasses[0].layoutData + *mobyTag + 12);
			mobyTag = (int*)(LevelWad.layoutClasses[0].layoutData + *mobyTag);
			*(Vector3D*)(mobyTag + 0) = { position.pos.x, position.pos.y + 5000, position.pos.z };
			*(Vector3D*)(mobyTag + 4) = { position.pos.x, position.pos.y, position.pos.z };
			*(Vector3D*)(mobyTag + 8) = { position.pos.x, position.pos.y - 5000, position.pos.z };

			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(62, { 71111, 31895, 19813 });
			position = { { 51500, 54000, 28710 }, { 0, 0, 0x40 } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, position);
			mobyTag = (int*)(LevelWad.layoutClasses[0].GetTagPointer(index));
			mobyTag = (int*)(LevelWad.layoutClasses[0].layoutData + *mobyTag + 12);
			mobyTag = (int*)(LevelWad.layoutClasses[0].layoutData + *mobyTag);
			*(Vector3D*)(mobyTag + 0) = { position.pos.x, position.pos.y - 5000, position.pos.z };
			*(Vector3D*)(mobyTag + 4) = { position.pos.x, position.pos.y, position.pos.z };
			*(Vector3D*)(mobyTag + 8) = { position.pos.x, position.pos.y + 5000, position.pos.z };

			// Aly (dragon 1) / Melt the snowman - render distance 0
			index = LevelWad.layoutClasses[0].GetEggIndexByNumber(1);
			LevelWad.layoutClasses[0].mobys[index].lowDrawDistance = 0;

			// Bentley
			index = LevelWad.layoutClasses[1].GetMobyIndexFromClass(233)[0];
			LevelWad.layoutClasses[1].DeleteMoby(index);

			// Cat Hockey area NPC
			index = LevelWad.layoutClasses[2].GetMobyIndexFromClass(119)[0];
			LevelWad.layoutClasses[2].DeleteMoby(index);

			// Renaming NPCs
			UpdateName(&LevelWad, 0, 385, 6, "Pepper");

			// Gabrielle
			string msg = "Legend has foretold this moment! In our time of greatest need, a heroic dragon will appear to protect our temple. It's you, right? Spyro?";
			UpdateDialogue(&LevelWad, 0, 385, 6, 0, msg);

			// Ernest
			MoveDialogueStringBackwards(&LevelWad, 0, 119, 3, 1, 32);
			MoveDialogueStringBackwards(&LevelWad, 0, 119, 3, 2, 32);

			msg = "Oh, you thought I had an egg? For the last time, I'm not Eugene. Not all penguins look the same, you know.";
			UpdateDialogue(&LevelWad, 0, 119, 3, 0, msg);
			msg = "I saw Eugene swimming around before the snowmen came. Maybe he got trapped under the ice.";
			UpdateDialogue(&LevelWad, 0, 119, 3, 1, msg);
			msg = "I saw Eugene swimming around before the snowmen came. Maybe he got trapped under the ice.";
			UpdateDialogue(&LevelWad, 0, 119, 3, 2, msg);

			// Alex
			msg = "The temple is under siege by evil snowmen! I can only imagine this is the will of Itztlacoliuhqui. Today, we were meant to honor our gods by sacrificing a sheep at the altar, but rhynocs swarmed our city before we could.";
			UpdateDialogue(&LevelWad, 0, 119, 0, 0, msg);
			msg = "If you happen to find a sheep lying around, let me know.";
			UpdateDialogue(&LevelWad, 0, 119, 0, 1, msg);
			msg = "You're really bothering me now. Stop.";
			UpdateDialogue(&LevelWad, 0, 119, 0, 2, msg);

			// Eustace
			msg = "Can you help me melt this snowman? If we don't act fast, it'll be an ice age!";
			UpdateDialogue(&LevelWad, 0, 119, 4, 0, msg);
			msg = "Why on Earth are you using frost breath here? You realize we're trying to melt things, right?";
			UpdateDialogue(&LevelWad, 0, 119, 4, 1, msg);
			msg = "Less talking, more saving the temple.";
			UpdateDialogue(&LevelWad, 0, 119, 4, 2, msg);

			// Eugene
			msg = "Brrr, I don't know why, but I feel reaaalllly cold.";
			UpdateDialogue(&LevelWad, 0, 119, 1, 0, msg);

			// Moneybags 198 5 n
			MoveDialogueStringBackwards(&LevelWad, 0, 198, 5, 1, 116);

			msg = "Hi, Spyro. I, uh, 'came across' a message that Zoe was trying to send you. It seems pretty urgent. I'd love to read it to you after we discuss my finder's fee. What do you say? &Pay ^^^^ gems for Zoe's message? &Sure. &I'll pass.";
			UpdateDialogue(&LevelWad, 0, 198, 5, 0, msg);
			msg = "Let's see... where'd that letter go? Ah, here it is. Ahem... 'Spyro, I need to talk to you. I discovered something incredible! Come meet me in Frosty Gardens. Swim behind the Icy Peak portal and look for the sparkling from my wand. I'll tell you more there!'";
			UpdateDialogue(&LevelWad, 0, 198, 5, 1, msg);
			msg = "No? What could you possibly mean by that? I think you're underestimating the importance of the message I stole... Err.. 'found'.";
			UpdateDialogue(&LevelWad, 0, 198, 5, 2, msg);
			msg = "Truthfully, Spyro, if you knew exactly what I was offering, you'd pay twice as much as what I'm asking. 2000 gems is a steal! I promise... &[Top secret intel costs ^^^^ gems.";
			UpdateDialogue(&LevelWad, 0, 198, 5, 3, msg);
			msg = "Shoo, dragon.";
			UpdateDialogue(&LevelWad, 0, 198, 5, 4, msg);

			// Sign
			position = { { 44890, 57376, 20953 }, { 0, 0, 0xFF } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(408, { position });
			msg = "Cat hockey match cancelled by PETA.";
			UpdateDialogue(&LevelWad, 0, 408, 130, 0, msg);
			UpdateDialogue(&LevelWad, 0, 408, 130, 1, msg);
			UpdateDialogue(&LevelWad, 0, 408, 130, 2, msg);

			UpdateSkybox("resources/31.sky", LevelData.skyboxPointer);
			UpdateVram("resources/31.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/31.texture", LevelData.block0Pointer);
			UpdateLighting("resources/31.lighting", LevelData.block1Pointer);

			// Frozen Altars has no scene vram
			// So moving models in here wouldn't have been so bad, but we didn't really have time for it
			//auto Scene = SceneData(0, (unsigned char*)LevelWad.sceneDataBlocks[0], LevelWad.sceneHeaders[0].length);
			//Scene.UpdateVram("resources/31.vram");
			//LevelWad.UpdateScene(0);
			// Do other sublevels
			// Possible that area 0 has no scene VRAM anyway

		}
		else if (levelNo == 20) { // Lost Fleet
			/*{ 45363, 125960, 17562 } "In the Skating Challenge."
			TODO - move Hunter here - not sure how this'll work due to the two stages of the challenge?
			either deactivate Hunter or use one of the area 1 eggs up and keep both skate races instead
			Is this still relevant?*/

			// Remove Zoe
			Vector3D pos = { 87556, 90862, 25083 };
			int index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(272, pos);
			LevelWad.layoutClasses[0].DeleteMoby(index);

			// Armoured chest
			pos = { 36686, 53276, 28335 }; // formerly { 36728, 53271, 28333 } // formerly { 36811, 53261, 28330 }, "Blast the armored chest" too
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(203, { pos });

			// Vase - no vases in Lost Fleet :(
			// pos = { 78943, 62942, 37734 };
			// LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(201, { pos });

			// Baskets
			vector<Vector3D> positions = {
				{ 102723, 64534, 35303 },
				{ 100901, 71125, 36199 },
				{  78943, 62942, 37734 } // formerly a vase
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(200, positions);

			// Gems
			positions = {
				{ 106488, 67202, 39178 },
				{ 105186, 67857, 38677 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);

			// Hunter
			index = LevelWad.layoutClasses[2].GetMobyIndexFromClass(103)[0];
			LevelWad.layoutClasses[2].DeleteMoby(index);

			// Crazy Ed
			index = LevelWad.layoutClasses[1].GetMobyIndexFromClass(44)[0];
			LevelWad.layoutClasses[1].DeleteMoby(index);

			string msg = "You've found an Easter egg... not quite as good as a dragon egg, huh? - HWD and JT";
			UpdateDialogue(&LevelWad, 0, 44, 0, 2, msg);

			UpdateSkybox("resources/32.sky", LevelData.skyboxPointer);
			UpdateVram("resources/32.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/32.texture", LevelData.block0Pointer);
			UpdateLighting("resources/32.lighting", LevelData.block1Pointer);

			for (int i = 0; i < 3; i++) {
				auto Scene = SceneData(i, (unsigned char*)LevelWad.sceneDataBlocks[i], LevelWad.sceneHeaders[i].length);
				Scene.UpdateVram("resources/32.vram"); // not ideal, some area 1 and 2 textures are wrong
				LevelWad.UpdateScene(i);
			}

		}
		else if (levelNo == 21) { // Fireworks Factory

			// Remove entrance to A9 area, as this is causing crashes
			Vector3D pos = { 99931, 56177, 9000 };
			int index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(3, pos);
			LevelWad.layoutClasses[0].DeleteMoby(index);
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1022);
			for (int i = 0; i < indices.size(); i++) {
				Moby* moby = &(LevelWad.layoutClasses[0].mobys[indices[i]]);
				Moby1022Tag* tag = (Moby1022Tag*)(LevelWad.layoutClasses[0].layoutData + moby->mobyTag);
				if (tag->fromArea == 2 || tag->toArea == 2) {
					LevelWad.layoutClasses[0].DeleteMoby(indices[i]);
				}
			}

			// Move some gems and things
			vector<Vector3D> positions = {
				{ 62242, 102587, 12759 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(563, positions);
			positions = {
				{ 39918, 110207, 17621 },
				{ 31711, 110214, 17621 },
				{ 36857,  94241, 12406 }, // 25
				{ 55334,  94156, 12407 }, // 25
				{ 65517,  63535, 18786 }  // 5
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);
			positions = {
				{ 87280, 70565, 13271 },
				{ 86431, 74411, 13272 }
				// { 81589, 77320, 13270 } deprecated
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(201, positions);
			positions = {
				{ 36656, 68800, 24809 },
				{ 42238, 66153, 24809 },
				{ 46185, 61470, 24809 }
			};
			LevelWad.layoutClasses[1].UpdateMobyPositionsByClass(201, positions); // dragons area
			positions = {
				{ 77855, 79219, 13272 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(203, positions);

			indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1);
			LevelWad.layoutClasses[0].mobys[indices[2]].levelSectorIndex = 25;
			LevelWad.layoutClasses[0].mobys[indices[3]].levelSectorIndex = 25;
			LevelWad.layoutClasses[0].mobys[indices[4]].levelSectorIndex = 5;

			// DO NOT OVERWRITE INDICES AS IT IS BEING USED BY THE WHIRLWIND
			// Whirlwind
			LevelWad.layoutClasses[0].DuplicateMoby(1023, 0, { { 63799, 62738, 10714 }, { 0, 0, 0x01 } });
			int* whirlwindTag = (int*)(LevelWad.layoutClasses[0].GetTagPointer(LevelWad.layoutClasses[0].mobyBlock->count - 1));
			whirlwindTag[0] = 8186; // height
			whirlwindTag[2] = 5; // type
			whirlwindTag[5] = indices[4]; // linked to 5 gem

			// Handel
			index = LevelWad.layoutClasses[1].GetMobyIndexFromClass(449)[0];
			LevelWad.layoutClasses[1].DeleteMoby(index);

			// Handel
			MoveDialogueStringBackwards(&LevelWad, 3, 449, 3, 1, 8);
			string msg = "I think I saw an egg in the twin dwagon sublevel.";
			UpdateDialogue(&LevelWad, 3, 449, 3, 0, msg);
			UpdateDialogue(&LevelWad, 3, 449, 3, 1, msg);

			UpdateSkybox("resources/33.sky", LevelData.skyboxPointer);
			UpdateVram("resources/33.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/33.texture", LevelData.block0Pointer);
			UpdateLighting("resources/33.lighting", LevelData.block1Pointer);

			for (int i = 0; i < 3; i++) {
				
				if (i == 0) {
					auto Scene = SceneData(i, (unsigned char*)LevelWad.sceneDataBlocks[i], LevelWad.sceneHeaders[i].length);
					Scene.UpdateVram("resources/33.vram");
					LevelWad.UpdateScene(i);

					// TODO - probably should do this for the other sublevels too, otherwise there will be errors

				}
			}

		}
		else if (levelNo == 22) { // Charmed Ridge

			// Move EOL Cat Wizard (366) from around { 107428 , 95300 , 25335 } to { 68551, 52885, 19366 }, 0x2E
			Vector3D position = { 106660, 95119, 24975 };
			int catIndex = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(366, position);
			if ((catIndex != -1) && !explorationMode) {
				//cout << "DEBUG - REMOVE LATER - Cat Wizard at ";
				//cout << LevelWad.layoutClasses[0].mobys[catIndex].position.x << ", ";
				//cout << LevelWad.layoutClasses[0].mobys[catIndex].position.y << ", ";
				//cout << LevelWad.layoutClasses[0].mobys[catIndex].position.z << endl;
				PosInfo newPos = { { 68551, 52885, 19366 }, { 0, 0, 0x2E } };
				LevelWad.layoutClasses[0].UpdateMobyPosition(catIndex, newPos);
				//printf("DEBUG - REMOVE LATER - Cat Wizard moved successfully\n");
			}
			
			// Seeds (area 2, class 545) - move the yellow seed from around { 37989, 49438, 13480 } to { 37472, 72403, 12570 }
			position = { 36357, 48139, 13078 }; // formerly 37989, 49438, 13080
			int index = LevelWad.layoutClasses[2].GetClosestMobyIndexFromClass(545, position);
			position = { 37472, 72403, 12570 };
			LevelWad.layoutClasses[2].UpdateMobyPosition(index, position);

			// Remove the yellow seed from the platform with the bars/headbash crate and move a red seed there
			// (yellow outside bars: 44841, 37192, 16236)
			// Remove the red seed from the platform nearest to the completion platform for the first part of the challenge
			// (red: 28959, 28979, 20332)
			index = LevelWad.layoutClasses[2].GetClosestMobyIndexFromClass(545, { 44841, 37192, 16236 });
			int rIndex = LevelWad.layoutClasses[2].GetClosestMobyIndexFromClass(545, { 28959, 28979, 20332 });
			position = LevelWad.layoutClasses[2].mobys[index].position;
			LevelWad.layoutClasses[2].UpdateMobyPosition(rIndex, position);
			LevelWad.layoutClasses[2].DeleteMoby(index);

			// Moneybags
			PosInfo posInfo = { { 118813, 79343, 17831 }, { 0, 0, 0x8F } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(180, { posInfo });

			// Gems
			vector<Vector3D> positions = {
				{ 91844, 80883, 26574 },
				{ 90106, 81267, 26188 },
				{ 83186, 81983, 26846 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);

			// Jack - disable
			vector<int> jackIndices = LevelWad.layoutClasses[2].GetMobyIndexFromClass(384);
			for (int i = 0; i < jackIndices.size(); i++) {
				LevelWad.layoutClasses[2].DeleteMoby(jackIndices[i]);
			}

			// Area 1 fairy - disable
			index = LevelWad.layoutClasses[1].GetMobyIndexFromClass(388)[0];
			LevelWad.layoutClasses[1].DeleteMoby(index);

			// Cpt. Byrd
			UpdateName(&LevelWad, 1, 89, 10, "Cpt. Byrd");

			// Moneybags 180 1 n
			MoveDialogueStringForwards(&LevelWad, 0, 180, 1, 3, 140);
			MoveDialogueStringForwards(&LevelWad, 0, 180, 1, 2, 168);
			MoveDialogueStringForwards(&LevelWad, 0, 180, 1, 1, 56);

			string msg = "There are two eggs that you can lock yourself out of if you collect the end of level eggs in their levels before you find them... but there might be other ways. Step 1: give me gems. Step 2... we'll get to that after Step 1. &Pay ^^^^ gems for the info? &Yes. &No.";
			UpdateDialogue(&LevelWad, 0, 180, 1, 0, msg);
			msg = "You have chosen wisely, Spyro. In Icy Catacombs, 'Secret in the doorframe.' is also in the Agent 9 area invisible on the wall behind the entrance portal. In Amethyst Isle, 'The impossible egg.' is also inside the end of the level crystal tower. Swim under the island to reach it.";
			UpdateDialogue(&LevelWad, 0, 180, 1, 1, msg);
			msg = "Well, if you change your mind, you know where to find me. The line between confidence and arrogance is a thin one, my friend.";
			UpdateDialogue(&LevelWad, 0, 180, 1, 2, msg);
			msg = "Go find some gems, you poor bastard. &[^^^^ gems for some help.";
			UpdateDialogue(&LevelWad, 0, 180, 1, 3, msg);
			msg = "Oh, piss off.";
			UpdateDialogue(&LevelWad, 0, 180, 1, 4, msg);

			UpdateSkybox("resources/34.sky", LevelData.skyboxPointer);
			UpdateLighting("resources/34.lighting", LevelData.block1Pointer);
		}
		else if (levelNo == 23) { // Honey Speedway

			// Sparx - change message 1 to point at message 0 instead (experimental)
			int sparxIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(144)[0]; // Speedway Sparx is 144
			char* sparxTag = LevelWad.layoutClasses[0].GetTagPointer(sparxIndex);
			int* dialoguePointers = (int*)(sparxTag + 16); // 0xC is name, 0x10 is msg 0
			dialoguePointers[1] = dialoguePointers[0];

			// Sparx - change msg 0 string
			char* messagePointer = LevelWad.layoutClasses[0].layoutData + dialoguePointers[0];
			messagePointer += *messagePointer; // Pass over dialogue header
			const char* newMessage = "Which challenge would you like to play? &Choose a challenge: &Say again? &Race the Bumblebees &Exit Level"; // Shorter than original
			memcpy(messagePointer, (char*)newMessage, strlen(newMessage));
			*(messagePointer + strlen(newMessage)) = 0; // Null-terminate

			// Hunter - disable
			int hunterIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(376)[0];
			LevelWad.layoutClasses[0].DeleteMoby(hunterIndex); // This is crude - maybe I just need to put him in an uninteractable state instead

			UpdateSkybox("resources/35.sky", LevelData.skyboxPointer);
			UpdateVram("resources/35.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/35.texture", LevelData.block0Pointer);
			UpdateLighting("resources/35.lighting", LevelData.block1Pointer);

		}
		else if (levelNo == 24) { // Bentley's Outpost

			// Change spawn point
			LayoutHeader* header = (LayoutHeader*)LevelWad.layoutClasses[0].layoutData;
			header->spawnPosition = { 102070, 13284, 9730 };
			header->spawnRotation = { 0, 0, 0x41 };

			// Move a totem pole piece (experimental)
			// Vector3D position = { 106864, 60226, 12009 };
			// int index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(69, position);
			// LevelWad.layoutClasses[0].UpdateMobyPosition(index, position);

			// Copy Class 75 (Pushable Block)
			int index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(75)[0];
			Moby block = LevelWad.layoutClasses[0].GetMoby(index);
			int taglen = LevelWad.layoutClasses[0].GetTagLength(index);
			vector<char> tag = LevelWad.layoutClasses[0].GetMobyTag(index, taglen);
			// block.state = 6; // Experimental

			// Add some blocks
			vector<PosInfo> positions = {
				{ { 65670, 45767, 11878 }, { 0, 0, 0xA5 } },
				{ { 42762, 42235, 12417 }, { 0, 0, 0xFC } },
				{ { 94629, 41472, 10259 }, { 0, 0, 0xCF } }, // previously { 94298, 41372, 10259 }
				{ { 109324, 38843, 9000 }, { 0, 0, 0x7E } }  // { { 110011, 38836, 9000 }, { 0, 0, 0x7E } }
			};
			for (int i = 0; i < positions.size(); i++) {
				block.position = positions[i].pos;
				block.angle = positions[i].angle;
				LevelWad.layoutClasses[0].AddMoby(block, tag);
			}

			// Disable portal?
			index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(991)[0];
			BoundingBox* portalTag = (BoundingBox*)(LevelWad.layoutClasses[0].layoutData + LevelWad.layoutClasses[0].mobys[index].mobyTag);
			portalTag->pos = { 0, 0, 0 };
			index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(3)[0];
			LevelWad.layoutClasses[0].DeleteMoby(index);
			// Note that we'll still use the above as a spawn position for Bentley, assuming this is the moby he's linked to

			// index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(3)[0];
			// Vector3D pos = LevelWad.layoutClasses[0].mobys[index].position;
			// int portalIndex = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(991, pos);
			// LevelWad.layoutClasses[0].DeleteMoby(portalIndex);
			// LevelWad.layoutClasses[0].DeleteMoby(index);

			// Move Bentley spawn point
			index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(15)[0];
			int* bentleyTag = (int*)(LevelWad.layoutClasses[0].layoutData + LevelWad.layoutClasses[0].mobys[index].mobyTag);
			index = bentleyTag[17];
			PosInfo posInfo = { { 109935, 38812, 10490 }, { 0, 0, 0x7F } }; // 109324, 38843, 10490 // { { 110545, 38782, 10490 }, { 0, 0, 0x7F } };
			if (!explorationMode) { // spawn position is exploration only, otherwise it just uses the standard spawn position
				LevelWad.layoutClasses[0].UpdateMobyPosition(index, posInfo);
			}
			// This won't cause warping issues because we've disabled this portal! :)
			// Bentley's ACTUAL position before speaking to him is unchanged

			// Move gong (challenge only)
			Vector3D pos = { 96748, 57180, 11947 };
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(137, pos);
			posInfo = { { 81681, 33207, 12323 }, { 0, 0, 0x0D } };
			if (!explorationMode) {
				LevelWad.layoutClasses[0].UpdateMobyPosition(index, posInfo);
			}
			
			// Gems
			vector<Vector3D> posns = {
				{ 71163, 55377, 19107 },
				{ 73356, 56941, 19675 },
				{ 94787, 49526, 16350 },
				{ 95226, 50788, 16675 },
				{ 33313, 65888, 18604 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, posns);
			posns = {
				{ 74441, 54618, 20192 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(200, posns);

			// Move a red gem, trying not to change the total gem count :)
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1);
			pos = { 22990, 60231, 16888 };
			for (int i = 5; i < indices.size(); i++) {
				if (LevelWad.layoutClasses[0].mobys[indices[i]].levelSectorIndex == 1) {
					LevelWad.layoutClasses[0].UpdateMobyPosition(indices[i], pos);
					break;
				}
				if (i + 1 == indices.size()) {
					cout << "ERROR - can't move a gem in Bentley's Outpost" << endl;
				}
			}

			// Move another basket - previously we deleted this, we might as well keep it but move it just in case we delete one of the ones we want to actually move
			// There's a lot of other baskets before this one in both versions, anyway
			pos = { 94298, 41372, 10856 };
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(200, pos);
			posInfo = { { 47748, 64418, 15901 }, { 0x06, 0x00, 0xFB } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, posInfo);

			// Rotate / move eggs in exploration mode
			if (explorationMode) {
				index = LevelWad.layoutClasses[0].GetEggIndexByNumber(2);
				LevelWad.layoutClasses[0].mobys[index].angle.z += 0x80;

				index = LevelWad.layoutClasses[0].GetEggIndexByNumber(0);
				posInfo = { { 18519, 54207, 13669 }, { 0, 0, 0xFF } };
				LevelWad.layoutClasses[0].UpdateMobyPosition(index, posInfo);
			}

			// Bartholomew
			string msg = "This one's for Kampfi! Another gong!";
			UpdateDialogue(&LevelWad, 0, 135, 0, 7, msg);

			UpdateSkybox("resources/36.sky", LevelData.skyboxPointer);
			UpdateLighting("resources/36.lighting", LevelData.block1Pointer);

		}
		else if (levelNo == 26) { // Starfish Reef

			// Delete Sparx
			int index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(368)[0];
			LevelWad.layoutClasses[0].DeleteMoby(index);

			// End of level portal
			Vector3D position = { 41616, 109568, 10199 };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(453, { position });

			// Change spawn point
			LayoutHeader* header = (LayoutHeader*)LevelWad.layoutClasses[0].layoutData;
			header->spawnPosition = { 41312, 103130, 10600 };
			header->spawnRotation = { 0, 0, 0x40 };

		}
		else if (levelNo == 27) { // Midnight Mountain

			// Egg reqs
			vector<int> portals = LevelWad.layoutClasses[0].GetMobyIndexFromClass(988);
			for (int i = 0; i < portals.size(); i++) {
				Moby988Tag* portalTag = (Moby988Tag*)(LevelWad.layoutClasses[0].GetTagPointer(portals[i]));
				if (portalTag->LevelId == 43) {
					portalTag->EggReq = (explorationMode ? 1 : 45);
				}
				else if (portalTag->LevelId == 44) {
					portalTag->EggReq = (explorationMode ? 1 : 45);
				}
				else if (portalTag->LevelId == 45) {
					portalTag->EggReq = 47;
				}
				else if (portalTag->LevelId == 50) {
					portalTag->EggReq = 50;
				}
			}

			// Moneybags
			PosInfo posInfo = { { 46759, 119599, 14070 }, { 0, 0, 0xD1 } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(198, { posInfo });

			// Thief
			int index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(506)[0];
			LevelWad.layoutClasses[0].DeleteMoby(index);

			// Sorc portal
			index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(949)[0];
			//LevelWad.layoutClasses[0].mobys[index].position.z -= 6000; // Sorceress skips have been reinstated as of release version 1.0.2

			//BoundingBox* tag = (BoundingBox*)(LevelWad.layoutClasses[0].layoutData + LevelWad.layoutClasses[0].mobys[index].mobyTag);
			//tag->uh oh; // there's no z component

			// Sorc entrance text - doesn't affect the actual requirements, though
			index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(950)[0];
			int* entranceTag = (int*)(LevelWad.layoutClasses[0].GetTagPointer(index));
			entranceTag[0] = 80;

			// Gems
			vector<Vector3D> positions = {
				{ 29109,  79984, 13760 }, // 10
				{ 22848,  82951, 15088 }, // 10
				{ 13337,  78301, 16073 }, // 25 - formerly { 17962, 74909, 15674 }
				// { 49535, 77712, 15320 } // deprecated // EXPERIMENTAL - this one's responsible for the whirlwind currently
				{ 36345,  45134, 20807 },
				{ 37508,  43572, 20808 },
				{ 73508, 108782, 33912 },
				{ 77552, 111048, 33918 },
				{ 82171, 111045, 33918 },
				{ 86196, 108737, 33919 },
				{ 88490, 104737, 33920 },
				{ 95420,  54900, 16375 }, // 1
				{ 47035, 118894, 14072 }  // 25
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1);
			LevelWad.layoutClasses[0].mobys[indices[0]].levelSectorIndex = 10;
			LevelWad.layoutClasses[0].mobys[indices[1]].levelSectorIndex = 10;
			LevelWad.layoutClasses[0].mobys[indices[2]].levelSectorIndex = 25;
			LevelWad.layoutClasses[0].mobys[indices[10]].levelSectorIndex = 1;
			LevelWad.layoutClasses[0].mobys[indices[11]].levelSectorIndex = 25;

			// DO NOT UPDATE INDICES HERE - THE WHIRLWINDS USE IT BELOW
			// Whirlwind
			LevelWad.layoutClasses[0].DuplicateMoby(1023, 0, { { 109782, 66916, 10856 }, { 0, 0, 0xD1 } });
			int* whirlwindTag = (int*)(LevelWad.layoutClasses[0].GetTagPointer(LevelWad.layoutClasses[0].mobyBlock->count - 1));
			whirlwindTag[0] = 3500; // height // 3500 seems to be the lowest allowed
			//whirlwindTag[1] width
			whirlwindTag[2] = 5; // type
			whirlwindTag[5] = indices[10]; // linked to 1 gem

			// Whirlwind - Monkey Puzzle
			LevelWad.layoutClasses[0].DuplicateMoby(1023, 0, { { 13337, 78301, 16073 }, { 0, 0, 0xD8 } }); // experimental pos - { { 49535, 77712, 15320 }, { 0, 0, 0xAF } }
			whirlwindTag = (int*)(LevelWad.layoutClasses[0].GetTagPointer(LevelWad.layoutClasses[0].mobyBlock->count - 1));
			whirlwindTag[0] = 16927; // height
			whirlwindTag[2] = 5; // type
			whirlwindTag[5] = indices[2]; // linked to moved 25 gem // linked moby (I think 4 is a link too, not sure what for though)
			// type 5 with tag[5] linked to a moby that can be collected - when it's collected, the whirlwind spawns
			// or really any moby that can have state >= 0x80

			// Whirlwind - another one
			LevelWad.layoutClasses[0].DuplicateMoby(1023, 0, { { 122910, 36946, 4600 }, { 0, 0, 0x91 } });
			whirlwindTag = (int*)(LevelWad.layoutClasses[0].GetTagPointer(LevelWad.layoutClasses[0].mobyBlock->count - 1));
			whirlwindTag[0] = 35700; // height
			whirlwindTag[2] = 5; // type
			whirlwindTag[5] = indices[11]; // linked to 25 gem in front of Moneybags

			// Whirlwind - life jar one
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(563, { 69233, 11254, 23069 });
			LevelWad.layoutClasses[0].DuplicateMoby(1023, 0, { { 69233, 11254, 23069 }, { 0, 0, 0x3F } });
			whirlwindTag = (int*)(LevelWad.layoutClasses[0].GetTagPointer(LevelWad.layoutClasses[0].mobyBlock->count - 1));
			whirlwindTag[0] = 41569; // height
			whirlwindTag[2] = 5; // type
			whirlwindTag[5] = index; // linked to a life jar

			// Headbash crates
			positions = {
				{ 16761, 81558, 15000 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(202, positions);

			// Sparx sign
			posInfo = { { 12557, 77811, 16007 }, { 0, 0, 0x16 } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(325, { posInfo });

			// Delete final Moneybags
			index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(179)[0];
			LevelWad.layoutClasses[0].DeleteMoby(index);

			// Bianca 41 10 0 - note that there's two of these for some reason, so the implementation here is awkward
			indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(41);
			for (int i = 0; i < indices.size(); i++) {
				string msg = "You're nearly ready to take on the Sorceress again, Spyro! I knew we could count on you! The Sorceress may be stronger and cleverer than she was last time, but so are you!... You've proven that just by making it here. Keep going - we only need a few more eggs! The fate of the Forgotten Realms rests on your shoulders!";
				UpdateDialogueFromIndex(&LevelWad, 0, indices[i], 0, msg);
			}

			// Moneybags 198 0 n
			MoveDialogueStringBackwards(&LevelWad, 0, 198, 0, 1, 8);
			MoveDialogueStringForwards(&LevelWad, 0, 198, 0, 2, 44);
			MoveDialogueStringForwards(&LevelWad, 0, 198, 0, 3, 4);

			string msg = "A dog in Icy Catacombs told me a riddle that apparently leads to a hidden treasure. It could be worth a fortune! I'll be rich!... Err, well... more rich. Unfortunately, I haven't been able to figure it out yet. Do you want to hear it? &Pay ^^^ gems to hear the riddle? &Sure. &No thanks.";
			UpdateDialogue(&LevelWad, 0, 198, 0, 0, msg);
			msg = "Mmm... gems... As always, thank you, Spyro... Ahem... 'From the reptile island, let the lighthouse call you home. Glide between twin falls. Fear not! Water will not be what sweeps you away.' ... whatever that means.";
			UpdateDialogue(&LevelWad, 0, 198, 0, 1, msg);
			msg = "Fine! It's your loss, Spyro. I guess you don't actually care about finding dragon eggs. Suit yourself.";
			UpdateDialogue(&LevelWad, 0, 198, 0, 2, msg);
			msg = "If you don't have any gems for me, leave me alone, Spyro. I'm trying to concentrate on something. If I can solve this puzzle, I'll be a millionaire once and for all! Do you have any idea how many monocles I could buy with that kind of cash? &[^^^ gems needed to speak with the bear.";
			UpdateDialogue(&LevelWad, 0, 198, 0, 3, msg);

			// Sign
			msg = "Wheeeeeee!";
			UpdateDialogue(&LevelWad, 0, 325, 150, 0, msg);

			UpdateSkybox("resources/40.sky", LevelData.skyboxPointer);
			UpdateLighting("resources/40.lighting", LevelData.block1Pointer);
		}
		else if (levelNo == 28) {

			// Move chest
			PosInfo posinfo = { { 57082, 91229, 17878 }, { 0, 0, 0x5B } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(205, { posinfo });

			// Move key
			Vector3D keyPos = { 84175, 69193, 20248 };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(212, { keyPos });

			// Move Moneybags (198)
			posinfo = { { 67441, 85888, 11225 }, { 0, 0, 0x33 } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(198, { posinfo });

			// Experimental - increase render distance of egg
			int index = LevelWad.layoutClasses[1].GetEggIndexByNumber(4); // formerly 2
			if (index != -1) {
				LevelWad.layoutClasses[1].mobys[index].lowDrawDistance *= 2;
				LevelWad.layoutClasses[1].mobys[index].highDrawDistance *= 2;
			}
			else {
				printf("Unable to update Hank.\n");
			}

			// Thief
			//index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(279)[0];
			//LevelWad.layoutClasses[0].DeleteMoby(index); // Now enabled again
			posinfo = { { 67928, 87838, 9000 }, { 0, 0, 0xF4 } }; // previously { { 88984, 73336, 13086 }, { 0, 0, 0x72 } }
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(279, { posinfo });

			// Gems
			vector<Vector3D> positions = {
				{ 115098, 94770, 20860 },
				{ 116761, 89981, 22292 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);
			positions = {
				{ 83145, 47778, 25045 },
				{ 76474, 43068, 25047 },
				{ 66807, 42877, 25045 },
				{ 60154, 47766, 25414 }
			};
			LevelWad.layoutClasses[1].UpdateMobyPositionsByClass(1, positions);
			positions = {
				{ 129550, 99048, 27402 },
				{ 90124, 99018, 23799 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(201, positions);
			posinfo = { { 110141, 94924, 23514 }, { 0x09, 0xEE, 0xC5 } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(200, { posinfo });

			// Whirlwind
			LevelWad.layoutClasses[0].DuplicateMoby(1023, 0, { 68171, 87578, 3784 });
			int* whirlwindTag = (int*)(LevelWad.layoutClasses[0].GetTagPointer(LevelWad.layoutClasses[0].mobyBlock->count - 1));
			whirlwindTag[0] = 3500; // height
			//whirlwindTag[1] = 4000; // width // REVERT THIS
			whirlwindTag[2] = 0; // type

			// Disable Bentley area NPC
			index = LevelWad.layoutClasses[2].GetMobyIndexFromClass(540)[0];
			LevelWad.layoutClasses[2].DeleteMoby(index);

			// Moneybags
			index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(198)[0];
			//cout << "DEBUG - REMOVE LATER - Moneybags index in Crystal Islands is " << dec << index << endl;
			int* moneybagsTag = (int*)(LevelWad.layoutClasses[0].layoutData + LevelWad.layoutClasses[0].mobys[index].mobyTag);
			//cout << "DEBUG - REMOVE LATER - Moneybags moby link in Crystal Islands is " << dec << moneybagsTag[10] << endl;
			//cout << "DEBUG - REMOVE LATER - Moneybags animation index in Crystal Islands is " << dec << moneybagsTag[12] << endl;
			moneybagsTag[10] = -1; // Moby -1 - hopefully will disable the bridge from extending

			// Copy Dragon 2
			vector<char> hankEggTag = sampleEggTag;
			hankEggTag[0] = 4; // formerly 2
			Moby hankEgg = sampleEgg;
			hankEgg.position = { 113811, 34189, 21969 };
			hankEgg.angle.z = 0xC0;
			LevelWad.layoutClasses[0].AddMoby(hankEgg, hankEggTag);

			// Speed boost stars
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(338);
			for (int i = 0; i < indices.size(); i++) {
				LevelWad.layoutClasses[0].DeleteMoby(indices[i]);
			}

			// Turtle fodder in slide area
			indices = LevelWad.layoutClasses[1].GetMobyIndexFromClass(562);
			for (int i = 0; i < indices.size(); i++) {
				LevelWad.layoutClasses[1].DeleteMoby(indices[i]);
			}

			// Renaming NPCs
			UpdateName(&LevelWad, 0, 540, 0, "The Marvelous HWD");
			UpdateName(&LevelWad, 0, 540, 2, "The Magnificent Shemp");
			UpdateName(&LevelWad, 0, 540, 3, "The Great JT");
			UpdateName(&LevelWad, 0, 540, 4, "The Astounding Altro");
			UpdateName(&LevelWad, 0, 540, 5, "The Fantastic Fissure");

			// Moneybags 198 7 n
			MoveDialogueStringForwards(&LevelWad, 0, 198, 7, 3, 104);
			MoveDialogueStringForwards(&LevelWad, 0, 198, 7, 2, 104);
			MoveDialogueStringForwards(&LevelWad, 0, 198, 7, 1, 16);

			string msg = "Spyro, did you know that I became a professional magician? After you forced me to rock bottom, I couldn't even afford a hot air balloon ride to Spooky Swamp to study poetry. Anyway, since then, I've been working on a magic trick just for you... Do you want to see it? &Pay ^^^ gems to see trick? &Ok, fine... &Not a chance.";
			UpdateDialogue(&LevelWad, 0, 198, 7, 0, msg);
			msg = "Very good! Errr... let's see... 'Hocus Pocus, Zigglety Zear, make this dragon's gems all disappear!' Have fun being a 'hero', Spyro.";
			UpdateDialogue(&LevelWad, 0, 198, 7, 1, msg);
			msg = "You shouldn't anger a magician, Spyro. I just might decide to turn you into a blue hedgehog or something.";
			UpdateDialogue(&LevelWad, 0, 198, 7, 2, msg);
			msg = "Hmph... These gnomes think they're so talented... I happen to know a pretty good magic trick myself. You'll need a few more gems if you want to see it, of course. Magician's school wasn't cheap. &[Magic show costs ^^^ gems.";
			UpdateDialogue(&LevelWad, 0, 198, 7, 3, msg);

			// Sign
			msg = "There's nothing in here. Look somewhere else!";
			UpdateDialogue(&LevelWad, 0, 408, 130, 2, msg);

			UpdateSkybox("resources/41.sky", LevelData.skyboxPointer);
			UpdateLighting("resources/41.lighting", LevelData.block1Pointer);

		}
		else if (levelNo == 29) {

			// Andy - Dragon 5
			int index = LevelWad.layoutClasses[0].GetEggIndexByNumber(5);
			if (index != -1) {
				LevelWad.layoutClasses[0].mobys[index].lowDrawDistance = 0;
			}
			else {
				printf("ERROR - Desert Ruins - Unable to update Andy.\n");
			}

			// Zoe
			int moneybagsIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(198)[0];
			Vector3D position = LevelWad.layoutClasses[0].mobys[moneybagsIndex].position;
			int zoeIndex = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(272, position);
			PosInfo posinfo = { { 50285, 73476, 23151 }, { 0, 0, 0x27 } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(zoeIndex, posinfo);

			// Moneybags
			posinfo = { { 134617, 101145, 22488 }, { 0, 0, 0x83 } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(moneybagsIndex, posinfo);
			// EXPERIMENTAL - update mobytag + 0x30 to 0
			int* moneybagsTag = (int*)(LevelWad.layoutClasses[0].layoutData + LevelWad.layoutClasses[0].mobys[moneybagsIndex].mobyTag);
			moneybagsTag[12] = 0; // Animation 0 - one of the hands
			// PUT EGG AT { 46844, 79592, 22402 }, (char)0x80

			// Move chest
			posinfo = { { 75393, 58987, 24536 }, { 0, 0, 0xED } }; // Formerly { 43805, 92611, 22812 }, (char)0x9F
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(205, { posinfo });

			// Gus
			posinfo = { { 39596, 79692, 20439 }, { 0, 0, 0xA9 } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(529, { posinfo });

			// Key
			position = { 76803, 38354, 25345 };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(212, { position });

			// Sign
			posinfo = { { 77891, 39541, 20438 }, { 0, 0, 0xC0 } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(683, { posinfo });

			// Set MobyTag + 0x10 ints to -1 for all scorpions (496) as this is the index of the eggs
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(496);
			int indexA = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(496, { 142725, 106865, 22528 }); // 129526, 101069, 22528 // 37888, 55552, 20578
			int indexB = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(496, { 67072, 50227, 19456 }); // 33536, 63171, 20736
			if (indexA == indexB) {
				printf("ERROR - both scorpion indices are the same!\n");
			}
			int eggIndex = LevelWad.layoutClasses[0].GetEggIndexByNumber(3);
			for (int i = 0; i < indices.size(); i++) {
				int len = LevelWad.layoutClasses[0].GetTagLength(indices[i]);
				if (len < 4 * 0x11) {
					continue;
				}

				int* tag = (int*)(LevelWad.layoutClasses[0].layoutData + LevelWad.layoutClasses[0].mobys[indices[i]].mobyTag);
				if (indices[i] == indexA || indices[i] == indexB) {
					tag[16] = eggIndex;
					//cout << "DEBUG - updated tag in moby " << dec << indices[i] << endl;
					continue;
				}

				tag[16] = -1;
			}

			// New large scorpions
			LevelWad.layoutClasses[0].DuplicateMoby(525, 0, { { 100074, 28902, 25560 }, { 0, 0, 0x12 } });
			LevelWad.layoutClasses[0].DuplicateMoby(525, 0, { { 118153, 40568, 24538 }, { 0, 0, 0x64 } });

			// New small scorpions
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(496, { 50258, 74598, 22016 }); // this one is on easy mode and it's stationary, it's also definitely not linked to an egg or a platform
			indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(496);
			for (int i = 0; i < indices.size(); i++) {
				if (indices[i] == index) {
					index = i; // just for DuplicateMoby
					break;
				}
			}
			LevelWad.layoutClasses[0].DuplicateMoby(496, index, { { 92343, 28372, 26582 }, { 0, 0, 0x09 } });
			LevelWad.layoutClasses[0].DuplicateMoby(496, index, { { 94151, 27045, 26584 }, { 0, 0, 0x12 } });


			// EXPERIMENTAL WHIRLWIND THINGS
			/*
			index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1023)[0];
			Moby whirlwind = LevelWad.layoutClasses[0].GetMoby(index);
			int len = LevelWad.layoutClasses[0].GetTagLength(index);
			vector<char> tag = LevelWad.layoutClasses[0].GetMobyTag(index, len);
			Moby whirlwindA = whirlwind;
			whirlwindA.lowDrawDistance = 0;
			whirlwindA.highDrawDistance = 0;
			whirlwindA.position = { 36642, 86157, 20439 };
			*(int*)(&(tag[0])) = 6000;
			LevelWad.layoutClasses[0].AddMoby(whirlwindA, tag);
			whirlwind.position = { 27168, 58519, 22000 };
			whirlwind.angle = { 0, 0x40, 0 };
			*(int*)(&(tag[0])) = 5000;
			LevelWad.layoutClasses[0].AddMoby(whirlwind, tag);
			*/

			// Add a couple of special gem
			// AddSpecialGem(&LevelWad, 0, { 59898, 71544, 24984 });
			// AddSpecialGem(&LevelWad, 0, { 86592, 72293, 30915 }); // not sure about this one
			// These break the game :(

			// Move the sublevel entrances
			indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1022);
			Moby1022Tag newTag;
			Vector3D newPos;
			for (int i = 0; i < indices.size(); i++) {
				// Get the data
				Moby* moby = &(LevelWad.layoutClasses[0].mobys[indices[i]]);
				Moby1022Tag* tag = (Moby1022Tag*)(LevelWad.layoutClasses[0].layoutData + moby->mobyTag);
				if (tag->fromArea == 0 && tag->toArea == 1) { // Sheila
					// Update Visuals - moving this to the spot where the **Sharks** entrance is
					int visualIndex = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(3, tag->box.pos);
					//int visualIndex = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(3, { 119398, 66161, 23040 });
					PosInfo pos = { { 99432, 79760, 19415 }, { 0, 0x40, 0 } }; // check it's not upside down? if that's even a thing

					//cout << "tag box pos " << tag->box.pos.x << ", " << tag->box.pos.y << ", " << tag->box.pos.z << endl;
					//cout << "visual pos " << LevelWad.layoutClasses[0].mobys[visualIndex].position.x;
					//cout << ", " << LevelWad.layoutClasses[0].mobys[visualIndex].position.y << ", " << LevelWad.layoutClasses[0].mobys[visualIndex].position.z << endl;

					LevelWad.layoutClasses[0].UpdateMobyPosition(visualIndex, pos);
				}
				else if (tag->fromArea == 0 && tag->toArea == 2) { // Sharks
					// Entrance
					newTag = *tag;
					newPos = moby->position;
					//cout << "DEBUG - found suitable portal in Desert Ruins" << endl;
				}
			}
			for (int i = 0; i < indices.size(); i++) {
				Moby* moby = &(LevelWad.layoutClasses[0].mobys[indices[i]]);
				Moby1022Tag* tag = (Moby1022Tag*)(LevelWad.layoutClasses[0].layoutData + moby->mobyTag);
				if (tag->fromArea == 0 && tag->toArea == 1) { // Sheila
					// Entrance
					moby->position = newPos;
					tag->box = newTag.box;
					tag->unknown = newTag.unknown;
					tag->musicZWidth = newTag.musicZWidth;
					tag->entranceZWidth = newTag.entranceZWidth;
				}
				else if (tag->fromArea == 0 && tag->toArea == 2) { // Sharks
					// Entrance
					moby->position = { 99432, 79760, 19415 };
					tag->box.pos = { 99432, 79760, 19415 };
					tag->box.widthX = 600;
					tag->box.widthY = 600;
					tag->unknown.pos = { 99432, 79760, 19415 };
					tag->unknown.widthX = 600;
					tag->unknown.widthY = 600;
					tag->musicZWidth = 200;
					tag->entranceZWidth = 500;
				}
			}

			// Change spawn point
			LayoutHeader* header = (LayoutHeader*)LevelWad.layoutClasses[1].layoutData; // this probably doesn't even work does it
			header->spawnPosition = { 15677, 9953, 29030 };
			header->spawnRotation = { 0, 0, 0 };

			// Area 2 entrance position
			posinfo = { { 79890, 73965, 25400 }, { 0, 0, 0xC3 } };
			LevelWad.UpdateSublevelExit(0, 2, posinfo);

			// Leave Tara (468) Alone!! Remove 965 instead
			vector<int> eolIndices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(965);
			for (int i = 0; i < eolIndices.size(); i++) {
				LevelWad.layoutClasses[0].DeleteMoby(eolIndices[i]);
			}

			// Sheila area enemies
			vector<int> scorpIndices = LevelWad.layoutClasses[1].GetMobyIndexFromClass(466);
			vector<int> metalIndices = LevelWad.layoutClasses[1].GetMobyIndexFromClass(652);
			vector<PosInfo> positions;
			for (int i = 0; i < scorpIndices.size(); i++) {
				positions.push_back({ LevelWad.layoutClasses[1].mobys[scorpIndices[i]].position, LevelWad.layoutClasses[1].mobys[scorpIndices[i]].angle });
				LevelWad.layoutClasses[1].DeleteMoby(scorpIndices[i]);
			}
			for (int i = 0; i < metalIndices.size(); i++) {
				positions.push_back({ LevelWad.layoutClasses[1].mobys[metalIndices[i]].position, LevelWad.layoutClasses[1].mobys[metalIndices[i]].angle });
				LevelWad.layoutClasses[1].DeleteMoby(metalIndices[i]);
			}
			index = LevelWad.layoutClasses[1].GetMobyIndexFromClass(496)[0];
			positions.push_back({ { 90998, 15690, 37850 }, { 0, 0, 0xFF } }); // extra position
			positions.push_back({ { 74398, 15586, 37847 }, { 0, 0, 0x00 } }); // extra position
			for (int i = 0; i < positions.size(); i++) {
				LevelWad.layoutClasses[1].DuplicateMoby(index, positions[i]);
				int* tag = (int*)(LevelWad.layoutClasses[1].GetTagPointer(LevelWad.layoutClasses[1].mobyBlock->count - 1));
				tag[2] = -1; // Turns off platform links
			}

			// Gems
			indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1);
			vector<Vector3D> posns = {
				{ 82686, 37628, 33340 },
				{ 85526, 71191, 31561 },
				{ 94796, 99367, 23559 },
				{ 73450, 95671, 22486 },
				{ 42929, 56898, 30113 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, posns);
			for (int i = 0; i < posns.size(); i++) {
				LevelWad.layoutClasses[0].mobys[indices[i]].levelSectorIndex = 25;
			}

			indices = LevelWad.layoutClasses[1].GetMobyIndexFromClass(1);
			posns = {
				{ 134715, 15291, 32729 }, // 1
				{ 128961, 15293, 33754 }, // 2
				{ 138059, 15212, 35766 }, // 5
				{ 126938, 15396, 36826 }, // 10
				{ 115665, 15387, 37849 }  // 25
			};
			LevelWad.layoutClasses[1].UpdateMobyPositionsByClass(1, posns);
			LevelWad.layoutClasses[1].mobys[indices[0]].levelSectorIndex = 1;
			LevelWad.layoutClasses[1].mobys[indices[1]].levelSectorIndex = 2;
			LevelWad.layoutClasses[1].mobys[indices[2]].levelSectorIndex = 5;
			LevelWad.layoutClasses[1].mobys[indices[3]].levelSectorIndex = 10;
			LevelWad.layoutClasses[1].mobys[indices[4]].levelSectorIndex = 25;

			indices = LevelWad.layoutClasses[2].GetMobyIndexFromClass(1);
			posns = {
				{ 69076, 43641, 27014 },
				{ 64069, 43621, 27014 },
				{ 75414, 48055, 24175 },
				{ 66558, 31245, 24181 }
			};
			LevelWad.layoutClasses[2].UpdateMobyPositionsByClass(1, posns);
			LevelWad.layoutClasses[2].mobys[indices[0]].levelSectorIndex = 2;
			LevelWad.layoutClasses[2].mobys[indices[1]].levelSectorIndex = 2;
			LevelWad.layoutClasses[2].mobys[indices[2]].levelSectorIndex = 5;
			LevelWad.layoutClasses[2].mobys[indices[3]].levelSectorIndex = 25;

			// Baskets
			index = LevelWad.layoutClasses[1].GetMobyIndexFromClass(200)[0];
			vector<PosInfo> posinfos = {
				{ 69987, 14183, 35799 },
				{ 68197, 14071, 35799 },
				{ 66321, 14074, 35799 }
			};
			for (int i = 0; i < posinfos.size(); i++) {
				LevelWad.layoutClasses[1].DuplicateMoby(index, posinfos[i]);
			}

			// Armoured chest
			position = { 66518, 36856, 30180 };
			LevelWad.layoutClasses[2].UpdateMobyPositionsByClass(203, { position });

			// Delete Sheila
			index = LevelWad.layoutClasses[1].GetMobyIndexFromClass(57)[0];
			LevelWad.layoutClasses[1].DeleteMoby(index);

			// Hunter
			index = LevelWad.layoutClasses[2].GetMobyIndexFromClass(277)[0];
			LevelWad.layoutClasses[2].DeleteMoby(index);

			// Gus 529 0 0
			string msg = "Amazing! I think we've been transported into the future! Judging by the markings on these robot heads, they were Rhynoc war machines. I could've sworn I saw one of the hands move earlier. Maybe I should clean my glasses. The scorpions seem to be guarding something. You might have to kill them all before you can collect it.";
			UpdateDialogue(&LevelWad, 0, 529, 0, 0, msg);

			// Moneybags 198 1 0
			MoveDialogueStringBackwards(&LevelWad, 0, 198, 1, 1, 36);
			MoveDialogueStringForwards(&LevelWad, 0, 198, 1, 3, 56);
			MoveDialogueStringForwards(&LevelWad, 0, 198, 1, 2, 56);

			msg = "I rigged one of the robot hands to move when I press this button and I've been using it all day to scare Gus. Here, I'll let you try. Just 'hand' over some gems first... heh heh... get it? Hand? &Pay ^^^ gems to move hand? &OK, fine. &Some other time.";
			UpdateDialogue(&LevelWad, 0, 198, 1, 0, msg);
			msg = "Heh heh, It really never gets old. Funny, right? It looks like the mechanism got jammed when the hand moved though. Can you go look to see what the gears got stuck on? Say hi to Gus for me!";
			UpdateDialogue(&LevelWad, 0, 198, 1, 1, msg);
			msg = "It's just as well. I was starting to get bored anyway. Run along then, Spyro. I only chat with paying customers.";
			UpdateDialogue(&LevelWad, 0, 198, 1, 2, msg);
			msg = "Spyro, do you want to help me play a prank on Gus? Go find some gems and I'll show you a funny trick. &[Pranking Gus costs ^^^ gems.";
			UpdateDialogue(&LevelWad, 0, 198, 1, 3, msg);
			msg = "Don't you have some baby dragons to save or something?";
			UpdateDialogue(&LevelWad, 0, 198, 1, 4, msg);

			// Sheila sign
			msg = "Sink the pillar.";
			UpdateDialogue(&LevelWad, 0, 683, 110, 2, msg);
			UpdateMessageOffset(&LevelWad, 0, 683, 110, 0, 2);
			UpdateMessageOffset(&LevelWad, 0, 683, 110, 1, 2);

			//UpdateSkybox("resources/42.sky", LevelData.skyboxPointer);
			UpdateVram("resources/42.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/42.texture", LevelData.block0Pointer);
			UpdateLighting("resources/42.lighting", LevelData.block1Pointer);

			UpdateSkybox("resources/42.sky", LevelWad.layoutClasses[0].layoutData + 0x30);
			UpdateSkybox("resources/42.sky", LevelWad.layoutClasses[1].layoutData + 0x30);

			auto Scene = SceneData(0, (unsigned char*)LevelWad.sceneDataBlocks[0], LevelWad.sceneHeaders[0].length);
			Scene.UpdateVram("resources/42.vram");
			LevelWad.UpdateScene(0);
			// Do other sublevels

			// Water
			Scene = SceneData(2, (unsigned char*)LevelWad.sceneDataBlocks[2], LevelWad.sceneHeaders[2].length);
			//cout << "Found " << Scene.collision.GetTrisInCylinder(100, 2500, { 66715, 36027, 23808 }).size() << " polygons in cylinder." << endl; // 256, 1720
			vector<int> polys = Scene.collision.GetTrisInCylinder(100, 2500, { 66715, 36027, 23808 });
			for (int i = 0; i < polys.size(); i++) {
				//cout << "Primary Z: " << Scene.collision.triangles[polys[i]].GetPoints()[0].z << endl;
				Scene.collision.triangles[polys[i]].UpdateZ(400);
			}
			LevelWad.UpdateScene(2);

			// Sheila polys
			Scene = SceneData(1, (unsigned char*)LevelWad.sceneDataBlocks[1], LevelWad.sceneHeaders[1].length);
			//cout << Scene.collision.collisionHeader->triCount << " total polys:" << endl;
			//cout << Scene.collision.collisionHeader->flaggedCount << " flagged polys:" << endl;
			//cout << Scene.collision.collisionHeader->unk1 << " ??? polys:" << endl;
			for (int i = 0; i < Scene.collision.collisionHeader->flaggedCount; i++) {
				//cout << Scene.collision.surfaceTypes[i] << endl;
				Scene.collision.surfaceTypes[i] = 65535;
			}
			LevelWad.UpdateScene(1);

		}
		else if (levelNo == 30) {

			// EOL NPC (487,3) and start of level NPC (487, 0)
			PosInfo position = { { 41729, 52767, 24024 }, { 0, 0, 0x43 } }; // formerly { { 46832, 59490, 24023 }, { 0, 0, 0x84 } }
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(487, 3, position);
			position = { { 118921, 63637, 18390 }, { 0, 0, 0x9A } };
			LevelWad.layoutClasses[0].UpdateNpcPositionByType(487, 0, position);

			// Move a vase
			Vector3D vasePosition = { 18591, 53350, 17878 }; // formerly 18193, 57553, 17881
			int index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(201, vasePosition);
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, vasePosition);

			// Move Zoes
			vector<PosInfo> positions = {
				//{ { 66971, 55716, 26290 }, { 0, 0, 0x9B } }, // deprecated
				{ { 68794, 57231, 23240 }, { 0, 0, 0x9A } }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(272, positions);

			// Baskets
			positions = {
				{ 47536, 59419, 24022 },
				{ 45358, 59268, 24025 },
				{ 43180, 59117, 24022 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(200, positions);

			// Gems
			positions = {
				{ 60523, 37541, 23510 },
				{ 54571, 33789, 23510 },
				{ 48767, 30136, 23910 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);

			// Life jars
			positions = {
				{ 25810, 69954, 24023 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(563, positions);

			// Earthshaper
			index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(492, { 116511, 75438, 16896 });
			position = { { 33383, 47034, 17880 }, { 0, 0, 0xDE } };
			LevelWad.layoutClasses[0].UpdateMobyPosition(index, position);
			// New Earthshaper // "Copy earthshaper at {33383, 47034, 17880 | DE} and place the copy at {40319, 47563, 17880 |9D}"
			position = { { 40319, 47563, 17880 }, { 0, 0, 0x9D } };
			LevelWad.layoutClasses[0].DuplicateMoby(index, position);

			// MJ - Dragon 5 (formerly 2)
			index = LevelWad.layoutClasses[0].GetEggIndexByNumber(5);
			//Moby mjEgg;
			//vector<char> mjTag;
			if (index != -1) {
				//mjEgg = LevelWad.layoutClasses[0].GetMoby(index);
				//int len = LevelWad.layoutClasses[0].GetTagLength(index); // since this is a weird case, there's a chance of it breaking
				//mjTag = LevelWad.layoutClasses[0].GetMobyTag(index,len);
				LevelWad.layoutClasses[0].mobys[index].lowDrawDistance = 0;
			}
			else {
				printf("ERROR - Haunted Tomb - Unable to update MJ.\n");
			}

			// Copy Dragon 2
			// One approach is above but commented out - just gonna use a generic egg instead
			vector<char> mjEggTag = sampleEggTag;
			mjEggTag[0] = 5; // formerly 2
			Moby mjEgg = sampleEgg;
			mjEgg.position = { 235630, 123205, 26071 };
			mjEgg.angle.z = 0xBD;
			mjEgg.lowDrawDistance = 0;
			mjEgg.highDrawDistance = 0;
			LevelWad.layoutClasses[2].AddMoby(mjEgg, mjEggTag);

			//cout << "Sample egg is now number " << (int)sampleEggTag[0] << endl;
			//cout << "MJ egg is now number " << (int)mjEggTag[0] << endl;

			// Move Tanks NPC
			index = LevelWad.layoutClasses[1].GetMobyIndexFromClass(487)[0];
			LevelWad.layoutClasses[1].mobys[index].position.x = 163440;
			LevelWad.layoutClasses[1].mobys[index].position.y = 65104;
			LevelWad.layoutClasses[1].mobys[index].position.z -= 4000;

			// Earthshapers
			vector<int> indices = LevelWad.layoutClasses[2].GetMobyIndexFromClass(492);
			for (int i = 0; i < indices.size(); i++) {
				LevelWad.layoutClasses[2].DeleteMoby(indices[i]);
			}

			// Delete Agent 9 - check that Spyro spawns in the desired spot
			index = LevelWad.layoutClasses[2].GetMobyIndexFromClass(99)[0];
			LevelWad.layoutClasses[2].DeleteMoby(index);

			// Move the sublevel entrances
			// int skillPointIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(947)[0]; // Skill point box
			indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1022);
			for (int i = 0; i < indices.size(); i++) {
				Moby* moby = &(LevelWad.layoutClasses[0].mobys[indices[i]]);
				Moby1022Tag* tag = (Moby1022Tag*)(LevelWad.layoutClasses[0].layoutData + moby->mobyTag);

				/*
				cout << "---" << endl;
				cout << "Moby " << indices[i] << " - from " << tag->fromArea << " to " << tag->toArea << endl;
				cout << "Box 1 - " << tag->box.pos.x << ", " << tag->box.pos.y << ", " << tag->box.pos.z << endl;
				cout << "Box 2 - " << tag->unknown.pos.x << ", " << tag->unknown.pos.y << ", " << tag->unknown.pos.z << endl;
				cout << "Difference - " << tag->box.pos.x - tag->unknown.pos.x << ", " << tag->box.pos.y - tag->unknown.pos.y << ", " << tag->box.pos.z - tag->unknown.pos.z << endl;
				cout << "LevelSectorIndex - " << (int)moby->levelSectorIndex << endl;
				*/

				if (tag->fromArea == 0 && tag->toArea == 1) { // Tanks entrance
					//cout << "Moby " << indices[i] << " is the Tanks entrance" << endl;
					int visualIndex = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(3, tag->box.pos);
					PosInfo pos = { { 73754, 36609, 20193 }, { 0, 0x40, 0 } };
					LevelWad.layoutClasses[0].UpdateMobyPosition(visualIndex, pos);
					tag->box.pos = pos.pos;
					tag->box.widthX = 2000;
					tag->box.widthY = 2000;
					tag->entranceZWidth = 1500;
					tag->musicZWidth = 1500;
					moby->position = tag->box.pos;
				}
				else if (tag->fromArea == 0 && tag->toArea == 2) { // A9 entrance
					//cout << "Moby " << indices[i] << " is the A9 entrance" << endl;
					tag->box.pos = { 25705, 63856, 15519 };
					tag->box.widthX = 1500;
					tag->box.widthY = 1500;
					tag->unknown.pos = { 66971, 55716, 26590 };
					tag->box.angleZ = 0;
					tag->entranceZWidth = 2000;
					tag->musicZWidth = 2000;
					moby->position = tag->box.pos;
				}
				else if (tag->fromArea == 2 && tag->toArea == 0) {
					//moby->position = { 66971, 55716, 26590 }; // Experimental, not sure if this is where it spits you out
					//tag->box.pos = moby->position;
					//tag->unknown.pos = moby->position;
				}
			}

			// Area 2 exit - old method
			/*
			indices = LevelWad.layoutClasses[2].GetMobyIndexFromClass(1022);
			for (int i = 0; i < indices.size(); i++) {
				Moby* moby = &(LevelWad.layoutClasses[2].mobys[indices[i]]);
				Moby1022Tag* tag = (Moby1022Tag*)(LevelWad.layoutClasses[2].layoutData + moby->mobyTag);
				// cout << "From " << tag->fromArea << " to " << tag->toArea << endl;
				if (tag->fromArea == 0 && tag->toArea == 2) { // Area 2 exit spawn in area 0! Confusing labels
					moby->position = { 69711, 58388, 26680 }; // Experimental, not sure if this is where it spits you out
					tag->box.pos = moby->position;
					tag->unknown.pos = moby->position;
					cout << "DEBUG - REMOVE LATER - experimentally moved a moby 1022 in Level 43 Area 2!" << endl;
					// This is crude - area exit no longer works (maybe due to one of the movements?) so this is one to watch out for
					// Alternatively, get the one that is TO AREA 0 and find its linked moby - this would be more accurate
				}
			}
			*/

			// Area 2 exit - note that currently UpdateSublevelExit doesn't update the box.pos or unknown.pos associated with exits, so test this
			position = { { 67346, 56215, 25813 }, { 0, 0, 0x9C } };
			LevelWad.UpdateSublevelExit(2, 0, position);

			// Dan 487 0 n
			MoveDialogueStringBackwards(&LevelWad, 0, 487, 0, 1, 28);
			string msg = "To find the treasures that you seek, you must unravel the mysteries of these catacombs completely. On this journey, a skillful explorer will never be unrewarded. I can help you only by delivering this clue...";
			UpdateDialogue(&LevelWad, 0, 487, 0, 0, msg);
			msg = "A dragon sleeps within these walls. To reach it, you must ask your first servant to bow to you, then ride on his shoulders as he stands tall again.";
			UpdateDialogue(&LevelWad, 0, 487, 0, 1, msg);

			// Ali 43-2 487 1 n
			MoveDialogueStringBackwards(&LevelWad, 2, 487, 1, 1, 180);
			MoveDialogueStringBackwards(&LevelWad, 2, 487, 1, 2, 16);
			MoveDialogueStringBackwards(&LevelWad, 2, 487, 1, 3, 4);

			msg = "Would you like to hear a riddle?";
			UpdateDialogue(&LevelWad, 2, 487, 1, 0, msg);
			msg = "I often house a dying beauty, cut freshly for decor.\ Resist the treasure that I've swallowed and you might find something more.\ For my body stands against a wall, spaced enough for you to fit.\ And if you squeeze within that space, I might send you up a bit.\ &What am I? &a mummy &a sarcophagus &bees? &a vase";
			UpdateDialogue(&LevelWad, 2, 487, 1, 1, msg);
			msg = "My, my, I didn't think you'd get that one! I wonder if there's a vase like that somewhere in these catacombs... Hmm...";
			UpdateDialogue(&LevelWad, 2, 487, 1, 2, msg);
			msg = "I'm sorry, my reptilian friend, but that was not even close. Perhaps you would prefer to go play with a nice bouncy ball.";
			UpdateDialogue(&LevelWad, 2, 487, 1, 3, msg);

			UpdateVram("resources/43.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/43.texture", LevelData.block0Pointer);
			UpdateLighting("resources/43.lighting", LevelData.block1Pointer);
			// Textures are a bit corrupted and I don't know why, oh well

			// Deprecated - area 0 scene doesn't have a vram block
			//auto scene = SceneData(0, (unsigned char*)LevelWad.sceneDataBlocks[0], LevelWad.sceneHeaders[0].length);
			//scene.UpdateVram("resources/43.vram");
			//LevelWad.UpdateScene(0);

		}
		else if (levelNo == 31) {

			// Update state of shaft egg, now in area 3
			vector<int> indices = LevelWad.layoutClasses[3].GetMobyIndexFromClass(132);
			for (int i = 0; i < indices.size(); i++) {
				LevelWad.layoutClasses[3].mobys[indices[i]].state = 0;
			}

			// Disable area 2 NPC type 1
			int npcIndex = LevelWad.layoutClasses[2].GetMobyIndexFromNpcType(756, 1);
			LevelWad.layoutClasses[2].DeleteMoby(npcIndex);

			// Disable area 3 NPC type 2
			npcIndex = LevelWad.layoutClasses[3].GetMobyIndexFromClass(594)[0];
			LevelWad.layoutClasses[3].DeleteMoby(npcIndex); // This is crude - maybe I just need to put him in an uninteractable state instead

			// Gems
			vector<Vector3D> positions = {
				{ 63242, 84850, 45191 },
				{ 65351, 87459, 43653 },
				{ 66459, 84304, 45447 },
				{ 52272, 50037, 38869 }
			};
			LevelWad.layoutClasses[3].UpdateMobyPositionsByClass(1, positions);
			vector<PosInfo> posInfos = {
				{ { 76538, 88060, 43317 }, { 0x00, 0xEA, 0x41 } },
				{ { 78984, 87851, 43442 }, { 0xFF, 0xEA, 0x40 } }
			};
			LevelWad.layoutClasses[3].UpdateMobyPositionsByClass(200, posInfos);
			positions = {
				{ 62500, 94666, 27307 },
				{ 67076, 94682, 27292 },
				{ 71665, 94698, 27291 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);
			positions = {
				{ 63707, 52284, 22134 },
				{ 65826, 52041, 22134 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(200, positions);

			// Copy "Beneath knocking footsteps." (dragon 2)
			int index = LevelWad.layoutClasses[0].GetEggIndexByNumber(2);
			PosInfo posInfo = { { 69654, 74085, 17644 }, { 0, 0, 0x3D } };
			LevelWad.layoutClasses[0].DuplicateMoby(index, posInfo);

			// Sign
			PosInfo position = { { 104510, 39636, 157152 }, { 0, 0, 0x3D } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(685, { position });
			string msg = "There's nothing in here. Look somewhere else!";
			UpdateDialogue(&LevelWad, 0, 685, 140, 0, msg);
			UpdateDialogue(&LevelWad, 0, 685, 140, 1, msg);
			UpdateDialogue(&LevelWad, 0, 685, 140, 2, msg);

			UpdateSkybox("resources/44.sky", LevelData.skyboxPointer);
			UpdateLighting("resources/44.lighting", LevelData.block1Pointer);

		}
		else if (levelNo == 32) {
			
			// Sparx - change message 2 to point at message 0 instead (experimental)
			int sparxIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(144)[0]; // Speedway Sparx is 144
			char* sparxTag = LevelWad.layoutClasses[0].GetTagPointer(sparxIndex);
			int* dialoguePointers = (int*)(sparxTag + 16); // 0xC is name, 0x10 is msg 0
			dialoguePointers[2] = dialoguePointers[0];

			// Sparx - change msg 0 string
			char* messagePointer = LevelWad.layoutClasses[0].layoutData + dialoguePointers[0];
			messagePointer += *messagePointer; // Pass over dialogue header
			const char* newMessage = "Which challenge would you like to play? &Choose a challenge: &Time Attack &Repeat that? &Exit Level"; // Shorter than original
			memcpy(messagePointer,(char*)newMessage,strlen(newMessage));
			*(messagePointer + strlen(newMessage)) = 0; // Null-terminate

			// Hunter - disable
			int hunterIndex = LevelWad.layoutClasses[0].GetMobyIndexFromClass(12)[0];
			LevelWad.layoutClasses[0].DeleteMoby(hunterIndex); // This is crude - maybe I just need to put him in an uninteractable state instead
			
		}
		else if (levelNo == 33) {
			
			// Update state of eggs
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(132);
			for (int i = 0; i < indices.size(); i++) {
				LevelWad.layoutClasses[0].mobys[indices[i]].state = 0;
				if (i == 0) { // First index will be of the egg already placed
					LevelWad.layoutClasses[0].mobys[indices[i]].angle.z = 0xFF; // Cheeky - it's because we're not changing the position of this one
				}
			}

			// Gems
			vector<Vector3D> positions = {
				{ 112644, 136292, 15750 },
				{ 111711, 139962, 15750 },
				{ 108726, 142536, 15750 },
				{ 105099, 145452, 15750 },
				{  99935, 147960, 15750 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);

			// Trees (184)
			positions = {
				{ 102174, 89542, 16346 },
				{ 101668, 88525, 16347 },
				{ 100871, 87673, 16347 },
				{ 102227, 81848, 16119 },
				{ 101389, 82856, 16120 },
				{ 100799, 83663, 16130 },
				{  99731, 84833, 16118 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(184, positions);

			// Boats (637)
			positions = {
				{ 112661,  53318, 15360 },
				{  72019,  93366, 15360 },
				{  69625, 129332, 15360 },
				{  98885, 152437, 15360 },
				{ 117757,  85701, 15360 },
				{ 156568,  75974, 15360 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(637, positions);

			// Armoured chest
			int index = LevelWad.layoutClasses[0].GetClosestMobyIndexFromClass(203, { 106598, 83251, 16384 });
			//LevelWad.layoutClasses[0].DeleteMoby(index); // deprecated

			// Experimental - move bomb spawner
			PosInfo posInfo = { { 92859, 121764, 16342 }, { 0, 0, 0x5A } };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(227, { posInfo });

			// Agent 9 model
			ModelHeader* monkey = LevelWad.GetModelPointer(0, 99);
			ModelAnimHeader* anim0 = (ModelAnimHeader*)((char*)monkey + monkey->animations[0]);
			RGB* colours = (RGB*)((char*)monkey + monkey->faces + anim0->colours);
			// cout << "Red in colours 1 and 2: " << hex << (int)colours[0].R << "; " << hex << (int)colours[1].R << endl;

			int whites = 0;
			int whiteChanges[] = {
				0x007584A6, // Upper-inner eye ridge   - 53, R255 G244 B220
				0x00707892, // Centre of eye           - 54, R255 G252 B239
				0x00787888, // Upper-outer eye corner  - 55, R198 G201 B198
				0x0068789B, // Lower-inner eye corner  - 70, R236 G228 B215
				0x007D7575, // Lower-outer eye ridge   - 76, R192 G197 B194
				0x00000000, // Gun - DO NOT CHANGE     - 114, R177 G186 B215
				0x00808080  // ??? - DO NOT CHANGE     - 115, R199 G216 B254
			};

			for (int i = 0; i < anim0->colourCount; i++) {
				if (colours[i].R > 0xB0 && colours[i].G > 0xB0 && colours[i].B > 0xB0) {
					// cout << "Colour " << dec << i << " seems to be quite white (R" << (int)colours[i].R << " G" << (int)colours[i].G << " B" << (int)colours[i].B << ")" << endl;
					if (whites < 5) {
						*(int*)(colours + i) = whiteChanges[whites];
						whites++;
					}
					else {
						// Do nothing
						// cout << "WARNING - not enough colours given" << endl;
					}
				}
				else if (colours[i].R < 0xB0 && colours[i].B < 0xB0 && colours[i].G > colours[i].R && colours[i].G > colours[i].B && colours[i].G > 0x38) {
					// cout << "Colour " << dec << i << " seems to be quite green (R" << (int)colours[i].R << " G" << (int)colours[i].G << " B" << (int)colours[i].B << ")" << endl;
					colours[i].R = min(255, colours[i].G * 1.3);
					colours[i].B = min(255, colours[i].B * 1.3);
				}
				else if (colours[i].G < 0x78 && colours[i].B < 0x78 && colours[i].R > colours[i].G && colours[i].R > colours[i].B && colours[i].R > 0x40) {
					// cout << "Colour " << dec << i << " seems to be quite red (R" << (int)colours[i].R << " G" << (int)colours[i].G << " B" << (int)colours[i].B << ")" << endl;
					colours[i].G = min(255, colours[i].G * 1.2);
					colours[i].B = min(255, colours[i].G * 1.05);
				}
				else if (colours[i].R < 0xB8 && colours[i].R > 0x78 && colours[i].R > colours[i].B && colours[i].R > colours[i].G && colours[i].B > 0x38 && colours[i].G > 0x38) {
					// 977866
					// cout << "Colour " << dec << i << " seems to be quite brown" << endl;
				}
			}
			LevelWad.UpdateScene(0);

			UpdateSkybox("resources/46.sky", LevelData.skyboxPointer);
			UpdateVram("resources/46.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/46.texture", LevelData.block0Pointer);
			UpdateLighting("resources/46.lighting", LevelData.block1Pointer);
		}
		else if (levelNo == 34) {

			// Special gem
			// AddSpecialGem(&LevelWad, 0, { 68919, 68953, 15317 }, false);
			// Nope - no gem code

			UpdateSkybox("resources/47.sky", LevelData.skyboxPointer);
			UpdateVram("resources/47.vram", LevelWad.vramDataBlock);
			UpdateBlock("resources/47.texture", LevelData.block0Pointer);
			UpdateLighting("resources/47.lighting", LevelData.block1Pointer);
		}
		else if (levelNo == 35) {

			// Delete Sparx
			int index = LevelWad.layoutClasses[0].GetMobyIndexFromClass(368)[0];
			LevelWad.layoutClasses[0].DeleteMoby(index);

			// End of level portal
			Vector3D position = { 64708, 29954, 8151 };
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(453, { position });

			// This level's layout is almost full and nearly collides with the dragon model
			// To be safe, I'm just going to move and revalue gems instead of using AddSpecialGem
			//cout << LevelWad.layoutClasses[0].GetMobyIndexFromClass(1).size() << " gems" << endl;

			// Gems
			vector<Vector3D> positions = {
				{ 64981,  73916,  8153 }, // 5
				{ 64916,  70765,  8153 }, // 5
				{ 67456,  73546,  8553 }, // 5
				{ 67132,  75591,  8151 }, // 10
				{ 63275,  71715,  8153 }, // 10
				{ 66059,  72347,  8152 }, // 10
				{ 93684, 102686,  8152 }, // 25
				{ 93711, 104804,  8149 }, // 25
				{ 48626, 117255, 14294 }, // 26
				{ 95458,  68124, 12207 }, // 26
				{ 93708, 120302, 14295 }, // 26
				{ 26122,  69371,  8152 }, // 26
				{ 28100,  43936,  8152 }, // 26
				{ 47441,  31083, 10711 }, // 26
				{ 62926,  15739, 10714 }, // 26
				{ 78456,  31079, 10714 }, // 26
				{ 95458,  68124,  8152 }  // 27
			};
			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(1);
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(1, positions);
			for (int i = 0; i < positions.size(); i++) { // NOT indices.size() !!!
				if (i < 3) {
					LevelWad.layoutClasses[0].mobys[indices[i]].levelSectorIndex = 5;
				}
				else if (i < 6) {
					LevelWad.layoutClasses[0].mobys[indices[i]].levelSectorIndex = 10;
				}
				else if (i < 8) {
					LevelWad.layoutClasses[0].mobys[indices[i]].levelSectorIndex = 25;
				}
				else if (i > 15) {
					LevelWad.layoutClasses[0].mobys[indices[i]].levelSectorIndex = 27;
				}
				else {
					LevelWad.layoutClasses[0].mobys[indices[i]].levelSectorIndex = 26;
				}
			}

			// Special gem
			//AddSpecialGem(&LevelWad, 0, { 48626, 117255, 14294 }, false);

		}
		else if (levelNo == 36) {

			for (int i = 0; i < 8; i++) {
				LevelWad.layoutClasses[0].DuplicateMoby(202, 0);
			}

			// Headbash crates
			/* deprecated
			vector<PosInfo> positions = {
				{ { 63046, 121624, 23025 }, { 0x40, 0, 0x19 } },
				{ { 62116, 121970, 23025 }, { 0x40, 0, 0x19 } },
				{ { 62797, 122376, 23025 }, { 0x40, 0, 0x99 } },
				{ { 61512, 111017, 23465 }, { 0, 0, 0 } }
			};
			*/
			vector<Vector3D> positions = {
				{ 87704,  74756, 18042 }, // default position { 68301, 114852, 21505 }
				{ 61512, 111017, 23465 }, // default position { 117617, 98488, 18079 }
				{ 62563, 121356, 22925 },
				{ 63301, 121803, 22925 },
				{ 62697, 122447, 22925 },
				{ 62078, 122102, 22925 },
				{ 62563, 121356, 23465 },
				{ 63301, 121803, 23465 },
				{ 62697, 122447, 23465 },
				{ 62078, 122102, 23465 }
			};
			LevelWad.layoutClasses[0].UpdateMobyPositionsByClass(202, positions);

			vector<int> indices = LevelWad.layoutClasses[0].GetMobyIndexFromClass(948);
			for (int i = 0; i < indices.size(); i++) {
				int* tag = (int*)(LevelWad.layoutClasses[0].GetTagPointer(indices[i]));
				switch (*tag) {
				case 16000:
				case 17000:
					*tag = 5000;
					//cout << "DEBUG - UPDATED SBR REQS" << endl;
					break;
				case 18500:
					*tag = 6000;
					//cout << "DEBUG - UPDATED SBR REQS" << endl;
					break;
				case 20000:
					*tag = 7000;
					//cout << "DEBUG - UPDATED SBR REQS" << endl;
					break;
				default:
					cout << "ERROR - could not update SBR door toll." << endl;
					break;
				}
			}

		}

		if (!explorationMode) {
			UpdateVram((short*)LevelWad.vramDataBlock, "resources/pete.vram", 0x1c1, 0x101, 15, 24);
		}
		else {
			UpdateVram((short*)LevelWad.vramDataBlock, "resources/cyprin.vram", 0x1c1, 0x101, 15, 24);
		}

		LevelWad.UpdateLayouts();
		LevelWad.UpdateData();
		LevelWad.UpdateVram();

	}
	/*
	printf("\nSample Egg: ");
	for (int i = 0; i < 0x58; i++) {
		cout << (int)(((unsigned char*)&sampleEgg)[i]) << " ";
	}
	printf("\nSample Tag: ");
	for (int i = 0; i < sampleEggTag.size(); i++) {
		cout << (int)(sampleEggTag[i]) << " ";
	}
	cout << endl;
	*/

	return;
}

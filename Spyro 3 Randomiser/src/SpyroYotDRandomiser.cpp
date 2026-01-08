#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <windows.h>
//#include <mmsystem.h>
//#include <cstdlib>
#include "types.cpp"
#include "header.h"
#include "resource.h"

using namespace std;

char musicArraySep14[] = { 0x04, 0x0E, 0x13, 0x11, 0x07, 0x1B, 0x08, 0x0D, 0x12, 0x0A, 0x17, 0x1A, 0x15, 0x01, 0x1B, 0x02, 0x06, 0x12, 0x04, 0x19, 0x14, 0x09, 0x10, 0x1B, 0x1C, 0x0C, 0x12, 0x0F, 0x07, 0x1A, 0x13, 0x11, 0x1B, 0x16, 0x04, 0x12, 0x14, 0x04, 0x04, 0x04 };
char musicArraySep29[] = { 0x04, 0x0C, 0x13, 0x11, 0x08, 0x19, 0x06, 0x0E, 0x14, 0x0A, 0x15, 0x1A, 0x17, 0x01, 0x19, 0x02, 0x09, 0x14, 0x04, 0x1B, 0x12, 0x07, 0x10, 0x19, 0x1C, 0x0D, 0x14, 0x0F, 0x08, 0x1A, 0x13, 0x11, 0x19, 0x16, 0x09, 0x14, 0x12, 0x04, 0x04, 0x04 };
// musicArrayOct24 matches Sep29
char musicArrayOct31[] = { 0x04, 0x11, 0x1A, 0x17, 0x08, 0x06, 0x09, 0x10, 0x18, 0x0B, 0x1E, 0x0D, 0x1C, 0x01, 0x0E, 0x02, 0x07, 0x18, 0x1F, 0x20, 0x24, 0x0A, 0x16, 0x13, 0x23, 0x0F, 0x18, 0x12, 0x14, 0x21, 0x15, 0x19, 0x22, 0x1D, 0x05, 0x18, 0x1B, 0x04, 0x04, 0x04 };

char* musicArrays[] = {
  &(musicArraySep14[0]),
  &(musicArraySep29[0]),
  &(musicArraySep29[0]), // Deliberate repeat
  &(musicArrayOct31[0])
};

unsigned int addressOffset = 0x8000F800;
unsigned int overlayOffsets[4] = { 0x800742d0, 0x8007778c, 0x8007778c, 0x800743b0 };
unsigned int endOfText[4] = { 0x80064f9c, 0x80067430, 0x80067430, 0x80065034 };
unsigned int sdataLocs[4] = { 0x8006c3b0, 0x8006f53c, 0x8006f53c, 0x8006c490 };
unsigned int exeChecksumStarts[4] = { 0x8001200e, 0x80013fc6, 0x80013fc6, 0x80012022 };

string version = "1.2.0-dev";
int gameVersion = -1;
bool hardMode = false;
char tab = 0x09;

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
Wad wadHeader; // Best to declare this here as it can be used in lots of places
unsigned char levelIds[] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 20, 21, 22, 23, 24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38, 40, 41, 42, 43, 44, 45, 46, 47, 48, 50 };

int highestEgg = 1;
unsigned int checksumValues[37] = { 0 };
bool collectMode = true;

void loadFileInResource(int name, int type, DWORD& size, const char*& data) // *& is passing the pointer by reference and not by val.
{
	HMODULE handle = ::GetModuleHandle(NULL);
	HRSRC rc = ::FindResource(handle, MAKEINTRESOURCEW(name), MAKEINTRESOURCEW(type));
	HGLOBAL rcData = ::LoadResource(handle, rc);
	size = ::SizeofResource(handle, rc);
	data = static_cast<const char*>(::LockResource(rcData));
	//LockResource does not actually lock memory; it is just used to obtain a pointer to the memory containing the resource data. 
	//return;
}

const char* getResource()
{
	DWORD size = 0;
	const char* data = NULL;
	loadFileInResource(IDR_TEXTFILE1, 256, size, data);
	/* Access bytes in data - here's a simple example involving text output*/
	// The text stored in the resource might not be NULL terminated.
	wchar_t* buffer = new wchar_t[size + 1];
	::memcpy(buffer, data, size);
	buffer[size] = 0; // NULL terminator
	delete[] buffer;
	return  data;
}

void outputEggInfo(char* eggInfoBlock, int levelNo, int eggNumber) {
	EggInfo* eggInfo = (EggInfo*)(eggInfoBlock + 4);
	ofstream eggFile;
	eggFile.open("eggInfo.txt", ios::binary | ios::app); // Update this
	eggFile.unsetf(ios::skipws);
	eggFile << "Level " << (int)(levelIds[levelNo]) << endl << "------" << endl;
	for (int i = 0; i < eggNumber; i++) {
		// Print name
		int offset = eggInfo[i].offset;
		while (*(eggInfoBlock + offset) != 0 && offset < *(int*)eggInfoBlock) {
			eggFile << *(eggInfoBlock + offset);
			offset++;
		}
		eggFile.write(&tab, 1);
		eggFile << (int)*((char*)&(eggInfo[i].unk1) + 3); // There's 3 0x80s first
		eggFile.write(&tab, 1);
		eggFile << (int)*((char*)&(eggInfo[i].unk2) + 3) << endl; // There's 3 0x80s first
	}
	eggFile << endl;
	eggFile.close();
	return;
}

void decryptOverlaysToFile(const char* wadPath) {

	fstream openWad(wadPath, ios_base::in | ios_base::out | ios_base::binary);
	unsigned int overlayOffset = overlayOffsets[gameVersion];

	for (int i = 0; i < 37; i++) {

		openWad.seekg(wadHeader.entries[98 + 2 * i].offset);
		char* ovl = (char*)malloc(wadHeader.entries[98 + 2 * i].length);
		openWad.read(ovl, wadHeader.entries[98 + 2 * i].length);

		decryptOverlay(ovl, wadHeader.entries[98 + 2 * i].length);

		string folder = ".\\overlays-" + to_string(gameVersion);
		CreateDirectoryA(folder.c_str(), NULL);
		string filename = folder + "\\" + to_string(i) + ".wad";
		ofstream openOvl;
		openOvl.open(filename, ios::binary | ios::app);
		openOvl.unsetf(ios::skipws);
		if (!openOvl) {
			cout << "Error writing to overlay." << endl;
			continue;
		}

openOvl.write(ovl, wadHeader.entries[98 + 2 * i].length);
free(ovl);
openOvl.close();

	}

	openWad.close();

	return;
}

bool fileExists(const char* fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

void error(int errorCode) {

	PlaySound(NULL, NULL, 0);
	PlaySound(MAKEINTRESOURCE(IDR_WAVE2), GetModuleHandle(NULL), SND_RESOURCE | /*SND_LOOP | */SND_ASYNC);

	const char* errorMsgs[] = {
		"ERROR: No WAD file detected. (Press ENTER to exit)\n",
		"ERROR: Invalid WAD file detected. (Press ENTER to exit)\n",
		"ERROR: No EXE file detected. (Press ENTER to exit)\n",
		"ERROR: Invalid EXE file detected. (Press ENTER to exit)\n",
		"ERROR: Invalid WAD-EXE combination detected. (Press ENTER to exit)\n",
		"ERROR: No file paths given. Please provide a path to the CD image file you wish to edit. (Press ENTER to exit)\n",
		"ERROR: Invalid file path given. Please provide a path to the CD image file you wish to edit. (Press ENTER to exit)\n",
		"ERROR: mkpsxiso.exe not present. Please ensure a copy of this executable is present in the working directory. (Press ENTER to exit)\n",
		"ERROR: dumpsxiso.exe not present. Please ensure a copy of this executable is present in the working directory. (Press ENTER to exit)\n",
		"ERROR: dump failed, WAD not found. This may mean that dumpsxiso failed to obtain the files. Try reformatting the file path string. (Press ENTER to exit)\n",
		"ERROR: dump failed, EXE not found. This may mean that dumpsxiso failed to obtain the files. Try reformatting the file path string. (Press ENTER to exit)\n",
		"ERROR: PAL version detected. PAL versions are currently not supported."
	};

	printf(errorMsgs[errorCode]);

	getchar();
	exit(1);
}

bool confirmValidity(char* wad, char* exe) {

	// Read WAD header, confirm WAD format
	ifstream openWad(wad, ios::binary);
	openWad.unsetf(ios::skipws);
	if (!openWad) {
		error(0);
	}
	streampos fileSize;
	openWad.seekg(0, ios::end);
	fileSize = openWad.tellg();
	openWad.seekg(0, ios::beg);
	if (fileSize < 0x800) {
		error(1);
	}
	openWad.read((char*)&wadHeader, sizeof(Wad));
	for (int i = 0; i < 256; i++) {
		WadEntry currentEntry = wadHeader.entries[i];
		if (currentEntry.offset + currentEntry.length > fileSize) {
			error(1);
		}
		if (currentEntry.offset == 0 && currentEntry.length == 0) {
			break;
		}
	}
	int wadCrc = getCRC((unsigned char*)&wadHeader, 0x800);
#ifdef _DEBUG
	printf("WAD CRC: 0x%08x\n", wadCrc); // Debug
#endif

	// PAL thing
	unsigned int palCreditsEditOffset = wadHeader.entries[187].offset + 0x10006c; // Only applicable for PAL; for Sep14 it's 183.wad + 0x100b3f
	char editedCharacter;
	openWad.seekg(palCreditsEditOffset);
	openWad.read((char*)&editedCharacter,1);
	bool palWadEdited = (editedCharacter != 0x77); // "w"

	openWad.close();

	// Read EXE, confirm EXE format
	ifstream openExe(exe, ios::binary);
	openExe.unsetf(ios::skipws);
	if (!openExe) {
		error(2);
	}
	streampos exeSize;
	openExe.seekg(0, ios::end);
	exeSize = openExe.tellg();
	openExe.seekg(0, ios::beg);
	if (exeSize < 0x10) {
		error(3);
	}
	char* exeBuffer = (char*)malloc(exeSize);
	openExe.read(exeBuffer, exeSize);
	ULONG magicNumber = 0x45584520582D5350;
	if (*(ULONG*)exeBuffer != magicNumber) {
		error(3);
	}
	int exeCrc = getCRC((unsigned char*)exeBuffer, exeSize);
#ifdef _DEBUG
	printf("EXE CRC: 0x%08x\n", exeCrc); // Debug
#endif
	openExe.close();

	// Version Checking - this makes some of the earlier stuff redundant! But oh well
	if (wadCrc != 0x5886ec7d && wadCrc != 0x557c6b74 && wadCrc != 0x85b1439f && wadCrc != 0x008aeda8) {
		printf("ERROR: Unrecognised WAD file detected (CRC: 0x%08x). (Press ENTER to exit)\n", wadCrc);
		getchar();
		exit(1);
	}
	if ((wadCrc == 0x557c6b74 || wadCrc == 0x85b1439f) && palWadEdited) {
		goto versionError;
	}
	/*
	if (exeCrc != 0x02c69f63 && (exeCrc != 0x6ece0846 && exeCrc != 0xa62fb184) && (exeCrc != 0xf58484c6 && exeCrc != 0x50adb608) && exeCrc != 0x5d7ff2fa) {
		printf("ERROR: Unrecognised EXE file detected (CRC: 0x%08x). (Press ENTER to exit)\n", exeCrc);
		getchar();
		exit(1);
	}
	*/
	if (wadCrc == 0x5886ec7d && exeCrc == 0x02c69f63) {
		gameVersion = 0; // Sep14
	}
	else if (wadCrc == 0x557c6b74) { // Exe spat out by dumpsxiso was different every time
		gameVersion = 1; // Sep29
		error(11);
	}
	else if (wadCrc == 0x85b1439f) {
		gameVersion = 2; // Oct24
		error(11);
	}
	else if (wadCrc == 0x008aeda8 && exeCrc == 0x5d7ff2fa) {
		gameVersion = 3; // Oct31
	}
	else {
		versionError:
		error(4);
	}
#ifdef _DEBUG
	cout << "Game version " << gameVersion << " detected" << endl;
#endif
	return true;
}

void updateCredits(char* wad) {
	printf("Updating credits...\n");

	int creditsOffsets[4] = { 0x098AC32C, 0x0AAFB059, 0x0AAFB859, 0x098CCB2C };

	stringstream sstream;
	sstream << "0x" << setw(8) << setfill('0') << hex << seed;
	string tmp = sstream.str();
	const char* seedString = tmp.c_str();
	const char* strings[] = { "Seed ", "Randomiser by Hwd405" };

	char block[42] = { 0 };
	memcpy(block, strings[0], strlen(strings[0]));
	memcpy(block + strlen(strings[0]), seedString, strlen(seedString));
	memcpy(block + 19, strings[1], strlen(strings[1])); // block + 19 is the pointer to the last string

	// Open file
	fstream openWad(wad, ios_base::in | ios_base::out | ios_base::binary);

	// Write names to file
	openWad.seekp(creditsOffsets[gameVersion]);
	openWad.write(block, sizeof(block));

	return;
}

unsigned char getLevelNameLength(int index, int levelNo) {

	unsigned char len = (strlen(namesFirstHalf[index]) + strlen(namesSecondHalf[index]) + 4) & 252;

	if (levelNo % 9 == 0 && levelNo != 36) { // Homeworlds
		len = (unsigned char)((strlen(namesFirstHalf[index]) + strlen(namesSecondHalf[index]) + strlen(" Home") + 4) & 252);
	}
	else if (levelNo == 36) { // SBR
		len = (unsigned char)((strlen("Super Bonus ") + strlen(namesSecondHalf[index]) + 4) & 252);
	}
	else if (levelNo % 9 == 5) { // Speedways
		len = (unsigned char)((strlen(namesFirstHalf[index]) + strlen("Speedway") + 4) & 252);
	}
	else if (levelNo % 9 == 6) { // Critters
		len = (unsigned char)((strlen(namesFixed[(levelNo / 9) + 3]) + strlen(namesSecondHalf[index]) + 4) & 252);
	}
	else if (levelNo % 9 == 7) { // Boss Levels
		len = (unsigned char)((strlen(namesFixed[(levelNo / 9) + 7]) + strlen(namesSecondHalf[index]) + 4) & 252);
	}

	return len;
}

void randomiseLevelNames(char* exe) {

	printf("Randomising level names...\n");

	// Currently only the mixed names mode is (to be) implemented
	int maxLengths[4] = { 0x26C, 0x270, 0x270, 0x270 };
	int nameOffsets[4] = { 0x14CC, 0x20C8, 0x20C8, 0x14CC };
	int pointerBlockOffsets[4] = { 0x5A4F8, 0x5CD84, 0x5CD84, 0x5A5D8 };
	int bossPointerOffsets[4] = { 0x5A678, 0x5D3A4, 0x5D3A4, 0x5A758 };
	// 0x14CC + 80010000 - 800 = 0x80010CCC (addressOffset)
	// 0x5A4F8 + 80010000 - 800 = 0x80069CF8
	// Sep14 pointers: 80069cf8
	// Sep14 names: 80010ccc (4-padded, reverse order)

	shuffle(&namesFirstHalf[0], &namesFirstHalf[levelsLengthFh], std::default_random_engine(seed + 80));
	shuffle(&namesSecondHalf[0], &namesSecondHalf[levelsLengthSh], std::default_random_engine(seed + 90));

	int noOfGeneratedNames = min(levelsLengthFh, levelsLengthSh);

	LevelNameStruct levels[37];

	// Set the names and calculate the lengths
	int totalLength = 0;
	for (int i = 0; i < 37; i++) {
		levels[i] = { (unsigned char)i, getLevelNameLength(i, i) };
		totalLength += levels[i].length;
	}

	// Check total length and recalculate
	int newAttempts = 37;
	while (totalLength > maxLengths[gameVersion]) {
		// Find longest
		unsigned char longest = 0;
		unsigned char longestLen = 0;
		for (int i = 0; i < 37; i++) {
			if (levels[i].length > longestLen) {
				longest = i;
				longestLen = levels[i].length;
			}
		}
		while (newAttempts < noOfGeneratedNames && getLevelNameLength(newAttempts, longest) >= longestLen) {
			newAttempts++;
			if (newAttempts >= noOfGeneratedNames - 1) {
				break;
			}
		}
		levels[longest] = { (unsigned char)newAttempts, getLevelNameLength(newAttempts, longest) };
		if (newAttempts >= noOfGeneratedNames - 1) {
			levels[longest].length = 8; // "Uh Oh###"
		}
		totalLength = totalLength - longestLen + levels[longest].length;
		newAttempts++;
	}

	// Create the name block
	char* newLevels = (char*)malloc(maxLengths[gameVersion]);
	int pointerOffsets[37] = { 0 };

	char* reading = newLevels;
	char* nameReading;
	for (int i = 0; i < 37; i++) {
		pointerOffsets[36 - i] = (reading - newLevels);

		// Failed strings
		if (levels[36 - i].index >= noOfGeneratedNames - 1) {
			nameReading = (char*)(namesFixed[11]);
			for (int j = 0; j < strlen(nameReading); j++) {
				*reading = nameReading[j];
				reading++;
			}
			while ((reading - newLevels) % 4 != 0) {
				*reading = 0;
				reading++;
			}
			pointerOffsets[i] = (reading - newLevels);
			continue;
		}

		// First Half
		if (i == 0) { // SBR
			nameReading = (char *)(namesFixed[1]);
		}
		else if ((36 - i) % 9 == 6) {
			nameReading = (char*)(namesFixed[((36 - i) / 9) + 3]);
		}
		else if ((36 - i) % 9 == 7) {
			nameReading = (char*)(namesFixed[((36 - i) / 9) + 7]);
		}
		else {
			nameReading = (char*)(namesFirstHalf[levels[36 - i].index]);
		}
		for (int j = 0; j < strlen(nameReading); j++) {
			*reading = nameReading[j];
			reading++;
		}

		// Second Half
		if ((36 - i) % 9 == 5) {
			nameReading = (char*)(namesFixed[2]);
		}
		else {
			nameReading = (char*)(namesSecondHalf[levels[36 - i].index]);
		}
		for (int j = 0; j < strlen(nameReading); j++) {
			*reading = nameReading[j];
			reading++;
		}

		// Homeworlds
		if ((36 - i) % 9 == 0 && i != 0) {
			nameReading = (char*)(namesFixed[0]);
			for (int j = 0; j < strlen(nameReading); j++) {
				*reading = nameReading[j];
				reading++;
			}
		}

		// Correct length
		*reading = 0; // Ensure one more of these
		reading++;
		while ((reading - newLevels) % 4 != 0) {
			*reading = 0;
			reading++;
		}
	}
	while (reading < newLevels + maxLengths[gameVersion]) {
		*reading = 0;
		reading++;
	}

	// Create the pointer block
	char* pointerBlock = (char*)malloc(4 * 37);
	unsigned int bossPointers[4] = { 0 };
	for (int i = 0; i < 37; i++) {
		((int*)pointerBlock)[i] = pointerOffsets[i] + addressOffset + nameOffsets[gameVersion];
		if (i % 9 == 7) {
			bossPointers[(i / 9) % 4] = ((int*)pointerBlock)[i];
		}
	}

	// Open file
	fstream openExe(exe, ios_base::in | ios_base::out | ios_base::binary);
	
	// Write names to file
	openExe.seekp(nameOffsets[gameVersion]);
	openExe.write(newLevels, maxLengths[gameVersion]);
	
	// Write pointers to file
	openExe.seekp(pointerBlockOffsets[gameVersion]);
	openExe.write(pointerBlock, 4 * 37);
	for (int i = 0; i < 4; i++) {
		openExe.seekp(bossPointerOffsets[gameVersion] + 8 * i);
		openExe.write((char*)&(bossPointers[i]), 4);
	}

	// De-allocate all
	free(newLevels);
	free(pointerBlock);
	return;
}

void randomiseDragonNames(char* wad) {

	printf("Randomising dragon names...\n");

	shuffle(&names[0], &names[namesLength], std::default_random_engine(seed));

	int eggNumber = 0;

	fstream openWad(wad, ios_base::in | ios_base::out | ios_base::binary);
	for (int i = 0; i < 37; i++) {

		int wadOffset = wadHeader.entries[97 + 2 * i].offset;
		int wadSize = wadHeader.entries[97 + 2 * i].length;

		// Check layout data size is valid
		DataHeader dataHeader;

		openWad.seekg(0x8 + wadHeader.entries[97 + 2 * i].offset);
		openWad.read((char*)&dataHeader, 8);

		if (dataHeader.start + dataHeader.length /*End of data section*/ > wadHeader.entries[97 + 2 * i].length /*End of WAD*/) {
			printf("An error occurred opening WAD %d. Dragon names untouched.\n", 97 + 2 * i);
			continue;
		}

		// Load the whole block into memory
		openWad.seekg(wadHeader.entries[97 + 2 * i].offset + dataHeader.start);
		char* levelData = (char*)malloc(dataHeader.length);
		openWad.read(levelData, dataHeader.length);

		// Locate the egg info block
		char* currentlyReading = levelData;
		bool checksFailed = false;
		for (int j = 0; j < 4; j++) {
			// Confirm we're not jumping into anywhere we don't want to be
			if (currentlyReading + *(int*)currentlyReading >= levelData + dataHeader.length) {
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
			printf("An error occurred opening WAD %d. Dragon names untouched.\n", 97 + 2 * i);
			continue;
		}
		currentlyReading += 3*4;
		for (int j = 0; j < 4; j++) {
			// Confirm we're not jumping into anywhere we don't want to be
			if (currentlyReading + *(int*)currentlyReading >= levelData + dataHeader.length) {
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
			printf("An error occurred opening WAD %d. Dragon names untouched.\n", 97 + 2 * i);
			continue;
		}
		currentlyReading += 4;

		char* eggInfoBlock = currentlyReading;
		EggInfo* eggInfo = (EggInfo*)(currentlyReading + 4);

		// Determine egg count
		int eggCount = 0;
		char* nameReading = eggInfoBlock + (eggInfo[0].offset);
		// printf("Checking eggs in level %d.\n", levelIds[i]); // What if I just used a fucking array like a normal person?
		while (*(short*)nameReading != 0) { // When this is true we've reached the end of the list
			if (*(short*)nameReading >> 8 == 0) {
				eggCount++; // We've reached the last character in a name
			}
			nameReading++;
		}
		// printf("Eggs in level %d - %d.\n", levelIds[i], eggCount);

		// Debug functionality
		// outputEggInfo(eggInfoBlock, i, eggCount);

		// Overwrite the names
		//printf("Updating dragon names in level %d.\n", levelIds[i]);
		nameReading = eggInfoBlock + (eggInfo[0].offset);
		int length;
		for (int j = 0; j < eggCount; j++) {
			eggInfo[j].offset = nameReading - eggInfoBlock;
			length = strlen(names[eggNumber + j]);
			for (int k = 0; k < length; k++) {
				*nameReading = (names[eggNumber + j])[k];
				nameReading++;
			}
			if (i == 36) { // For SBR, concatenate two names
				*nameReading = 0x20;
				nameReading++;
				length = strlen(names[eggNumber + j + 1]); // This is fine, it's the last level so we won't get any weird repeated names
				for (int k = 0; k < length; k++) {
					*nameReading = (names[eggNumber + j + 1])[k];
					nameReading++;
				}
			}
			*nameReading = 0;
			nameReading++;
		}
		while (nameReading < eggInfoBlock + *(int*)eggInfoBlock) {
			*nameReading = 0;
			nameReading++;
		}

		// Correct the length
		eggNumber += eggCount; // Increases the number of eggs checked so far by the egg count of that level
		if (nameReading > eggInfoBlock + *(int*)eggInfoBlock) {
			*eggInfoBlock = ((int)nameReading - (int)eggInfoBlock + 4) & 252; // Remove the lowest two bits to get a multiple of 4
		}

		// Overwrite the file
		openWad.seekp(((int)eggInfoBlock - (int)levelData) /*offset inside level data block*/ + dataHeader.start /*data header position*/ + wadHeader.entries[97 + 2 * i].offset /*WAD offset*/);   // Move to write position
		openWad.write(eggInfoBlock, *(int*)eggInfoBlock);
		free(levelData);

	}
	
	// Debug functionality
	/*
	for (int i = 0; i < 20; i++) {
		cout << names[i] << endl;
	}
	*/
	
	openWad.close();
	return;
}

void randomise(char* wad, char* exe) {

	collectChecksums(wad, exe);

	randomiseMobyData(wad);
		// Small chance of swapping sublevels in each level perhaps?
	randomiseDragonNames(wad);
	//randomisePortalSkyboxes(wad); // Could do this in the Moby Data one
		// Could randomise their end locations too? But only levels 1-4 and never in Sunrise Spring
	randomiseLoadingSkyboxes(wad, exe);
		// Could randomise the skybox array and also use some reference files to randomise the skyboxes themselves
	updateCredits(wad);
		// Perhaps I can make a selection of weird BGR555 images and randomise those too lol	
		// Could also shuffle the LBAs for the dialogue lines in each level? That might be funny
	randomiseMoneybagsReqs(exe);
	randomiseMusic(exe);
	randomiseLevelNames(exe);
	randomiseTextColours(exe);
	randomiseSparxColours(exe);
	renameSpyro(exe);
	recolorSpyro(wad);

	runChecksumCorrectionProcess(wad, exe);

	return;
}

int main(int argc, char* argv[]) {

	HANDLE  hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, 11);
	//printf("\x1B[36mTextExample\033[0m\t\t");

	string debugString = "";

#ifdef _DEBUG
	debugString = " - DEBUG MODE";
#endif

	//const char* date = getResource();
	const char* date = __DATE__;
	const char* time = __TIME__;

	cout << "Spyro: Year of the Dragon Randomiser (Version " << version << " - " << date << " " << time << debugString << ")" << endl << "Written by Hwd405" << endl;
	cout << "Special thanks to Altro and Pepper" << endl;
	cout << "------------------------------------------------------------------------" << endl;
	
	SetConsoleTextAttribute(hConsole, 15);
	printf("A copy of Spyro 3 (e.g. bin/cue) is required to run this tool. Only final versions of the game are compatible.\n\n");

	printf("Press ENTER to continue.\n_\n");

	PlaySound(MAKEINTRESOURCE(IDR_WAVE5), GetModuleHandle(NULL), SND_RESOURCE | /*SND_LOOP | */SND_ASYNC);

	getchar();

	PlaySound(NULL, NULL, 0);
	PlaySound(MAKEINTRESOURCE(IDR_WAVE3), GetModuleHandle(NULL), SND_RESOURCE | /*SND_LOOP | */SND_ASYNC);

	SetConsoleTextAttribute(hConsole, 12);

	// Check there is at least two arguments (incl. the command)
	if (argc < 2) {
		error(5);
	}

	char* inputIso = argv[1];

	// Check that the file is present
	if (!fileExists(inputIso)) {
		error(6);
	}

	// If there's also a seed present, load it in
	if (argc > 2) {
		sscanf(argv[2], "%d", &seed);
	}
	hardMode = (seed % 1000 == 405);
	if (hardMode) {
#ifdef _DEBUG
		cout << "Hard mode active." << endl;
#endif
	}

	const char* mkpsxisoFiles[] = { "mkpsxiso.exe", "dumpsxiso.exe" };

	// Check that dumpsxiso and mkpsxiso are present
	if (!fileExists(mkpsxisoFiles[0])) {
		error(7);
	}
	else if (!fileExists(mkpsxisoFiles[1]))  {
		error(8);
	}

	// dumpsxiso
	SetConsoleTextAttribute(hConsole, 15);
	cout << "------------------------------------------------------------------------" << endl;
	printf("Running dumpsxiso.exe...\n");
	string iso = inputIso;
	string command = "dumpsxiso -x data -s buildConfig.xml \"" + iso + "\"";
	//command = "\"" + command + "\"";
	system(command.c_str());
	cout << "------------------------------------------------------------------------" << endl;

	SetConsoleTextAttribute(hConsole, 12);
	if (!fileExists("data/WAD.WAD")) {
		error(9);
	}
	const char* inputWad = "data/WAD.WAD";
	const char* inputExe[] = { "data/SCUS_944.67", "data/SCES_028.35" };
	char exeName = 0;

	if (!fileExists("data/SCUS_944.67")) {
		if (!fileExists("data/SCES_028.35")) {
			error(10);
		}
		exeName = 1;
	}

	// check validity
	if (confirmValidity((char *)inputWad, (char*)(inputExe[exeName]))) {

		//This appends - be warned
		//decryptOverlaysToFile(inputWad);

		PlaySound(NULL, NULL, 0);
		PlaySound(MAKEINTRESOURCE(IDR_WAVE4), GetModuleHandle(NULL), SND_RESOURCE | /*SND_LOOP | */SND_ASYNC);
		SetConsoleTextAttribute(hConsole, 14);
		randomise((char*)inputWad, (char*)(inputExe[exeName]));

		PlaySound(NULL, NULL, 0);
		PlaySound(MAKEINTRESOURCE(IDR_WAVE4), GetModuleHandle(NULL), SND_RESOURCE | /*SND_LOOP | */SND_ASYNC);
		SetConsoleTextAttribute(hConsole, 15);
		cout << "------------------------------------------------------------------------" << endl;
		printf("Running mkpsxiso.exe...\n");
		system("mkpsxiso -q -o Spyro3Randomised.bin -c Spyro3Randomised.cue -y buildConfig.xml");
		cout << "------------------------------------------------------------------------" << endl;

		PlaySound(NULL, NULL, 0);
		PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE | /*SND_LOOP | */SND_ASYNC);
		SetConsoleTextAttribute(hConsole, 10);
		printf("_\nRandomise complete. (Press ENTER to exit)");
		getchar();
		SetConsoleTextAttribute(hConsole, 15);
		return 0;
	}

	SetConsoleTextAttribute(hConsole, 12);
	printf("ERROR: Unknown error occurred. (Press ENTER to exit)");
	getchar();
	return 0;

}
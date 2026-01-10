#include "CommonHeader.h"

unsigned int addressOffset = 0x8000F800;
unsigned int overlayOffsets[2] = { 0x800742d0, 0x800743b0 };

unsigned int musicLbaAddresses[2] = { 0x800652b0, 0x80065348 };
unsigned int musicArrayOffsets[2] = { 0x80065734, 0x8006580c };

unsigned int flightLimitAddresses[2] = { 0x80067648, 0x80067728 };

unsigned int skyboxArrayAddresses[2] = { 0x8006668c, 0x80066760 };

char musicArraySep14[] = { 0x04, 0x0E, 0x13, 0x11, 0x07, 0x1B, 0x08, 0x0D, 0x12, 0x0A, 0x17, 0x1A, 0x15, 0x01, 0x1B, 0x02, 0x06, 0x12, 0x04, 0x19, 0x14, 0x09, 0x10, 0x1B, 0x1C, 0x0C, 0x12, 0x0F, 0x07, 0x1A, 0x13, 0x11, 0x1B, 0x16, 0x04, 0x12, 0x14, 0x04, 0x04, 0x04 };
char musicArraySep29[] = { 0x04, 0x0C, 0x13, 0x11, 0x08, 0x19, 0x06, 0x0E, 0x14, 0x0A, 0x15, 0x1A, 0x17, 0x01, 0x19, 0x02, 0x09, 0x14, 0x04, 0x1B, 0x12, 0x07, 0x10, 0x19, 0x1C, 0x0D, 0x14, 0x0F, 0x08, 0x1A, 0x13, 0x11, 0x19, 0x16, 0x09, 0x14, 0x12, 0x04, 0x04, 0x04 };
// musicArrayOct24 matches Sep29
char musicArrayOct31[] = { 0x04, 0x11, 0x1A, 0x17, 0x08, 0x06, 0x09, 0x10, 0x18, 0x0B, 0x1E, 0x0D, 0x1C, 0x01, 0x0E, 0x02, 0x07, 0x18, 0x1F, 0x20, 0x24, 0x0A, 0x16, 0x13, 0x23, 0x0F, 0x18, 0x12, 0x14, 0x21, 0x15, 0x19, 0x22, 0x1D, 0x05, 0x18, 0x1B, 0x04, 0x04, 0x04 };

char* musicArrays[] = {
  &(musicArraySep14[0]),
  &(musicArrayOct31[0])
};

char* exe;

void decryptOverlay(char* ovl, unsigned int ovlLength) {
	unsigned int overlayOffset = overlayOffsets[gameVersion];
	unsigned int key = *(int*)(ovl + 4);
	if ((ovlLength < key - overlayOffset) || ovlLength <= 0x1000) {
		// Key should be less than or equal to the overlay length
		cout << "Error decrypting overlay. Errors may result from this." << endl;
		return;
	}
	unsigned int currentlyReading = 0x1000;
	while (currentlyReading < (key - overlayOffset)) {
		*(int*)(ovl + currentlyReading) = *(int*)(ovl + currentlyReading) ^ key;
		currentlyReading = currentlyReading + 4;
	}
	return;
}

void UpdateLevelNames() {

	string newNames[] = {
		"Sunrise Spring",
		"Sunny Villa",
		"Stormy Spires",
		"Mad Monster Crater",
		"Calamity Beach",
		"Mushroom Flight",
		"Sunset Alps",
		"Buzz's Dungeon",
		"Crawdad Farm",
		"Frosty Garden",
		"Icy Peak",
		"Midnight Towers",
		"Swamp Party",
		"Tranquil Terrace",
		"Farm Flight",
		"Cpt. Byrd's Base",
		"Spike's Glacier",
		"Spider Town",
		"Sunset Lagoon",
		"Chicken Itza",
		"Frigid Fleet",
		"Ninja Peak",
		"Charmed Ridge",
		"Bee Keepers",
		"Bentley's Outpost",
		"Scorch's Pit",
		"Starfish Reef",
		"Moonlit Observatory",
		"Amethyst Isle",
		"Future Ruins",
		"Icy Catacombs",
		"Dry Gulch",
		"Harbor Flight",
		"Monkeywave",
		"Sorceress' Lair",
		"Bugbot Factory",
		"Super Bogus Round",
	};

	if (!stormyMode) {
		newNames[2] = "Cloud City";
	}

	// taking these from the randomiser - ew, they're file offsets
	int maxLengths[2] = { 0x26C, 0x270 };
	int nameOffsets[2] = { 0x14CC, 0x14CC };
	int pointerBlockOffsets[2] = { 0x5A4F8, 0x5A5D8 };
	int bossPointerOffsets[2] = { 0x5A678, 0x5A758 };

	char* names = exe + nameOffsets[gameVersion];
	char* nameBlock = (char*)calloc(maxLengths[gameVersion], 1); // Copy this into the Atlas when it's confirmed that the buffer doesn't overflow!
	unsigned int* pointers = (unsigned int*)(exe + pointerBlockOffsets[gameVersion]);
	unsigned int* pointerBlock = (unsigned int*)calloc(37 * 4, 1);
	char* writing = nameBlock;

	for (int i = 36; i >= 0; i--) {

		pointerBlock[i] = addressOffset + nameOffsets[gameVersion] + (writing - nameBlock);

		if (writing + newNames[i].size() + 1 /*null*/ > nameBlock + maxLengths[gameVersion]) {
			cout << "ERROR - could not update level names - block too long." << endl;
			return;
		}

		memcpy(writing, &((newNames[i])[0]), newNames[i].size());
		writing += newNames[i].size();
		*writing = 0;
		writing++;
		while ((int)(writing - nameBlock) % 4 != 0) {
			*writing = 0;
			writing++;
		}
	
	}

	// Copy both blocks in if this succeeded
	memcpy(pointers, pointerBlock, 37 * 4);
	memcpy(names, nameBlock, maxLengths[gameVersion]);

	// Boss name pointers
	unsigned int* bossPointers = (unsigned int*)(exe + bossPointerOffsets[gameVersion]);
	bossPointers[0] = pointers[0 * 9 + 7]; // 17
	bossPointers[1] = pointers[0 * 9 + 0];
	bossPointers[2] = pointers[1 * 9 + 7]; // 27
	bossPointers[3] = pointers[1 * 9 + 0];
	bossPointers[4] = pointers[2 * 9 + 7]; // 37
	bossPointers[5] = pointers[2 * 9 + 0];
	bossPointers[6] = pointers[3 * 9 + 7]; // 47
	bossPointers[7] = pointers[3 * 9 + 0];

	free(pointerBlock);
	free(nameBlock);

	return;
}

void UpdateSkyboxes() {

	char skyb1;
	char skyb2;
	char* skyboxArray = exe + (skyboxArrayAddresses[gameVersion] - addressOffset);

	// Switch Honey and Bentley
	skyb1 = skyboxArray[23];
	skyb2 = skyboxArray[24];
	skyboxArray[23] = skyb2;
	skyboxArray[24] = skyb1;

	return;
}

void UpdateLbas() {

	unsigned int base = ((gameVersion == 1) ? 282506 : 253783);
	//unsigned int base = 253783;
	int levelNos[] = { 35, 3, 5, 15, 19, 20, 29, 33 }; // Level nos to alter

	// -16 is pop control because nothing fucking works and MC32 constantly puts pops at the end of tracks
	// never fucking do XA audio editing it fucking sucks

	XaData xaData[] = {
		{ ( base + 0 ) - 90000 , 293927 - 253783 - 16 , 0 }, // 35 / 48
		{ ( base + 1 ) - 90000 , 297144 - 253784 - 16 , 1 }, //  3 / 13
		{ ( base + 2 ) - 90000 , 272129 - 253785 - 16 , 2 }, //  5 / 15 - other speedway levels untouched on rev 1 (?)
		{ ( base + 3 ) - 90000 , 280722 - 253786 - 16 , 3 }, // 15 / 26
		{ ( base + 4 ) - 90000 , 280547 - 253787 - 16 , 4 }, // 19 / 31
		{ ( base + 5 ) - 90000 , 283332 - 253788 - 16 , 5 }, // 20 / 32
		{ ( base + 6 ) - 90000 , 290669 - 253789 - 16 , 6 }, // 29 / 42
		{ ( base + 7 ) - 90000 , 298046 - 253790 - 16 , 7 }  // 33 / 46
	};

	char* musicArray = exe + (musicArrayOffsets[gameVersion] - addressOffset);
	if (gameVersion == 0) {
		musicArray[20] = 5; // Lost Fleet using Sorceress' Lair theme
		musicArray[3] = 24; // Molten Crater using Evening Lake Home theme
		musicArray[11] = 2; // Enchanted Towers using Sgt. Byrd's Base's theme
	}
	else if (gameVersion == 1) {
		musicArray[34] = 4; // Sorceress' Lair using Sunrise Spring's theme
	}

	XaData* lbaArray = (XaData*)(exe + (musicLbaAddresses[gameVersion] - addressOffset));

	/*
	for (int i = 0; i < 7; i++) {
		int index = (musicArrays[gameVersion])[levelNos[i]];
		if (levelNos[i] == 20 && gameVersion == 0) { // In Rev 0, LF should be changed to use Sorceress' Lair
			index = 5; // formerly 24 (Evening Lake Home)
		}
		lbaArray[index] = xaData[i];
	}
	*/

	for (int i = 0; i < 8; i++) {
		lbaArray[musicArray[levelNos[i]]] = xaData[i];
	}

	return;
}

void UpdateGems() {

	unsigned int valueToState[2] = { 0x80066964, 0x5723C + addressOffset }; // 28 bytes long
	// followed by values, animationIds, colours - all 8 bytes

	unsigned char* gemData = (unsigned char*)(exe + (valueToState[gameVersion] - addressOffset));

	// SectorToState // from 0 to 27 - reads the gem's sector, typically this matches the value but it doesn't have to
	gemData[26] = 5; // 50
	gemData[27] = 6; // 250
	gemData += 28;

	// StateToValue // from 0 to 128?
	gemData[5] = 50;
	gemData[6] = 250;
	gemData += 8;

	// StateToAnim // from 0-4
	gemData[5] = 0; // Like Spyro 1 blue gems baybee!!!
	gemData[6] = 1;
	gemData += 8;

	// StateToColour - 0 by default
	gemData[5] = 8; // from 0-8, normally 1-5
	gemData[6] = 6;
	gemData += 8;

	// Gem colours - four ints per colour
	gemData += 4 * 4 * 6; // skip over the first 6 colours
	*(int*)(gemData + 0) = 0x00002868;
	*(int*)(gemData + 4) = 0x000060FF;
	*(int*)(gemData + 8) = 0x00004070;
	*(int*)(gemData + 12) = 0x000080d0;

	return;
}

void UpdateFallLimits() {
	int* fallArray = (int*)(exe + (flightLimitAddresses[gameVersion] - addressOffset)) + 40; // 40 ints after the flight limits
	fallArray[4 * 10 + 0] = 1000; // Icy Peak
	fallArray[4 * 27 + 0] = 4000; // Midnight Mountain
	return;
}

void UpdateFlightLimits() {
	int* flightArray = (int*)(exe + (flightLimitAddresses[gameVersion] - addressOffset));
	flightArray[23] = 34202; // Honey Speedway
	flightArray[15] = 31000; // Sgt. Byrd's Base
	return;
}

void UpdateMoneybags() {

	int moneybagsOffsets[2] = { 0x576AC, 0x57784 };
	MoneybagsEntry* entries = (MoneybagsEntry*)(exe + moneybagsOffsets[gameVersion]);
	// Lvl12, Lvl23, Lvl10, Lvl21, Lvl13, Lvl34, Lvl20, Lvl30, Lvl42, Lvl40, Lvl31, Lvl41;

	entries[0].value = 100; // 12
	entries[1].value = 800; // 23
	entries[2].value = 300; // 10
	entries[3].value = 100; // 21
	entries[4].value = 150; // 13
	entries[5].value = 1500; // 34
	entries[6].value = 500; // 20
	entries[7].value = 600; // 30
	entries[8].value = 400; // 42
	entries[9].value = 900; // 40
	entries[10].value = 2000; // 31
	entries[11].value = 600; // 41

	return;

}

void UpdateGemTotals() {

	int totalsOffsets[2] = { 0x57754, 0x57834 };
	unsigned char* entries = (unsigned char*)(exe + totalsOffsets[gameVersion]);

	unsigned char newTotals[40] = {  5, 5, 5, 4, 5, 4, 4, 0, 2,
									 4, 6, 5, 5, 4, 4, 5, 0, 6,
									 4, 7, 6, 5, 6, 4, 6, 0, 2,
									 5, 7, 8, 8, 7, 4, 7, 0, 8,
									50, 0, 0, 0 };
	
	memcpy(entries, newTotals, 40);
	return;

}

void UpdateExe(char* exePath) {

	// Open file
	fstream openExe(exePath, ios_base::in | ios_base::out | ios_base::binary);
	streampos fileSize;
	openExe.seekg(0, ios::end);
	fileSize = openExe.tellg();
	openExe.seekg(0, ios::beg);

	exe = (char*)malloc(fileSize);
	openExe.read(exe, fileSize);

	if (musicFilesPresent) { // Only if the updated build configs are present
		UpdateLbas();
	}
	else {
		printf("Some files were not found to be present, and so the music has not been updated.\n");
	}

	// Update level names
	UpdateLevelNames();

	// Update flight limit array
	UpdateFlightLimits();

	// Update skybox array
	UpdateSkyboxes();

	// Update fall heights array
	UpdateFallLimits();

	// Update Moneybags array
	UpdateMoneybags();

	// Update gem totals array
	UpdateGemTotals();

	// Update gems
	UpdateGems();

	// Overwrite exe
	openExe.seekp(0);
	openExe.write(exe, fileSize);

	// De-allocate all
	free(exe);

}

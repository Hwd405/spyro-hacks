#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <windows.h>
#include "header.h"
#include "types.cpp"

using namespace std;

void randomiseMusic(char* exe) {
	printf("Randomising music...\n");
	srand(seed + 1879);

	int musicOffsets[4] = { 0x55F34, 0x58720, 0x58720, 0x5600C };

	char music[37] = { 0 };
	char entry = 0;

	char max = 29;
	if (gameVersion == 3) {
		max = 37;
	}

	for (int i = 0; i < 37; i++) {
		bool foundValid = false;
		while (!foundValid) {
			entry = rand() % max;
			foundValid = (entry != 0x00) // Belfry
				&& (entry != 0x03) // Bone Dance
				&& (entry != 0x0b || gameVersion == 3) // Ice Dance, 0x0c in U Rev 1
				&& (entry != 0x0c || gameVersion != 3); // Ice Dance, 0x0b in earlier versions
		}
		music[i] = entry;
	}

	// Open file
	fstream openExe(exe, ios_base::in | ios_base::out | ios_base::binary);

	// Write names to file
	openExe.seekp(musicOffsets[gameVersion]);
	openExe.write(music, sizeof(music));

	return;
}

void randomiseLoadingSkyboxes(char* wad, char* exe) {
	// wad currently unused, may use it if I device to overwrite the skyboxes in the future
	// Standard array: 00 01 02 03 04 05 06 00 00 07 08 09 0A 0B 0C 0D 00 00 0E 0F 10 11 12 13 14 00 00 15 16 17 18 19 1A 1B 00 00 00
	// Levels X7 and X8 will just be set to 00 as they're not used during gameplay, 50 could be randomised
	// Start with the array that contains only the standard levels, shuffle it, and then fill in the rest
	printf("Randomising loading screen skyboxes...\n");

	int skyboxOffsets[4] = { 0x56E8C, 0x59678, 0x59678, 0x56F64 };

	char skyboxes[28] = { 0 };
	for (int i = 0; i < 28; i++) {
		skyboxes[i] = i;
	}

	shuffle(&skyboxes[0], &skyboxes[28], std::default_random_engine(seed + 243));

	char skyboxesNew[37] = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 7; j++) {
			skyboxesNew[i * 9 + j] = skyboxes[i * 7 + j]; // Skips X7 and X8
		}
	}
	srand(seed * 2);
	skyboxesNew[36] = rand() % 28;

	// Crashed during a loading transition, so changing the homeworld ones back to normal
	for (int i = 0; i < 4; i++) {
		skyboxesNew[i * 9] = i * 7;
	}

	// Open file
	fstream openExe(exe, ios_base::in | ios_base::out | ios_base::binary);

	// Write names to file
	openExe.seekp(skyboxOffsets[gameVersion]);
	openExe.write(skyboxesNew, sizeof(skyboxesNew));

	return;
}

void randomiseSparxColours(char* exe) {
	printf("Randomising Sparx colours...\n");
	srand(seed + 1998);
	
	unsigned int sparxColourOffsets[4] = { 0x56060, 0x5884c, 0x5884c, 0x56138 }; // All in .data

	unsigned int greens[] = { 0x0a009000 /*green*/, 0x0a000090 /*red*/, 0x0ae000d0 /*magenta*/, 0x0a606060 /*grey*/ };
	unsigned int blues[] = { 0x09d03020 /*blue*/, 0x094090e0 /*orange*/, 0x09a0a040 /*cyan*/, 0x0980a030 /*teal*/ };
	unsigned int golds[] = { 0x0900D0E0 /*gold*/, 0x099090E0 /*pink*/, 0x09e000d0 /*purple*/ , 0x09c0c0c0 /*white*/ };

	unsigned int* allowedColours[] = { &(greens[0]), &(blues[0]), &(golds[0]), &(golds[0]) };

	// Open file
	fstream openExe(exe, ios_base::in | ios_base::out | ios_base::binary);

	// Load block into memory
	char* colours = (char*)malloc(16);
	openExe.seekg(sparxColourOffsets[gameVersion]);
	openExe.read(colours, 16);

	for (int i = 0; i < 4; i++) {
		((int*)colours)[i] = (allowedColours[i])[rand() % 4];

		// Extra randomness
		if (rand() % 10 == 0) {
			for (int j = 0; j < 3; j++) {
				colours[4 * i + j] = rand() % 256;
			}
		}
	}

	// Modes
	int gen = rand() % 20;
	if (gen < 2) {
		// Gummy
		colours[3] = 0x34 + (rand() % 9);
		colours[7] = 0x34 + (rand() % 9);
		colours[11] = 0x34 + (rand() % 9);
		colours[15] = 0x34 + (rand() % 9);
	}
	else if (gen == 2) {
		// Invisible
		colours[3] = 0x10;
		colours[7] = 0x10;
		colours[11] = 0x10;
		colours[15] = 0x10;
	}
	else if (gen == 3) {
		// Rave
		colours[4 * (rand() % 4) + 3] = 0x40;
	}
	else if (gen == 4) {
		// Upside-down
		colours[3] = 0x50;
	}

	// Write to file
	openExe.seekp(sparxColourOffsets[gameVersion]);
	openExe.write(colours, 16);

	openExe.close();
	free(colours);

};

void randomiseTextColours(char* exe) {
	printf("Randomising text colours...\n");
	srand(seed + 2023);

	unsigned int colourChoices[] = {
		0x64305868,
		0x643098B0,
		0x643098B0,
		0x644050F8,
		0x64086888,
		0x64005070,
		0x643070B0,
		0x64A878C8,
		0x64586830,
		0x6410A0D0,
		0x64485860,
		0x64808080,
		0x640508B0,
		0x64050880,
		0x64C3EC00,
		0x64A0C840,
		0x6438F898,
		0x64C83070,
		0x648040C0,
		0x6440C080,
		0x64C08040,
		0x644080C0,
		0x6480C040,
		0x64C04080,
		0x64E0A040
	};

	shuffle(begin(colourChoices), end(colourChoices), default_random_engine(seed + 2023));

	unsigned int textColourOffsets[4] = { 0x57020, 0x59804, 0x59804, 0x570F8 }; // All in .data

	// Open file
	fstream openExe(exe, ios_base::in | ios_base::out | ios_base::binary);

	// Load block into memory
	char* colours = (char*)malloc(4 * 14);
	openExe.seekg(textColourOffsets[gameVersion]);
	openExe.read(colours, 4 * 14);

	// Do stuff
	((int*)colours)[0] = colourChoices[0];
	((int*)colours)[1] = colourChoices[1];
	((int*)colours)[2] = colourChoices[2];
	if (rand() % 3 == 0) { // Selection colour, normally white - this can look good randomised but I deliberately don't have this white in the list above
		((int*)colours)[3] = colourChoices[3];
	}
	((int*)colours)[4] = colourChoices[4];
	for (int i = 0; i < 14; i++) {
		if (rand() % 8 == 0) {
			// A bit silly, randomly changes the colour, possibly to something ugly
			colours[4 * i + 0] = 0x20 + (rand() % 25) * 8;
			colours[4 * i + 1] = 0x20 + (rand() % 25) * 8;
			colours[4 * i + 2] = 0x20 + (rand() % 25) * 8;
			if (colours[4 * i + 0] + colours[4 * i + 1] + colours[4 * i + 2] < 0x80) {
				// A bit silly, makes it a bit brighter
				int offset = rand() % 3;
				unsigned int colour = (unsigned int)colours[4 * i + offset];
				colours[4 * i + offset] = min(colour + 0x80, 0xF8) % 256;
			}
		}
	}

	// Write to file
	openExe.seekp(textColourOffsets[gameVersion]);
	openExe.write(colours, 4 * 14);

	openExe.close();
	free(colours);

	return;

};

void renameSpyro(char* exe) {

	const char* altNames[] = { "Pete", "Pyro", "Pierre" };

	int name = 2;
	int random = rand() % 100;
#ifdef _DEBUG
	random = 90;
#endif
	if (random < 90) {
		return;
	}
	else if (random < 95) {
		name = 0;
	}
	else if (random < 99) {
		name = 1;
	}
	const char* chosenName = altNames[name];

	// Open file
	fstream openExe(exe, ios_base::in | ios_base::out | ios_base::binary);

	// Load block into memory
	char* nameBlock = (char*)malloc(8);

	for (int i = 0; i < strlen(chosenName); i++) {
		nameBlock[i] = chosenName[i];
		if (i + 1 == strlen(chosenName)) {
			for (int j = i + 1; j < 8; j++) {
				nameBlock[j] = 0;
			}
		}
	}

	openExe.seekp(sdataLocs[gameVersion] - addressOffset);
	openExe.write(nameBlock, 8);
	
	return;
};
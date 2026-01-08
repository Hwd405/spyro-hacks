#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <windows.h>
#include "header.h"
#include "types.cpp"

using namespace std;

void randomiseMoneybagsReqs(char* exe) {

	std::printf("Randomising Moneybags requirements...\n");

	//srand(seed + 320);

	MoneybagsEntries entries;
	int moneybagsOffsets[4] = { 0x576AC, 0x59E90, 0x59E90, 0x57784 };

	short* values[] = {
		&(entries.Lvl10.value),
		&(entries.Lvl12.value),
		&(entries.Lvl13.value),
		&(entries.Lvl20.value),
		&(entries.Lvl21.value),
		&(entries.Lvl23.value),
		&(entries.Lvl30.value),
		&(entries.Lvl31.value),
		&(entries.Lvl34.value),
		&(entries.Lvl40.value),
		&(entries.Lvl41.value),
		&(entries.Lvl42.value)
	};

	const char* mbLevels[] = {
		"Sunrise Spring Home - Sheila",
		"Cloud Spires - Primary",
		"Molten Crater - Secondary",
		"Midday Garden Home - Sgt. Byrd",
		"Icy Peak - Secondary",
		"Spooky Swamp - Primary",
		"Evening Lake Home - Bentley",
		"Frozen Altars - Secondary",
		"Charmed Ridge - Primary",
		"Midnight Mountain Home - Agent 9",
		"Crystal Islands - Primary",
		"Desert Ruins - Primary"
	};

	short average[] = {
		300, // 10
		200,
		300,
		700, // 20
		500,
		500,
		1000, // 30
		800,
		600,
		1300, // 40
		1000,
		800
	};

	short maxima[] = { // Calculated based on total gems realistically available at any stage, scaled down to be roughly what the player expects
		400,
		800,
		800,
		2100,
		2800,
		2800,
		5000,
		5600,
		5600,
		8000,
		8800,
		8800
	};

	default_random_engine generator;
	generator.seed(seed + 320);

	short gen;
	int tot = 0;

	for (int i = 0; i < 12; i++) {
		short limit = maxima[i] - tot;
		short mean = min(average[i], limit);
		short stdev = min(4 * average[i] / 5, limit); // av seems too large, av/2 is too small

		normal_distribution<double> normal(mean,stdev);
		bool valid = false;
		while (!valid) {
			gen = static_cast<short>(normal(generator));
			if (gen >= 0 && gen < limit) {
				valid = true;
			}
		}
		gen = gen - (gen % 5); // Round down to multiple of 5
		*(values[i]) = gen;
		tot += gen;
	}

	// Open file
	fstream openExe(exe, ios_base::in | ios_base::out | ios_base::binary);

	// Write names to file
	openExe.seekp(moneybagsOffsets[gameVersion]);
	openExe.write((char*)(&entries), sizeof(entries));
	openExe.close();

	// Open reqs txt
	fstream openTxt("requirements.txt", ios_base::binary | ios::app);
	openTxt << endl << "Moneybags Requirements" << endl << "----------------------" << endl << endl;

	// Write names to file
	for (int i = 0; i < 12; i++) {
		openTxt << *(values[i]) << (char)0x09 << (char)0x09;
		openTxt << mbLevels[i] << endl;
	}
	openTxt.close();

	return;
}

// Now unused
void randomiseMoneybagsReqsOld(char* exe) {

	std::printf("Randomising Moneybags requirements...\n");

	MoneybagsEntries entries;
	int moneybagsOffsets[4] = { 0x576AC, 0x59E90, 0x59E90, 0x57784 };

	short* values[] = {
		&(entries.Lvl10.value),
		&(entries.Lvl12.value),
		&(entries.Lvl13.value),
		&(entries.Lvl20.value),
		&(entries.Lvl21.value),
		&(entries.Lvl23.value),
		&(entries.Lvl30.value),
		&(entries.Lvl31.value),
		&(entries.Lvl34.value),
		&(entries.Lvl40.value),
		&(entries.Lvl41.value),
		&(entries.Lvl42.value)
	};

	short maxima[] = { // Calculated based on total gems realistically available at any stage, scaled down to be roughly what the player expects
		400,
		800,
		800,
		2100,
		2800,
		2800,
		5000,
		5600,
		5600,
		8000,
		8800,
		8800
	};

	short gen;
	int tot = 0;
	int max = 0;
	for (int i = 0; i < 12; i++) {
		srand(seed + 20 * i);
		max = max(maxima[i] + 1 - tot, 2); // Just in case it goes negative; if it's < 5 it'll round down to zero later anyway
		gen = rand() % max;
		gen = gen - (gen % 5); // Round down to multiple of 5
		*(values[i]) = gen;
		tot += gen;
	}

	// Open file
	fstream openExe(exe, ios_base::in | ios_base::out | ios_base::binary);

	// Write names to file
	openExe.seekp(moneybagsOffsets[gameVersion]);
	openExe.write((char*)(&entries), sizeof(entries));

	return;
}
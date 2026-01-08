#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <windows.h>
#include "header.h"
#include "types.cpp"

using namespace std;

typedef struct {
	Vector3D pos;
	u8 yaw;
} EggPosInfo;

typedef struct {
	Vector3D polygonPoints[5];
	u32 skyboxPointer;
	u8 renderDistance;
	u8 sidedness;
	u8 levelId;
	u8 animationState;
} PortalInfo;

vector<EggPosInfo> easyEggs[] = {
	/*10*/ {{{  36705,  48349, 20838 }, 107 }, {{  43215, 46974, 18280 }, 164 }, {{  88898, 22210, 15974 },  25 }},
	/*11*/ {{{  54176,  62868, 14695 }, 115 }, {{  13555, 41903, 16605 }, 234 }, {{  36524, 45334, 14186 },  39 }},
	/*12*/ {{{  81595,  97780,  5992 }, 138 }, {{  50910, 95271,  7274 },  33 }, {{  33228, 43374,  5480 }, 177 }},
	/*13*/ {{{  66923,  98463, 10808 },  11 }},
	/*14*/ {{{  84153,  47047, 21386 },  14 }, {{ 109852, 58100, 16214 },   1 }, {{  83953, 95081, 22872 }, 225 }},
	/*20*/ {{{  57118,  20094,  7527 },  58 }, {{  84884, 25223,  7528 },  62 }},
	/*21*/ {{{  64725,  75170, 11113 }, 151 }, {{  84306, 53187, 16232 }, 227 }},
	/*22*/ {{{  75247,  26046, 17768 }, 154 }, {{ 106165, 71986, 23120 }, 160 }, {{  67992, 84892, 18801 },   4 }},
	/*23*/ {{{  38337,  15823,  4968 },  92 }, {{  14018, 68119,  5479 },  51 }, {{  69188, 82487,  7784 },   1 }, {{ 90099, 48738, 11304 }, 62 }},
	/*24*/ {{{  91848,  57907, 14694 }, 179 }, {{ 135853, 51192, 18792 }, 159 }},
	/*30*/ {{{  46773,  61761, 39269 },  92 }, {{  68747, 66511, 31590 },  87 }},
	/*31*/ {{{  96764,  51813, 19814 },  95 }, {{ 100588, 76816, 22534 },  82 }},
	/*32*/ {{{ 104601,  68735, 25927 }, 231 }, {{  57512, 62881, 24520 }, 199 }, {{  63071, 87711, 22886 },   0 }},
	/*33*/ {{{  66355,  35098,  5481 },   2 }, {{  14953, 70131,  6503 },  36 }},
	/*34*/ {{{  96832, 109554, 25334 },  17 }, {{ 117321, 71576, 19769 }, 214 }, {{ 156097, 65246, 16199 },  23 }},
	/*40*/ {{{  70531,  55809, 12646 }, 233 }, {{ 121083, 65293, 20839 },  59 }},
	/*41*/ {{{  45006, 104699,  5989 }, 221 }},
	/*42*/ {{{  85430,  75844, 20926 },  43 }, {{ 121911, 64230, 22890 }, 153 }},
	/*43*/ {{{  56103,  21581, 16232 },  38 }},
	/*44*/ {{{  68732,  60600, 21352 }, 150 }, {{  63124, 93196, 21352 }, 233 }}
};

vector<EggPosInfo> medEggs[] = {
	/*10*/ {{{  57875,  53077, 21864 },  96 }, {{  24737, 68017, 19558 },  60 }, {{  93253,  38636, 24166 },   2 }},
	/*11*/ {{{  55346,  63895, 18278 },  51 }, {{  83386, 76810, 15955 },  62 }, {{  70031,  69479, 23911 }, 188 }},
	/*12*/ {{{  62811,  53984,  7272 }, 248 }, {{  71890, 73776, 14695 },  16 }},
	/*13*/ {{{  30917,  96503, 12807 }, 196 }},
	/*14*/ {{{ 143131,  81165, 25258 }, 133 }, {{ 116718, 88784, 26667 },  15 }, {{  76900,  42443, 19952 }, 151 }, {{ 76011, 28784, 21277 }, 46 }},
	/*20*/ {{{  40081,  71187,  5477 },  57 }, {{  56083, 53725, 11622 }, 110 }, {{  39064,  29999, 11622 }, 161 }},
	/*21*/ {{{  55784,  33482, 15191 }, 149 }, {{  49758, 29223,  9121 }, 132 }},
	/*22*/ {{{  34009,  69710, 22518 },  93 }, {{  66664, 82028, 28951 }, 153 }},
	/*23*/ {{{  17148,  34958,  7015 },  59 }, {{  44176, 83405, 12329 }, 178 }, {{  31381,  79197, 11366 }, 104 }},
	/*24*/ {{{ 125493,  30117, 17255 }, 225 }, {{  71274, 67804, 12648 },  46 }, {{  99441,  79701, 18233 },  77 }},
	/*30*/ {{{  67843,  62010, 14804 }, 151 }, {{  58402, 40069, 22886 }, 216 }},
	/*31*/ {{{  74515, 108097, 25445 }, 189 }, {{  77417, 65394, 26469 }, 194 }},
	/*32*/ {{{  88910,  29459, 25368 }, 119 }, {{ 104358, 68909, 34318 }, 133 }, {{  69843,  79215, 27484 }, 226 }},
	/*33*/ {{{  35273,  25572,  9574 },  84 }, {{  34401, 96695,  6502 }, 220 }, {{  67807,  78240, 13669 },   0 }},
	/*34*/ {{{  66273,  55253, 21475 }, 183 }, {{  86264, 58337, 21180 }, 188 }, {{ 102500,  85697, 25363 },  18 }},
	/*40*/ {{{ 100763, 104736, 19870 }, 203 }},
	/*41*/ {{{ 113834,  32986, 25448 },  65 }, {{ 108284, 49520, 21354 },  25 }, {{  55718, 113634, 18275 }, 213 }},
	/*42*/ {{{  43209,  63752, 25115 },  54 }, {{  57342, 63638, 25383 }, 161 }, {{  93980,  24019, 26982 }, 106 }},
	/*43*/ {{{  92458,  58013, 15720 },  12 }},
	/*44*/ {{{  87212, 114566, 12650 },  12 }, {{ 102212, 82566,  8650 }, 118 }, {{ 108205,  45779, 24934 }, 101 }}
};

vector<EggPosInfo> hardEggs[] = {
	/*10*/ {{{  57511, 23160, 20665 }, 159 }, {{ 24746, 27280, 22109 }, 239 }},
	/*11*/ {{{  52145, 51725, 23399 }, 247 }},
	/*12*/ {{{  34581, 30777, 10734 },  81 }},
	/*13*/ {{{  66371, 75350, 20839 }, 131 }},
	/*14*/ {{{ 121868, 48144, 24705 },  15 }},
	/*20*/ {{{  49966, 85591, 12649 },  34 }},
	/*21*/ {{{ 114596, 28120, 19246 }, 123 }},
	/*22*/ {{{  73164, 22138, 40412 },  44 }},
	/*23*/ {{{  50055, 20309,  8499 },  26 }},
	/*24*/ {{{  56049, 81078, 21608 }, 171 }},
	/*30*/ {{{  62467, 40573, 39314 }, 138 }}, // {{  24622, 10751, 48239 },  55 } and {{  22236, 72550, 11551 },  73 } seem to be broken
	/*31*/ {{{  59236, 62472, 28710 }, 158 }, {{ 89593, 36922, 28667 }, 126 }},
	/*32*/ {{{  36651, 53491, 28759 }, 218 }, {{ 72576, 65153, 29663 }, 121 }},
	/*33*/ {{{  86033, 63036, 19304 }, 125 }, {{ 34608, 94247, 12806 },   0 }},
	/*34*/ {{{ 125535, 87599, 27281 },  61 }, {{ 89719, 80361, 26788 }, 125 }},
	/*40*/ {{{  74387, 89360, 21672 },  70 }},
	/*41*/ {{{  95982, 97290, 20528 },  41 }},
	/*42*/ {{{  36885, 48355, 26448 }, 226 }, {{ 81559, 71564, 25384 },  33 }, {{ 59897, 71536, 25383 },  33 }, {{ 58318, 57575, 29932 }, 135 }},
	/*43*/ {{{  41016, 47729, 18280 }, 134 }},
	/*44*/ {{{  81658, 52039, 27054 }, 130 }} /* {{ 166947, 95848, 10599 }, 0 } sublevel special egg? */
};

short sorcReq = 0;

void printEggLocations(char* layoutData, char* mobyBlock, int level, int sublevel) {

	Moby* mobys = (Moby*)(mobyBlock + 8);
	int numberOfMobys = *(int*)(mobyBlock + 4);

	for (int i = 0; i < numberOfMobys; i++) {
		if (mobys[i].mobyClass == 132) {
			cout << "Egg - Level " << dec << level << ", Sublevel " << dec << sublevel << ": { ";
			cout << dec << mobys[i].position.x << ", ";
			cout << dec << mobys[i].position.y << ", ";
			cout << dec << mobys[i].position.z << " }, state ";
			cout << dec << (int)mobys[i].state << ", dragon ";
			cout << dec << *(int*)(layoutData + mobys[i].mobyTag) << ", tag data { ";
			cout << dec << *(int*)(layoutData + mobys[i].mobyTag + 4) << ", ";
			cout << dec << *(int*)(layoutData + mobys[i].mobyTag + 8) << ", ";
			cout << dec << *(int*)(layoutData + mobys[i].mobyTag + 12) << ", ";
			cout << dec << *(int*)(layoutData + mobys[i].mobyTag + 16) << ", ";
			cout << dec << *(int*)(layoutData + mobys[i].mobyTag + 20);
			cout << " }" << endl;
			// *tag is the dragon name number!!!
		}
		if (mobys[i].mobyClass == 563) {
			cout << "Extra life - Level " << dec << level << ", Sublevel " << dec << sublevel << ": { ";
			cout << dec << mobys[i].position.x << ", ";
			cout << dec << mobys[i].position.y << ", ";
			cout << dec << mobys[i].position.z << " }, state ";
			cout << dec << (int)mobys[i].state << endl;
		}
	}
	return;
}

char* getPortalBlock(char* levelData, int levelNo, DataHeader dataHeader) {

	// Locate the portal info block
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
		printf("An error occurred opening WAD %d. Portals untouched.\n", 97 + 2 * levelNo);
		return 0;
	}
	currentlyReading += 3 * 4;
	for (int j = 0; j < 2; j++) {
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
		printf("An error occurred opening WAD %d. Portals untouched.\n", 97 + 2 * levelNo);
		return 0;
	}
	currentlyReading += 8;

	return currentlyReading;
}

void randomisePortals(char* levelData, char* layoutData, char* mobyBlock, int levelNo, DataHeader dataHeader) {

	// For later worlds, shuffle the level IDs
	// For all worlds, shuffle the skyboxes

	PortalInfo* portalInfoBlock = (PortalInfo*)getPortalBlock(levelData, levelNo, dataHeader);
	if (portalInfoBlock == 0) {
		printf("An error occurred opening WAD %d. Portals untouched.\n", 97 + 2 * levelNo);
		return;
	}
	int numberOfPortals = *((int*)portalInfoBlock - 1);

	vector<u32> skyboxPointers;
	// Shuffle the skyboxes
	for (int i = 0; i < numberOfPortals; i++) {
		skyboxPointers.push_back(portalInfoBlock[i].skyboxPointer);
	}
	shuffle(begin(skyboxPointers), end(skyboxPointers), default_random_engine(seed + 183 * levelNo));
	for (int i = 0; i < skyboxPointers.size(); i++) {
		portalInfoBlock[i].skyboxPointer = skyboxPointers[i];
	}

	if (levelNo > 9) {
		int newOrder[4] = { 1, 2, 3, 4 };
		shuffle(begin(newOrder), end(newOrder), default_random_engine(seed + 185 * levelNo));
		int portalIndices[4] = { 0 };

		for (int i = 0; i < numberOfPortals; i++) {
			if (portalInfoBlock[i].levelId % 10 < 5 && portalInfoBlock[i].levelId % 10 != 0) {
				portalIndices[(portalInfoBlock[i].levelId % 10 - 1) % 4] = i;
			}
		}

		Moby* mobys = (Moby*)(mobyBlock + 8);
		int numberOfMobys = *(int*)(mobyBlock + 4);
		int mobyClass = 988;

		for (int i = 0; i < numberOfMobys; i++) {
			if (mobys[i].mobyClass == mobyClass) {
				Moby988Tag* mobyTag = (Moby988Tag*)(layoutData + mobys[i].mobyTag);
				int localLevelNo = mobyTag->LevelId % 10;
				if (localLevelNo < 5 && localLevelNo % 10 != 0) {
					mobyTag->LevelId = mobyTag->LevelId - localLevelNo + newOrder[localLevelNo - 1];
					portalInfoBlock[portalIndices[localLevelNo - 1]].levelId = mobyTag->LevelId;
				}
			}
		}
	}
	
	return;
}

void changeSparxHeight(char* mobyBlock) {
	// Doesn't work, probably does something in code but I don't know where
	srand(seed + 0x120);

	Moby* mobys = (Moby*)(mobyBlock + 8);
	int numberOfMobys = *(int*)(mobyBlock + 4);

	for (int i = 0; i < numberOfMobys; i++) {
		if (mobys[i].mobyClass == 120) {
			int gen = rand();
			char offset = min((gen & 0xff) % 16, ((gen & 0xff00) >> 8) % 16); // Keeps the offset smaller, generally
			if ((gen & 0x10000) >> 16 == 1) {
				offset ^= 0x80;
			}
			mobys[i].scaleDown = (0x20 + offset);
//#ifdef _DEBUG
			//(gen % 2 == 0) ? mobys[i].scaleDown = 0x30 : mobys[i].scaleDown = 0x10;
//#endif
		}
	}

	return;
}

void updateSorcReq(char* ovl, unsigned int ovlLen) {

	unsigned int eggReqOffsets[4] = { 0x562C, 0x57E0, 0x57E0, 0x562C };
	unsigned int eggReqOffset = eggReqOffsets[gameVersion];
	unsigned int overlayOffset = overlayOffsets[gameVersion];

	if (eggReqOffset + 2 /* End of the short */ > ovlLen) {
		printf("Error getting Moby code pointers.\n");
		return;
	}

	*(short*)(ovl + eggReqOffset) = sorcReq;

	return;
}

void randomiseEggReqs(char* layoutData, char* mobyBlock, int world) {

	printf("Randomising egg requirements for a homeworld...\n");

	Moby* mobys = (Moby*)(mobyBlock + 8);
	int numberOfMobys = *(int*)(mobyBlock + 4);

	int mobyClass = 988;
	int sorcClass = 950;

	short average[] = {
		10, // 13; 0 = (3 % 9 - 3) + 3 * (3 / 9)
		14,
		20,
		25, // 23; 3 = (12 % 9 - 3) + 3 * (12 / 9)
		30,
		36,
		50, // 33
		58,
		65,
		70, // 43
		80,
		90,
		100 // 47
	};

	short maxima[] = {
		(world + 1) * 36 - 16,
		(world + 1) * 36 - 16,
		min((world + 2) * 36 - 16, 140)
	};

	short generated[] = {
		0,
		0,
		0
	};
	short generatedSorc = 0;

	const char* eggLevels[8] = {
		"Sunrise Spring",
		"Mushroom Speedway",
		"Midday Gardens",
		"Country Speedway",
		"Evening Lake",
		"Honey Speedway",
		"Midnight Mountain",
		"Harbor Speedway"
	};

	default_random_engine generator;
	generator.seed(seed + 38);
	short gen;

	for (int i = 0; i < 3; i++) {

		short mean = average[i + (world * 3)];
		short stdev = min(2 * mean / 5, maxima[i]);

		normal_distribution<double> normal(mean, stdev);
		bool valid = false;
		while (!valid) {
			gen = static_cast<short>(normal(generator));
			if (gen >= highestEgg && gen < maxima[i]) {
				valid = true;
			}
			/*
			if (i == 2) { // For extra skewing downwards for speedways?
				short gen2;
				bool valid2 = false;
				while (!valid2) {
					gen2 = static_cast<short>(normal(generator));
					if (gen2 >= highestEgg && gen < maxima[i]) {
						valid2 = true;
					}
				}
				gen = min(gen, gen2);
			}
			*/
		}
		generated[i] = gen;
	}

	sort(std::begin(generated), std::end(generated));
	highestEgg = generated[1];

	if (world == 3) {

		short mean = average[12];
		short stdev = min(1 * mean / 5, 145);

		normal_distribution<double> normal(mean, stdev);
		bool valid = false;
		while (!valid) {
			gen = static_cast<short>(normal(generator));
			if (gen >= highestEgg && gen < 145) {
				valid = true;
			}
		}
		generatedSorc = gen;
	}

	char applOrder = rand() % 2;

	for (int i = 0; i < numberOfMobys; i++) {
		if (mobys[i].mobyClass == mobyClass) {
			Moby988Tag* mobyTag = (Moby988Tag*)(layoutData + mobys[i].mobyTag);
			if (mobyTag->EggReq != 0) {
				if ((mobyTag->LevelId) % 10 == 5) {
					mobyTag->EggReq = generated[2];
				}
				else if (mobyTag->LevelId != 50) {
					mobyTag->EggReq = generated[applOrder];
					applOrder = 1 - applOrder;
				}
			}
		}
		else if (mobys[i].mobyClass == sorcClass) {
			int* mobyTag = (int*)(layoutData + mobys[i].mobyTag);
			*mobyTag = generatedSorc; // Egg limit is at MobyTag + 0
		}
	}

	// Open reqs txt
	fstream openTxt;

	if (world == 0) {
		openTxt.open("requirements.txt", ios_base::out | ios_base::binary);
		openTxt << "Egg Requirements" << endl << "----------------------" << endl << endl;
	}
	else {
		openTxt.open("requirements.txt", ios_base::binary | ios::app);
	}

	// Write names to file
	openTxt << generated[0] << ", " << generated[1] << (char)0x09 << (char)0x09;
	openTxt << eggLevels[2 * world] << " Levels" << endl;
	openTxt << generated[2] << (char)0x09 << (char)0x09;
	openTxt << eggLevels[2 * world + 1] << endl;
	if (world == 3) {
		openTxt << generatedSorc << (char)0x09 << (char)0x09;
		openTxt << "Sorceress' Lair" << endl;
	}
	openTxt.close();

	if (generatedSorc != 0) {
		sorcReq = generatedSorc;
	}

	return;

}

void randomiseEggReqsOld(char* layoutData, char* mobyBlock, int world) {

	printf("Randomising egg requirements for a homeworld...\n");

	Moby* mobys = (Moby*)(mobyBlock + 8);
	int numberOfMobys = *(int*)(mobyBlock + 4);

	int mobyClass = 988;
	int sorcClass = 950;

	// For now this is simple and only updates levels that already have requirements - I need to look into other levels a bit more (maybe later worlds are easier to figure out than earlier ones though)

	int low = highestEgg;
	int high = (world + 1) * 36 - 16;

	// gen 3 from low to high - 6, high, high + 6
	// set highestEgg to the max of the three
	// when overwriting, only overwrite the ones with a nonzero egg count
	// set the speedway to the highest
	// set the other two to the lower of the two, randomly

	int limits[] = { 0, 0, 0 };
	srand(seed + 42);
	for (int i = 0; i < 3; i++) {
		int modHigh = high + 6 * i - 6; // essentially means we're using the upper bounds high - 6, high, high + 6
		limits[i] = (rand() % (modHigh - low)) + low;
	}
	sort(std::begin(limits), std::end(limits));
	highestEgg = limits[2];
	char applOrder = rand() % 2;

	for (int i = 0; i < numberOfMobys; i++) {
		if (mobys[i].mobyClass == mobyClass) {
			Moby988Tag* mobyTag = (Moby988Tag*)(layoutData + mobys[i].mobyTag);
			if (mobyTag->EggReq != 0) {
				if ((mobyTag->LevelId) % 10 == 5) {
					mobyTag->EggReq = limits[2];
				}
				else if (mobyTag->LevelId != 50) {
					mobyTag->EggReq = limits[applOrder];
					applOrder = 1 - applOrder;
				}
			}
		}
		else if (mobys[i].mobyClass == sorcClass) {
			int* mobyTag = (int*)(layoutData + mobys[i].mobyTag);
			// Should be between highestEgg and 145, we generate two variables and take the minimum to skew it lower
			// Note that the expectation of the minimum of two uniform variables U[0,n] is n/3
			int gen1 = (rand() % (145 - highestEgg)) + highestEgg;
			int gen2 = (rand() % (145 - highestEgg)) + highestEgg;
			*mobyTag = min(gen1, gen2); // Egg limit is at MobyTag + 0
		}
	}
	return;
}

void shuffleEggPlacement(char* mobyBlock, int levelNo) {
	srand(seed + 91 * levelNo);

	Moby* mobys = (Moby*)(mobyBlock + 8);
	int numberOfMobys = *(int*)(mobyBlock + 4);

	vector<short> foundMobys;
	vector<Vector3D> foundPositions;
	vector<unsigned char> foundAngles;

	int superMode = 0;

	vector<short> mobyClasses = { 132 };
	if (   levelNo != 13 /* Bamboo Terrace - too many extra lives */
		&& levelNo != 20 /* Lost Fleet - acid (there's one in a sublevel which is fine but not useful) */
		&& levelNo != 21 /* Fireworks Factory - unused / inaccessible under normal means */
		&& levelNo != 27 /* Midnight Mountain - island, I could probably add these into the moby list (NO POS) if there's enough good locations though! Add more later */ 
		&& rand() % 2 == 0) {
		mobyClasses.push_back(563);
		superMode ^= 1;
	}
	else if (levelNo == 21 && rand() % 16 == 0) { // Since it's not technically impossible to get this one, but make it a very rare event
		mobyClasses.push_back(563);
		superMode ^= 1;
	}
	else if (levelNo == 18) { // To avoid whale-related issues, keeping the positions vector nice and long
		mobyClasses.push_back(563);
		superMode ^= 1;
	}
	if (rand() % 2 == 0) {
		//mobyClasses.push_back(202); This causes issues, just use its position instead
		superMode ^= 2; // This is for class 202 using the previous implementation
	}
#ifdef _DEBUG
	if (superMode) {
		cout << "Eggs to be super shuffled in level " << dec << (int)levelIds[levelNo] << ", mode " << dec << superMode << endl;
	}
	else {
		cout << "Eggs to be shuffled in level " << dec << (int)levelIds[levelNo] << endl;
	}
#endif

	for (int i = 0; i < numberOfMobys; i++) {
		if (count(mobyClasses.begin(), mobyClasses.end(), mobys[i].mobyClass)) {
			if (levelNo ==  2 && mobys[i].mobyClass == 132 && mobys[i].position.x ==  24074) { // Exclude Level 12 island egg because I can't figure it out (something in code?)
				continue;
			}
			if (levelNo == 18 && mobys[i].mobyClass == 132 && mobys[i].position.x ==  25088) { // Exclude Level 30 whale egg POSITION due to issues
				// Note that if egg shuffling to new locations ever becomes optional, this needs to be carefully looked at
				// This should only work because the randomiser guarantees to collect #positions >= #eggs
				if (rand() % 4 == 0) {
					foundMobys.push_back(i);
				}
				continue;
			}
			if (levelNo == 20 && mobys[i].mobyClass == 132 && mobys[i].position.x ==  94966) { // Exclude Level 32 EOL egg
				continue;
			}
			if (levelNo == 29 && mobys[i].mobyClass == 132 && mobys[i].position.x == 133868) { // Exclude Level 42 cave egg because I can't figure it out (something in code?)
				continue;
			}
			foundMobys.push_back(i);
			foundPositions.push_back(mobys[i].position);
			if (levelNo ==  0 && mobys[i].mobyClass == 563 && mobys[i].position.x ==  49695) { // Exceptions for extra lives facing awkward directions
				foundAngles.push_back(mobys[i].angle.z + 0x80);
				continue;
			}
			foundAngles.push_back(mobys[i].angle.z);
		}
		if (mobys[i].mobyClass == 202 && superMode > 1) {
			foundPositions.push_back(mobys[i].position);
			foundPositions.back().z += 440; // Eggs will sit on top of headbash chests with this
			foundAngles.push_back(mobys[i].angle.z);
		}
	}

	int levelIndex = (levelNo % 9) + 5 * (levelNo / 9);
	int difficulty = rand() % 10;
	if (difficulty < 6 && !hardMode) {
		// Add some "easy" eggs from a lookup table
		for (int i = 0; i < easyEggs[levelIndex].size(); i++) {
			Vector3D pos = (easyEggs[levelIndex])[i].pos;
			pos.z -= 400; // These all seem too high
			foundPositions.push_back(pos);
			foundAngles.push_back((easyEggs[levelIndex])[i].yaw);
		}
	}
	if (difficulty > 2) {
		// Add some "medium" eggs from a lookup table
		for (int i = 0; i < medEggs[levelIndex].size(); i++) {
			Vector3D pos = (medEggs[levelIndex])[i].pos;
			pos.z -= 400; // These all seem too high
			foundPositions.push_back(pos);
			foundAngles.push_back((medEggs[levelIndex])[i].yaw);
		}
	}
	if (difficulty >= 9 - (levelNo / 9) /*Later worlds are made harder*/ || hardMode) {
		// Add some "hard" eggs from a lookup table
		for (int i = 0; i < hardEggs[levelIndex].size(); i++) {
			Vector3D pos = (hardEggs[levelIndex])[i].pos;
			pos.z -= 400; // These all seem too high
			foundPositions.push_back(pos);
			foundAngles.push_back((hardEggs[levelIndex])[i].yaw);
		}
	}

	// Quick error handler
	while (foundPositions.size() < foundMobys.size()) {
		cout << "Error occurred - no egg shuffling in level " << dec << (int)levelIds[levelNo] << endl;
		return;
	}

	vector<int> order;
	for (int i = 0; i < foundPositions.size(); i++)
	{
		order.push_back(i);
	}
	shuffle(begin(order), end(order), default_random_engine(seed + 83));

	for (int i = 0; i < foundMobys.size(); i++) {
		mobys[foundMobys[i]].position = foundPositions[order[i]];
		mobys[foundMobys[i]].angle.z = foundAngles[order[i]];
	}

	return;
}

void randomiseMobyPlacement(char* mobyBlock, vector<short> mobyClasses) {
	Moby* mobys = (Moby*)(mobyBlock + 8);
	int numberOfMobys = *(int*)(mobyBlock + 4);

	vector<short> foundMobys;
	vector<Vector3D> foundPositions;

	for (int i = 0; i < numberOfMobys; i++) {
		if (count(mobyClasses.begin(), mobyClasses.end(), mobys[i].mobyClass)) { // Checks the vector for any instances of the given class
			foundMobys.push_back(i);
			foundPositions.push_back(mobys[i].position);
		}
	}

	shuffle(begin(foundPositions), end(foundPositions), default_random_engine(seed + 1));

	for (int i = 0; i < foundMobys.size(); i++) {
		mobys[foundMobys[i]].position = foundPositions[i];
	}

	return;
}

void randomiseThiefColours(char* ovl, unsigned int ovlLen) {

	unsigned int mobyCodePointers = *(int*)(ovl + 8);
	unsigned int endAddress = mobyCodePointers - 1;
	unsigned int overlayOffset = overlayOffsets[gameVersion];
	unsigned int* mobyCodePointersBlock = (unsigned int*)(ovl + (mobyCodePointers - overlayOffset));
	if ((char*)mobyCodePointersBlock > ovl + ovlLen) {
		printf("Error getting Moby code pointers.\n");
		return;
	}
	short thiefClasses[] = { 88, 230, 506, 517, 519, 582, 584 };

	int newOrder[3] = {0, 1, 2};
	shuffle(begin(newOrder), end(newOrder), default_random_engine(seed + 31 * (*ovl /* Close to the level ID */)));
	srand(seed + 37 * (*ovl));

	for (int classIndex = 0; classIndex < 7; classIndex++) {
		unsigned int thiefAddress = mobyCodePointersBlock[thiefClasses[classIndex]];
		if (thiefAddress != 0) {
			unsigned int debugColour = 0;
			unsigned int currentAddress = thiefAddress;
			unsigned int* currentlyReading = (unsigned int*)(ovl + (currentAddress - overlayOffset));
			int foundLocsNo = 0;
			if ((char*)currentlyReading > ovl + ovlLen) {
				printf("Error getting Moby code pointers.\n");
				return;
			}
			while (currentAddress < endAddress && *currentlyReading != 0x03e00008 && foundLocsNo < 3) {
				if ((*currentlyReading & 0xfffffffc /* Just the lowest two bits to ignore here */) == 0xa2620054 && *currentlyReading != 0xa2620057) {
					// Work backwards and find the setter
					bool found = false;
					unsigned int* localRead = currentlyReading;
					while (localRead > (unsigned int*)(ovl + (thiefAddress - overlayOffset)) && !found) {
						localRead--;
						if ((*localRead & 0xffffff00) == 0x24020000) {
							unsigned char colour = (rand() % 5) * 0x20 + 0x40;
							*(unsigned char*)localRead = colour;
							*((unsigned char*)&debugColour + newOrder[foundLocsNo % 3]) = colour;
							found = true;
						}
					}
					// Which colour gets set
					*(unsigned char*)currentlyReading = 0x54 + newOrder[foundLocsNo % 3];
					foundLocsNo++;
				}
				else if ((*currentlyReading & 0xfffffffc /* Just the lowest two bits to ignore here */) == 0xa2600054) {
					// This is the zero setting one
					if (*currentlyReading != 0xa2600057 && rand() % 30 == 0) { // This is the alpha
						// Do nothing right now - maybe have a "turn thief invisible" thing at some point by switching this with one of the actual setters
					}
					else {
						*(unsigned char*)currentlyReading = 0x54 + newOrder[foundLocsNo % 3];
						foundLocsNo++;
					}
				}
				currentAddress += 4;
				currentlyReading += 1;
			}
#ifdef _DEBUG:
			cout << "A thief has changed colour! Level " << dec << (int)levelIds[(*ovl - 12)] << ", colour 0x" << hex << debugColour << endl;
#endif
		}
	}
	// Make it print something for debugging reasons
	return;
}

void randomiseThiefColoursOld(char* mobyBlock) { // Doesn't work - this is overwritten by code
	Moby* mobys = (Moby*)(mobyBlock + 8);
	int numberOfMobys = *(int*)(mobyBlock + 4);
	short thiefClasses[] = { 88, 230, 506, 517, 519, 582, 584 };

	srand(seed + 812);

	for (int i = 0; i < numberOfMobys; i++) {
		if (count(begin(thiefClasses), end(thiefClasses), mobys[i].mobyClass)) { // Checks the vector for any instances of the given class
			//mobys[i].colour = rand() & 0x00FFFFFF; // Could maybe make it each just take a random value for each from 0x00, 0x40, 0x80, 0xC0, if this looks bad (test it?)
			char* colour = (char*)&(mobys[i].colour);
			for (int j = 0; j < 3; j++) {
				colour[j] = (rand() % 4) * 0x40;
			}
			if (rand() % 50 == 0) { // Rare chance per thief
				colour[3] = 0x10;
			}
		}
	}
	return;

	// Ideas for the future - update it in code
	// e.g. Midday Garden Home:
	/*
	
     n::8007f480 80 00 02 24     li         v0,0x80
     n::8007f484 55 00 62 a2     sb         v0,0x55(s3)
     n::8007f488 c0 00 02 24     li         v0,0xc0
     n::8007f48c 56 00 62 a2     sb         v0,0x56(s3)
     n::8007f490 01 00 02 24     li         v0,0x1
     n::8007f494 ff ff a5 34     ori        a1,a1,0xffff
     n::8007f498 54 00 60 a2     sb         zero,0x54(s3)
     n::8007f49c 57 00 60 a2     sb         zero,0x57(s3)
     n::8007f4a0 49 00 62 a2     sb         v0,0x49(s3)

	This only loads colours into two of them and sets the other colour and the alpha to 0
	Alpha being non-zero should be decided first, then nothing else needs to be changed aside switching which values are modified

	This would only need to happen once per level, not per sublevel
	*/




}

void randomiseVaseColours(char* mobyBlock) {
	Moby* mobys = (Moby*)(mobyBlock + 8);
	int numberOfMobys = *(int*)(mobyBlock + 4);
	short vaseClass = 201;

	for (int i = 0; i < numberOfMobys; i++) {
		if (vaseClass == mobys[i].mobyClass) {
			mobys[i].colour = rand() & 0x00FFFFFF;
			char* colour = (char*)&(mobys[i].colour);
			colour[3] = (rand() % 9) + 0x34;
			if (rand() % 7 < 6) {
				colour[rand() % 3] = 0; // Probably gives more interesting colours
			}
		}
	}
	return;
}

int getSublevels(int level, char* subwadHeader) {
	int wadOffset = wadHeader.entries[97 + 2 * level].offset;

	// Get the number of sublevels
	int numberOfSublevels = 1;
	int checkSublevels = *(int*)(subwadHeader + 0x18);

	while (checkSublevels != 0) {
		if (0x18 + 16 * numberOfSublevels > 16 * 6) { // Allowed for 6 rows, this should break as soon as the number of sublevels > 4
			// Something bad happened, don't update the value any further! This shouldn't be triggered
			checkSublevels = 0;
			numberOfSublevels = 0;
			printf("An error occurred reading WAD %d. Mobys untouched.\n", 97 + 2 * level);
		}
		else {
			checkSublevels = *(int*)(subwadHeader + 0x18 + 16 * numberOfSublevels);
			if (checkSublevels != 0) {
				numberOfSublevels++;
			}
		}
	}
	return numberOfSublevels;
}

char* getMobyBlock(char* layoutData, unsigned int layoutLength) {
	// Locate the moby block
	char* currentlyReading = layoutData + 0x30;
	bool checksFailed = false;

	for (int k = 0; k < 9; k++) {
		// Confirm we're not jumping into anywhere we don't want to be
		if (currentlyReading + *(int*)currentlyReading >= layoutData + layoutLength) {
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
		if (currentlyReading + *(int*)currentlyReading >= layoutData + layoutLength) {
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

	return currentlyReading;
}

void randomiseMobyData(char* wad) {
	srand(seed + 874);

	int thiefLevels[7] = { 12, 13, 20, 21, 24, 40, 50 };

	fstream openWad(wad, ios_base::in | ios_base::out | ios_base::binary);
	printf("Randomising Moby placement and egg requirements...\n");
	for (int i = 0; i < 37; i++) {

		int wadOffset = wadHeader.entries[97 + 2 * i].offset;
		int wadSize = wadHeader.entries[97 + 2 * i].length;

		// Get the number of sublevels
		char* header = (char*)malloc(16*6);
		openWad.seekg(wadOffset);
		openWad.read(header, 16 * 6);
		int numberOfSublevels = getSublevels(i, header);

		for (int j = 0; j < numberOfSublevels; j++) {

			// Check header
			DataHeader dataHeader = *(DataHeader*)(header + 0x18 + 16 * j);
			if (dataHeader.start + dataHeader.length /*End of data section*/ > wadHeader.entries[97 + 2 * i].length /*End of WAD*/) {
				printf("An error occurred reading WAD %d. Mobys untouched.\n", 97 + 2 * i);
				continue;
			}

			// Load the whole block into memory
			openWad.seekg(wadHeader.entries[97 + 2 * i].offset + dataHeader.start);
			char* layoutData = (char*)malloc(dataHeader.length);
			openWad.read(layoutData, dataHeader.length);

			// Locate the moby block
			char* mobyBlock = getMobyBlock(layoutData, dataHeader.length);
			if (mobyBlock == 0) {
				printf("An error occurred reading WAD %d. Mobys untouched.\n", 97 + 2 * i);
				continue;
			}

			/* THIS IS WHERE I CAN PUT NEW MOBY RELATED THINGS!!! */

			// *(int*)mobyBlock is length
			// *(int*)(mobyBlock + 4) is no of mobys
			// (Moby*)(mobyBlock + 8) is mobys

#ifdef _DEBUG
			//printEggLocations(layoutData, mobyBlock, i, j); // VERY DEBUG
#endif

			//printf("Randomising gem placement...\n");
			randomiseMobyPlacement(mobyBlock, { 1 });

			if (rand() % 3 < 2) {
				randomiseMobyPlacement(mobyBlock, { 200, 201 }); // Not every level needs this
			}
			if ((i % 9 == 0) && (i != 36)) {
				randomiseEggReqs(layoutData, mobyBlock, i / 9); // Pass in both arguments as offsets are relative to layoutData
			}
			if (count(begin(thiefLevels), end(thiefLevels), levelIds[i])) {
				randomiseThiefColoursOld(mobyBlock); // Probably doesn't work, maybe does for some levels though?
			}
			if (rand() % 30 == 0) { // Rare
				randomiseVaseColours(mobyBlock);
			}
			if (j == 0 /* && (rand() % 4 < 3 ---currently always runs--- )*/ && (i % 9 < 5) && i != 36) {
				shuffleEggPlacement(mobyBlock, i);
			}
			// DOESN'T WORK
#ifdef _DEBUG
			changeSparxHeight(mobyBlock);
#endif

			// randomise the size of some mobys

			// Get the data block loaded into memory too
			// Check level data size is valid
			DataHeader levelDataHeader = *(DataHeader*)(header + 8);

			if (levelDataHeader.start + levelDataHeader.length /*End of data section*/ > wadHeader.entries[97 + 2 * i].length /*End of WAD*/) {
				printf("An error occurred opening WAD %d. Portals untouched.\n", 97 + 2 * i);
			}
			else {

				// Load the whole block into memory
				openWad.seekg(wadHeader.entries[97 + 2 * i].offset + levelDataHeader.start);
				char* levelData = (char*)malloc(levelDataHeader.length);
				openWad.read(levelData, levelDataHeader.length);

				if ((i % 9 == 0) && (i != 36)) {
					randomisePortals(levelData, layoutData, mobyBlock, i, levelDataHeader);
				}

				// Rewrite data block
				openWad.seekp(levelDataHeader.start /*level data header position*/ + wadHeader.entries[97 + 2 * i].offset /*WAD offset*/);
				openWad.write(levelData, levelDataHeader.length);
				free(levelData);
			}

			// Overlays
			if (j == 0 && (count(begin(thiefLevels), end(thiefLevels), levelIds[i] /*|| i == 27 This is already a thief level */))) {
				srand(seed + 495 * i);
				openWad.seekg(wadHeader.entries[98 + 2 * i].offset);
				char* ovl = (char*)malloc(wadHeader.entries[98 + 2 * i].length);
				openWad.read(ovl, wadHeader.entries[98 + 2 * i].length);

				decryptOverlay(ovl, wadHeader.entries[98 + 2 * i].length);

				// UPDATE OVERLAY
				randomiseThiefColours(ovl, wadHeader.entries[98 + 2 * i].length);
				if (i == 27) {// Midnight Mountain
					updateSorcReq(ovl, wadHeader.entries[98 + 2 * i].length);
				}

				decryptOverlay(ovl, wadHeader.entries[98 + 2 * i].length);

				openWad.seekp(wadHeader.entries[98 + 2 * i].offset);
				openWad.write(ovl, wadHeader.entries[98 + 2 * i].length);
				free(ovl);
			}

/*

- Change the egg req generation to be a normal distribution around their original values, capped at the reasonable limits and using the highest *normal* level from the previous world as a lower limit
-- Speedways should be allowed to go up as high as the next world's standard upper limit, but should be skewed negatively
- Print out a text document containing all of the generated limits

- Attempt in-world portal shuffling for levels X1-X4, X>1
-- This would also mean updating the 988 structs to show the correct level name
- Portal skybox changes

- For each level, shuffle a vector of (compatible) Rhynoc classes to shift around together (hopefully without crashing)

- Similarly, create a function to take in a vector of classes and change some features of those mobys
-- Specifically, maybe the RGBA and the Scale? Perhaps a small chance of turning off the high LOD?

Colour
u32 colour; // 0x54
For vases it seems A = 0x3F makes the colours saturated but this will go down to 0x30 when flamed - maybe a small chance of colouring all the vases in a level randomly?

NOT FOR HERE:
Maybe could randomly award Sparx abilities?
Sparx colours could be altered - they're just after the sparx struct (8006585c in Sep14)
	Red looks cool instead of green, maybe I should generate a bunch of colours to choose from for each

Egg shuffling could be mixed with extra life vases too, though bamboo terrace is a weird one for this

*/

			// Rewrite the entire layout data block
			openWad.seekp(dataHeader.start /*data header position*/ + wadHeader.entries[97 + 2 * i].offset /*WAD offset*/);
			openWad.write(layoutData, dataHeader.length); // Somewhat inefficient, I could just do mobys + tags but I'm not sure about the tag block length
			free(layoutData);
		}
		free(header);
	}

	openWad.close();
	return;
}
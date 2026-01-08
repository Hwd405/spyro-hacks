#pragma once

// CRC32
typedef unsigned long ULONG;
int getCRC(unsigned char* buffer, ULONG bufsize);

// WAD
typedef struct {
	int offset;
	int length;
} WadEntry;

typedef struct {
	WadEntry entries[256];
} Wad;

// Dragon names
extern const char* names[];
extern int namesLength;

// Level names
extern const char* namesFirstHalf[];
extern const char* namesSecondHalf[];
extern const char* namesFixed[];
extern int levelsLengthFh;
extern int levelsLengthSh;

// Global variables
extern unsigned int seed;
extern int gameVersion;
extern Wad wadHeader;
extern int highestEgg;
extern unsigned char levelIds[];
extern unsigned int addressOffset;
extern unsigned int overlayOffsets[4];
extern unsigned int checksumValues[37];
extern unsigned int endOfText[4];
extern unsigned int sdataLocs[4];
extern unsigned int exeChecksumStarts[4];
extern bool collectMode;
extern bool hardMode;

// Functions
void randomiseMoneybagsReqs(char* exe);
void randomiseMusic(char* exe);
void randomiseLoadingSkyboxes(char* wad, char* exe);
void randomiseMobyData(char* wad);
void randomiseMobyPlacement(char* mobyBlock, std::vector<short> mobyClass);
void randomiseEggReqs(char* layoutData, char* mobyBlock, int world);
void randomiseTextColours(char* exe);
void randomiseSparxColours(char* exe);
void renameSpyro(char* exe);
void recolorSpyro(char* wadPath);
void runChecksumCorrectionProcess(char* wad, char* exe);
void collectChecksums(char* wadPath, char* exePath);
void decryptOverlay(char* ovl, unsigned int ovlLength);
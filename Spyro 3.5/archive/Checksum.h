#pragma once
#include "CommonHeader.h"

typedef enum ScanType {
	LEVEL,
	CRITTER,
	OTHER
};

typedef enum ScanMode {
	EXE,
	OVL
};

typedef struct ChecksumScanInfo {

	// Mode
	ScanMode scanMode;

	// Pointer to blocks in memory
	char* pointer;

	// Address offset
	unsigned int offset;

	// Scan start addresses
	unsigned int startAddress;

	// Scan end addresses
	unsigned int endAddress;

	// Correction variable, likely only need to use the second one
	unsigned int correctionAddress;
	unsigned int scanBeforeCorrection;
	unsigned int scanAfterCorrection;

	// Scanning variables
	int bitmask;
	int shift;

};

class Checksum {
public:
	// Scan type
	ScanType scanType;

	// Info for each scan
	ChecksumScanInfo exe;
	ChecksumScanInfo ovl;

	int levelNumber;

	// Not sure if I'll need this
	unsigned int runningCheck;

	// Scanned value?
	unsigned int scannedValue;

	Checksum(char* exePointer, char* ovlPointer, int levelNumber, ScanType scanType);
	unsigned int Scan(unsigned int initialCheckValue, unsigned int startAddress, unsigned int endAddress, ScanMode scanMode);
	unsigned int Scan(unsigned int initialCheckValue, ChecksumScanInfo scan);

	void RunCorrectionScan(bool fastMode); // scan over both using correction variable
	unsigned int GetFullScan(); // scan over both and return output

};


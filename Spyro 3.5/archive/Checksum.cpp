#include "Checksum.h"

/**
* 
* Checksum.cpp
* Possibly unused checksum / antipiracy related tools
* These are very much incomplete and I'll be making better versions of them in the future
* 
*/

void setCorrectionLocation(ChecksumScanInfo &scan) {

    unsigned int currentAddress = scan.startAddress;
    int* currentlyReading;
    unsigned int location = 0;

    while (currentAddress < scan.endAddress) {
        currentlyReading = (int*)(scan.pointer + scan.startAddress - scan.offset);
        if (*currentlyReading == 0x03e00008) {
            if ((*(currentlyReading + 2) >> 24) == 8) { // Usually a sign of a correction value
                location = currentAddress + 8 /* +8 because the read position is still at the jr ra */;
                break;
            }
        }
        currentAddress++;
    }
    scan.correctionAddress = location;

    return;
}

void setOverlayBoundaries(ChecksumScanInfo &scan) {

    if (scan.scanMode != OVL) {
        printf("ERROR - invalid ChecksumScanInfo passed to setOverlayBoundaries\n");
        return;
    }

    scan.endAddress = *(unsigned int*)(scan.pointer + 8);

    int* mobyCodePointers = (int*)(scan.pointer + scan.endAddress - scan.offset);

    for (int i = 0; i < 1024; i++) {
        if (mobyCodePointers[i] != 0) {
            scan.startAddress = mobyCodePointers[i];
            break;
        }
        if (i == 1023) {
            printf("ERROR - could not find a valid moby code pointer\n");
            return;
        }
    }

    return;
}

void Checksum::RunCorrectionScan(bool fastMode) {
    // Scan over both using correction variable

    unsigned int end = this->ovl.correctionAddress + 4;
    unsigned int desiredValue = this->ovl.scanAfterCorrection;
    if (!fastMode) {
        end = this->ovl.endAddress;
        desiredValue = 0;
    }
    
    // Assuming LEVEL?
    unsigned int runningCheck = Scan(0, this->exe);
    runningCheck = Scan(runningCheck, this->ovl.startAddress, this->ovl.correctionAddress, OVL);
    unsigned int checkTest = 0x08000000;

    *(int*)(this->ovl.pointer + this->ovl.correctionAddress - this->ovl.offset) = checkTest;

    while (Scan(runningCheck, this->ovl.correctionAddress, end, OVL) != desiredValue) {
        checkTest++;
        *(int*)(this->ovl.pointer + this->ovl.correctionAddress - this->ovl.offset) = checkTest;
    }
    return;

}; 


unsigned int Checksum::GetFullScan() {
    // scan over both and return output
    
    // Assuming LEVEL?
    unsigned int runningCheck = Scan(0, this->exe);
    return Scan(runningCheck, this->ovl);

};

unsigned int Checksum::Scan(unsigned int initialCheckValue, unsigned int startAddress, unsigned int endAddress, ScanMode scanMode) {

    // Setup
    unsigned int currentAddress = startAddress;
    unsigned int currentReadValue;
    unsigned int runningCheck = initialCheckValue;
    int iVar6;
    unsigned int uVar7;

    unsigned int memoryOffset = 0;
    char* code = 0;
    unsigned int bitmask;

    if (scanMode == EXE) {
        memoryOffset = this->exe.offset;
        code = this->exe.pointer;
        bitmask = this->exe.bitmask;
    }
    else if (scanMode == OVL) {
        memoryOffset = this->ovl.offset;
        code = this->ovl.pointer;
        bitmask = this->ovl.bitmask;
    }

    // Scan
    while (currentAddress < endAddress) {
        iVar6 = 0;
        currentReadValue = *(unsigned int*)(code + (currentAddress - memoryOffset));
        runningCheck = runningCheck ^ currentReadValue << 8;
        do {
            uVar7 = runningCheck << 1;
            if ((runningCheck & 0x8000) != 0) {
                uVar7 = uVar7 ^ 0x8005;
            }
            runningCheck = uVar7;
            iVar6 = iVar6 + 1;
        } while (iVar6 < 8);
        currentAddress = (currentAddress + (runningCheck & bitmask) + 1);
    }

    return runningCheck;

}

unsigned int Checksum::Scan(unsigned int initialCheckValue, ChecksumScanInfo scan) {
	return Scan(initialCheckValue, scan.startAddress, scan.endAddress, scan.scanMode);
}

// If using this, make sure that the overlay is already decrypted
Checksum::Checksum(char* exePointer, char* ovlPointer, int levelNumber, ScanType scanType) {

    this->runningCheck = 0;
    this->levelNumber = levelNumber;
    this->scanType = scanType;

    unsigned int addressOffset = 0x8000F800;
    unsigned int overlayOffsets[2] = { 0x800742d0, 0x800743b0 };
    unsigned int exeEnd[2] = { 0x80064f9c, 0x80065034 };
    unsigned int exeStart[2] = { 0x8001200e, 0x80012022 };

    this->exe.scanMode = EXE;
    this->ovl.scanMode = OVL;

    this->exe.pointer = exePointer;
    this->ovl.pointer = ovlPointer;

    this->exe.offset = addressOffset;
    this->ovl.offset = overlayOffsets[gameVersion];

    if (this->scanType == LEVEL) {

        setOverlayBoundaries(this->ovl);
        setCorrectionLocation(this->ovl);

        this->exe.startAddress = exeStart[gameVersion] + this->levelNumber;
        this->exe.endAddress = exeEnd[gameVersion];

        this->exe.bitmask = 3;
        this->ovl.bitmask = 0;

        this->exe.shift = 0;
        this->ovl.shift = 0;

        unsigned int runningCheck = Scan(0, this->exe);
        runningCheck = Scan(runningCheck, this->ovl.startAddress, this->ovl.correctionAddress + 4, OVL);
        this->ovl.scanAfterCorrection = runningCheck;
        this->scannedValue = Scan(runningCheck, this->ovl.correctionAddress + 4, this->ovl.endAddress, OVL);
        printf("DEBUG - Found value of checksum: %d\n", this->scannedValue);

        return;

    }

    printf("ERROR - Invalid scan type provided.\n");
	return;
}
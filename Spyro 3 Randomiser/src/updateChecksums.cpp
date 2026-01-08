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

int jumps = 0;

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

unsigned int getChecksumLocation(char* ovl) {

    // Code assumes overlay is decrypted
    unsigned int overlayOffset = overlayOffsets[gameVersion];
    unsigned int mobyCodePointers = *(int*)(ovl + 8);
    unsigned int endAddress = mobyCodePointers - 1;
    unsigned int startAddress = *(int*)(ovl + (mobyCodePointers - overlayOffset));
    int iter = 1;
    while (startAddress == 0) {
        startAddress = *(int*)(ovl + (mobyCodePointers + 4 * iter - overlayOffset)); // Finds first moby function
        iter++;
    }

    int* currentlyReading = (int*)(ovl + startAddress - overlayOffset);
    int* endPosition = (int*)(ovl + endAddress - overlayOffset);
    int checkLoc = 0;

    while (currentlyReading < endPosition) {
        if (*currentlyReading == 0x03e00008) {
            if ((*(currentlyReading + 2) >> 24) == 8) {
                checkLoc = (unsigned int)currentlyReading - (unsigned int)ovl + overlayOffset + 8 /* +8 because the read position is still at the jr ra */;
                break;
            }
        }
        currentlyReading = currentlyReading + 1;
    }

    return checkLoc;
}

void lookForChecksums(char* wadPath) {

    fstream openWad(wadPath, ios_base::in | ios_base::out | ios_base::binary);

    for (int i = 0; i < 37; i++) {

        // Load ovl into a malloc
        openWad.seekg(wadHeader.entries[98 + 2 * i].offset);
        char* ovl = (char*)malloc(wadHeader.entries[98 + 2 * i].length);
        openWad.read(ovl, wadHeader.entries[98 + 2 * i].length);

        decryptOverlay(ovl, wadHeader.entries[98 + 2 * i].length);

        unsigned int checkLoc = getChecksumLocation(ovl);
        
        if (checkLoc != 0) {
            cout << "Level " << dec << i << " checksum location " << hex << checkLoc << ", checksum value " << hex << *(int*)(ovl + (checkLoc - overlayOffsets[gameVersion])) << endl;
        }
        else {
            cout << "Level " << dec << i << " checksum location not found." << endl;
        }

        free(ovl);

    }

    openWad.close();
    return;

}

unsigned int performChecksum(char* code, unsigned int startAddress, unsigned int endAddress, unsigned int addressMemoryOffset, unsigned int initialRunningCheck, unsigned int bitmask) {

    // code - code block to checksum, e.g. exe or ovl
    // startAddress - initial address
    // endAddress - final address
    // addressMemoryOffset - e.g. file offset to memory address
    // initialRunningCheck - running check at the beginning
    // bitmask - bitmask used in moving the pointer

    // outputs the checksum at the end

    int iVar6;
    unsigned int uVar7;
    unsigned int currentReadAddress = startAddress;
    unsigned int runningCheck = initialRunningCheck;
    unsigned int currentlyReadingVariable = 0;

    if (currentReadAddress == 0x0) {
        currentReadAddress = startAddress;
    }
    while (currentReadAddress < endAddress) {
        iVar6 = 0;
        currentlyReadingVariable = *(unsigned int*)(code + (currentReadAddress - addressMemoryOffset));
        runningCheck = runningCheck ^ currentlyReadingVariable << 8;
        do {
            uVar7 = runningCheck << 1;
            if ((runningCheck & 0x8000) != 0) {
                uVar7 = uVar7 ^ 0x8005;
            }
            runningCheck = uVar7;
            iVar6 = iVar6 + 1;
        } while (iVar6 < 8);
        currentReadAddress = (currentReadAddress + (runningCheck & bitmask) + 1);
        jumps += 1; // Debugging only
    }

    return runningCheck;

}

unsigned int checksumExe(char* exe, int levelNo) {

    return performChecksum(exe, levelNo + exeChecksumStarts[gameVersion], endOfText[gameVersion], addressOffset, 0, 3);

}

void updateChecksum(char* wadPath, char* exePath, int levelNo, bool fastMode) {

    // Perform checksums up to the check location
#ifdef _DEBUG
    if (fastMode) {
        printf("Weakly invalid checksum found in level %d. Correcting...\n", (int)levelIds[levelNo]);
    }
#endif
    if (!fastMode) {
        printf("Strongly invalid checksum found in level %d. Correcting (this may take a while)...\n", (int)levelIds[levelNo]);
    }

    fstream openExe(exePath, ios_base::in | ios_base::out | ios_base::binary);
    fstream openWad(wadPath, ios_base::in | ios_base::out | ios_base::binary);

    streampos fileSize;
    openExe.unsetf(ios::skipws);
    openExe.seekg(0, ios::end);
    fileSize = openExe.tellg();
    openExe.seekg(0, ios::beg);

    char* exe = (char*)malloc(fileSize);
    openExe.read(exe, fileSize);

    unsigned int overlayOffset = overlayOffsets[gameVersion];

    // Exe checksum
    int iVar6;
    unsigned int uVar7;
    unsigned int readAddress;
    unsigned int currentReadAddress = 0;
    unsigned int runningCheck = 0;
    unsigned int currentlyReadingVariable = 0;

    runningCheck = checksumExe(exe, levelNo);

    openWad.seekg(wadHeader.entries[98 + 2 * levelNo].offset);
    char* ovl = (char*)malloc(wadHeader.entries[98 + 2 * levelNo].length);
    openWad.read(ovl, wadHeader.entries[98 + 2 * levelNo].length);

    decryptOverlay(ovl, wadHeader.entries[98 + 2 * levelNo].length);

    unsigned int mobyCodePointers = *(int*)(ovl + 8);
    unsigned int startAddress = *(int*)(ovl + (mobyCodePointers - overlayOffset));
    int iter = 1;
    while (startAddress == 0) {
        startAddress = *(int*)(ovl + (mobyCodePointers + 4 * iter - overlayOffset)); // Finds first moby function
        iter++;
    }

    unsigned int checksumLocation = getChecksumLocation(ovl);
    if (checksumLocation == 0) {
        printf("Error updating checksum for the overlay for level %d. Antipiracy will trigger.\n", levelNo);
        return;
    }

    // Overlay checksum part 1
    unsigned int checkBeforeBruteforce = performChecksum(ovl, startAddress, checksumLocation, overlayOffset, runningCheck, 0);

    // currentReadAddress should be checksumLocation right now
    unsigned int checkTest = 0x08000000; // All checksums seem to be of this form

    unsigned int finalCheck;
    unsigned int endAddress;
    if (fastMode) {
        finalCheck = checksumValues[levelNo] + 1; // Assumes everything after the check value has been untouched and that these values were correct in the first pass
        endAddress = checksumLocation + 4;
    }
    else {
        finalCheck = 1;
        endAddress = mobyCodePointers;
    }

    unsigned int desiredValue = finalCheck - 1;

    // Overlay checksum part 2
    while (finalCheck != desiredValue) {

        currentReadAddress = checksumLocation;
        runningCheck = checkBeforeBruteforce;
        *(unsigned int*)(ovl + (currentReadAddress - overlayOffset)) = checkTest;

        while (currentReadAddress < endAddress) {
            iVar6 = 0;
            currentlyReadingVariable = *(unsigned int*)(ovl + (currentReadAddress - overlayOffset));
            runningCheck = runningCheck ^ currentlyReadingVariable << 8;
            do {
                uVar7 = runningCheck << 1;
                if ((runningCheck & 0x8000) != 0) {
                    uVar7 = uVar7 ^ 0x8005;
                }
                runningCheck = uVar7;
                iVar6 = iVar6 + 1;
            } while (iVar6 < 8);
            currentReadAddress = currentReadAddress + 1;
        }

        finalCheck = runningCheck;
        checkTest++;
    }

    checkTest--; // Removes an off-by-one error, but at this point the ovl in memory should be perfect anyway...

    // Update file
    decryptOverlay(ovl, wadHeader.entries[98 + 2 * levelNo].length); // Re-encrypt
    openWad.seekp(wadHeader.entries[98 + 2 * levelNo].offset);
    openWad.write(ovl, wadHeader.entries[98 + 2 * levelNo].length);

    free(ovl);
    openExe.close();
    openWad.close();
    return;
}

int calculateChecksum(char* wadPath, char* exePath, int levelNo) {
    int exeJumps = 0;
    int ovlJumps = 0;

    fstream openExe(exePath, ios_base::in | ios_base::out | ios_base::binary);
    fstream openWad(wadPath, ios_base::in | ios_base::out | ios_base::binary);

    streampos fileSize;
    openExe.unsetf(ios::skipws);
    openExe.seekg(0, ios::end);
    fileSize = openExe.tellg();
    openExe.seekg(0, ios::beg);

    char* exe = (char*)malloc(fileSize);
    openExe.read(exe, fileSize);

    unsigned int overlayOffset = overlayOffsets[gameVersion];

    // Exe checksum
    int iVar6;
    unsigned int uVar7;
    unsigned int readAddress;
    unsigned int currentReadAddress = 0;
    unsigned int runningCheck = 0;
    unsigned int currentlyReadingVariable = 0;

    jumps = 0; // Debugging only
    runningCheck = checksumExe(exe, levelNo);
    exeJumps = jumps; // Debugging only
    jumps = 0; // Debugging only

    //cout << "Checksum after exe (Level " << dec << levelNo << ") - " << hex << runningCheck << endl;

    openWad.seekg(wadHeader.entries[98 + 2 * levelNo].offset);
    char* ovl = (char*)malloc(wadHeader.entries[98 + 2 * levelNo].length);
    openWad.read(ovl, wadHeader.entries[98 + 2 * levelNo].length);

    decryptOverlay(ovl, wadHeader.entries[98 + 2 * levelNo].length);

    unsigned int mobyCodePointers = *(int*)(ovl + 8);
    unsigned int endAddress = mobyCodePointers - 1;
    unsigned int startAddress = *(int*)(ovl + (mobyCodePointers - overlayOffset));
    int iter = 1;
    while (startAddress == 0) {
        startAddress = *(int*)(ovl + (mobyCodePointers + 4 * iter - overlayOffset)); // Finds first moby function
        iter++;
    }
    currentReadAddress = startAddress;

    unsigned int checksumLocation = getChecksumLocation(ovl);
    if (checksumLocation == 0) {
        printf("Error calcuating checksum location for the overlay for level %d. Antipiracy may trigger.\n", levelNo);
    }

    // Overlay checksum
    while (currentReadAddress <= endAddress) {
        iVar6 = 0;
        currentlyReadingVariable = *(unsigned int*)(ovl + (currentReadAddress - overlayOffset));
        runningCheck = runningCheck ^ currentlyReadingVariable << 8;
        do {
            uVar7 = runningCheck << 1;
            if ((runningCheck & 0x8000) != 0) {
                uVar7 = uVar7 ^ 0x8005;
            }
            runningCheck = uVar7;
            iVar6 = iVar6 + 1;
        } while (iVar6 < 8);
        currentReadAddress = currentReadAddress + 1;
        ovlJumps += 1; // Debugging only
        if (collectMode && (currentReadAddress == checksumLocation + 4)) {
            checksumValues[levelNo] = runningCheck;
#ifdef _DEBUG
            cout << "Expected running checksum for level " << dec << (int)levelIds[levelNo] << ": " << hex << runningCheck << endl; // To get a lookup table
#endif
        }
    }

    //cout << "Checksum after ovl (Level " << dec << levelNo << ") - " << hex << runningCheck << endl;

#ifdef _DEBUG
    if (collectMode) {
        printf("Jumps - Exe: %d, Ovl: %d, Total: %d\n",exeJumps,ovlJumps,exeJumps+ovlJumps);
    }
#endif

    free(ovl);
    openExe.close();
    openWad.close();
    return runningCheck;

}

void runChecksumCorrectionProcess(char* wadPath, char* exePath) {

    // PAL VERSIONS NOT YET SUPPORTED - THANKS LIBCRYPT

    printf("Cleaning up...\n");

    // Correct EXE via initialise (check after the fGetClock function)
    // Is this the only way in which the EXE is checksummed? Implement this later
    // Either way this is... quite important lol

    // Correct the overlays
    // Would be nice to use the checksums from a lookup table here
    for (int i = 0; i < 37; i++) {
        if (calculateChecksum(wadPath, exePath, i) != 0) {
            // Soft corrections - for if there are edits earlier in the overlays
            updateChecksum(wadPath, exePath, i, true);

            // Hard corrections - for if there are edits late in the overlay, or if the original checksums weren't valid
            if (calculateChecksum(wadPath, exePath, i) != 0 /* Try again just to be sure */) {
                updateChecksum(wadPath, exePath, i, false);
            }
        }
    }

    // This should be more than just levels, Common is a good one to check too - implement these later

    return;
}

void collectChecksums(char* wadPath, char* exePath) {

    // Crude checksum collection up until the location of the check address
    for (int i = 0; i < 37; i++) {
        calculateChecksum(wadPath, exePath, i); // Not using this value, just updating the checksumValues
    }
    collectMode = false; // This stops the checksumValues being updated in the future
    return;
}
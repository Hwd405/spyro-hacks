#include "SpyroExporter.h"

unsigned int spyroStringAddress = 0x8006c3b0; // Sep14
// 53 70 79 72 6f 00 00 00
DWORD spyroStringValue = 0x0000006f72797053;

unsigned int currentLevelAddress = 0x8006c5bc; // Sep14
unsigned int endOfLayout = 0x8006c7e0;
unsigned int emulatorBaseAddress = 0x00000000; // Sample value, should work
unsigned int emulatorExampleOffset = 0x00000000; // Sample value, should work
DWORD emulatorLimit = 0x10000000; // Sample value
//DWORD emulatorOffset = 0x7F7DAEC0; // Difference between the actual memory addresses and the ePSXe offset

DWORD epsxeVramAddress = 0x0534C420; // 0x0C96E420; // Changes every time??
//DWORD epsxeVramAddress2 = 0x0C96E420 - 0x014E5140; // Relative to 0x80000000

unsigned int dataPointersPointer = 0x800722C0;

int main(void) {

    // Standard stuff
    HANDLE  hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    time_t currentTime = time(NULL);
    HWND hwnd = FindWindowA(NULL, "ePSXe - Enhanced PSX emulator");
    if (hwnd == NULL) {
        printf("Cannot find window\n");
        Sleep(3000);
        exit(-1);
    }

    // Set up process handle
    DWORD procID;
    GetWindowThreadProcessId(hwnd, &procID);
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
    if (procID == NULL) {
        printf("Cannot find process ID\n");
        Sleep(3000);
        exit(-1);
    }

    // Search for 
    printf("Searching...\n");
    bool found = false;
    bool overflow = false;
    DWORD* reading = (DWORD*)((unsigned int)emulatorBaseAddress + emulatorExampleOffset);
    DWORD buffer = 0;
    while (!found && !overflow) {
        reading++;
        ReadProcessMemory(handle, reading, &buffer, sizeof(buffer), 0);
        if (buffer == spyroStringValue) {
            found = true;
        }
        if ((DWORD)reading > emulatorLimit) {
            overflow = true;
        }
    }
    if (overflow) {
        printf("Cannot find Spyro string\n");
        Sleep(3000);
        exit(-1);
    }

    DWORD* spyroStringMemory = reading;
    DWORD* baseMemory = (DWORD*)((unsigned int)spyroStringMemory - (spyroStringAddress - 0x80000000)); // This is essentially address 0x80000000 in the PSX's memory

    unsigned int levelIdMemAddress = (unsigned int)spyroStringMemory + (currentLevelAddress - spyroStringAddress);
    unsigned int levelId;
    ReadProcessMemory(handle, (DWORD*)levelIdMemAddress, &levelId, sizeof(levelId), 0);

    printf("Found Spyro string position. Press ENTER to export VRAM and Scene.\n");
    getchar();

    // DWORD* vramMemory = (DWORD*)((unsigned int)baseMemory + epsxeVramAddress2);

    DWORD* vramMemory = (DWORD*)epsxeVramAddress;
    DWORD vramBuffer;

    // Create a vram file using the current timestamp
    ofstream vramFile;
    string filename = ".\\vram-" + to_string(levelId) + "-" + to_string(currentTime) + ".vram";
    vramFile.open(filename, ios::binary | ios::app);

    // Now do the loop
    for (int i = 0; i < 0x200; i++) {
        // One row at a time
        for (int j = 0; j < 0x100; j++) {
            ReadProcessMemory(handle, vramMemory + j + 0x100 * i * 2 /*This is to skip the space assigned to the frame buffer*/, &vramBuffer, sizeof(vramBuffer), 0);
            for (int k = 0; k < 4; k++) {
                vramFile << (char)(vramBuffer % 256);
                vramBuffer >>= 8;
            }
            //vramFile << vramBuffer;
        }
    }
    vramFile.close();

    printf("VRAM export complete!\n");

    // Get some stuff
    DataPointers pointers;
    unsigned int pointersMemory = (unsigned int)spyroStringMemory + (dataPointersPointer - spyroStringAddress);
    ReadProcessMemory(handle, (DWORD*)pointersMemory, &pointers, sizeof(pointers), 0);

    DataPointers memory = pointers;
    memory.levelData += ((unsigned int)spyroStringMemory - spyroStringAddress);
    memory.scene += ((unsigned int)spyroStringMemory - spyroStringAddress);
    memory.layout += ((unsigned int)spyroStringMemory - spyroStringAddress);
    memory.unk += ((unsigned int)spyroStringMemory - spyroStringAddress);

    int endOfLayoutMemory = endOfLayout + ((unsigned int)spyroStringMemory - spyroStringAddress);
    int endOfLayoutPointer;
    ReadProcessMemory(handle, (DWORD*)endOfLayoutMemory, &endOfLayoutPointer, 4, 0);

    int levelDataLength = pointers.scene - pointers.levelData; // bytes
    int sceneLength = pointers.layout - pointers.scene; // bytes
    int layoutDataLength = endOfLayoutPointer - pointers.layout;

    // Create a scene file using the current timestamp
    char* sbuffer = (char*)malloc(sceneLength);
    ReadProcessMemory(handle, (DWORD*)memory.scene, sbuffer, sceneLength, 0);

    ofstream sceneFile;
    filename = ".\\scene-" + to_string(levelId) + "-" + to_string(currentTime) + ".scene";
    sceneFile.open(filename, ios::binary | ios::app);

    // Now do the loop
    for (int i = 0; i < sceneLength; i++) {
        sceneFile << sbuffer[i];
    }

    sceneFile.close();

    printf("Scene export complete!\n");

    // Create a leveldata file using the current timestamp
    char* dbuffer = (char*)malloc(levelDataLength);
    ReadProcessMemory(handle, (DWORD*)memory.levelData, dbuffer, levelDataLength, 0);

    ofstream dataFile;
    filename = ".\\leveldata-" + to_string(levelId) + "-" + to_string(currentTime) + ".data";
    dataFile.open(filename, ios::binary | ios::app);

    // Now do the loop
    for (int i = 0; i < levelDataLength; i++) {
        dataFile << dbuffer[i];
    }

    dataFile.close();

    printf("Level data export complete!\n");

    // Create a layoutData file using the current timestamp [up to the mobys]
    char* lbuffer = (char*)malloc(layoutDataLength);
    ReadProcessMemory(handle, (DWORD*)memory.layout, lbuffer, layoutDataLength, 0);

    ofstream layoutFile;
    filename = ".\\layout-" + to_string(levelId) + "-" + to_string(currentTime) + ".data";
    layoutFile.open(filename, ios::binary | ios::app);

    // Now do the loop
    for (int i = 0; i < layoutDataLength; i++) {
        layoutFile << lbuffer[i];
    }

    layoutFile.close();

    printf("Layout data export complete!\n");

    return 0;
}

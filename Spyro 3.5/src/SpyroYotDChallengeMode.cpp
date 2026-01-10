#include <chrono>
#include <iostream>
#include <windows.h>
#include "CommonHeader.h"

string version = "1.1.0";
string subversion = "1.1.0"; // This is just used in the title screen version number; e.g. 1.0.2p4
int gameVersion = -1;
Wad wadHeader;
bool musicFilesPresent = false;
unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
bool stormyMode = false;
bool explorationMode = false; // hardcoded

bool fileExists(const char* fileName) {
	std::ifstream infile(fileName);
	return infile.good();
}

void error(int errorCode) {

	// PlaySound(NULL, NULL, 0);
	// PlaySound(MAKEINTRESOURCE(IDR_WAVE2), GetModuleHandle(NULL), SND_RESOURCE | /*SND_LOOP | */SND_ASYNC);

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
	if ((wadCrc == 0x557c6b74 || wadCrc == 0x85b1439f)) {
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
		//gameVersion = 1; // Sep29
		error(11);
	}
	else if (wadCrc == 0x85b1439f) {
		//gameVersion = 2; // Oct24
		error(11);
	}
	else if (wadCrc == 0x008aeda8 && exeCrc == 0x5d7ff2fa) {
		gameVersion = 1; // Oct31
		cout << "NTSC-U Revision 1 detected. Please note that it is recommended that the player uses NTSC-U Revision 0 for this mod." << endl;
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

void UpdateBuildConfig() {

	const char* buildConfigPath = "buildConfig.xml";

	if (gameVersion == 0) {
		system("copy resources\\buildConfigSep14.xml buildConfig.xml /y >nul");
	}
	else {
		system("copy resources\\buildConfigOct31.xml buildConfig.xml /y >nul");
	}
	system("copy resources\\SPEECH2.STR data >nul");
	system("copy resources\\SPEECH3.STR data >nul");
	system("copy resources\\SPEECH4.STR data >nul");
	system("copy resources\\SPEECH5.STR data >nul");

	return;
}

void UpdateGame(char* wadPath, char* exePath) {

	if (seed % 2 > 0) {
		stormyMode = true;
	}
	stormyMode = true; // late development change, we don't really need Cloud City

	// For each level open a level wad class and do stuff accordingly
	UpdateLevels(wadPath, exePath);

	// Exe things
	UpdateExe(exePath);

	if (musicFilesPresent) {
		UpdateBuildConfig();
	}

	// Overlays
	UpdateOverlays(exePath, wadPath);

	// Checksums, if necessary

	return;
}

int main(int argc, char* argv[]) {

	HANDLE  hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, 11);
	//printf("\x1B[36mTextExample\033[0m\t\t");

	string debugString = "";
	string explorationString = "";

#ifdef _DEBUG
	debugString = " - DEBUG MODE";
#endif
	if (explorationMode) {
		explorationString = " - Exploration Build";
	}

	//const char* date = getResource();
	const char* date = __DATE__;
	const char* time = __TIME__;

	cout << "Spyro 3.5: Return to the Forgotten Realms" << explorationString << " (Version " << version << " - " << date << " " << time << debugString << ")" << endl;
	cout << "Written and designed by Hwd405 and Jeremy Thompson" << endl;
	cout << "Extra special thanks to Altro50, CrystalFissure, Composer, Pepper, Shemp, Lumilaura and WaffleWizard1" << endl;
	cout << "------------------------------------------------------------------------" << endl;

	SetConsoleTextAttribute(hConsole, 15);
	printf("A copy of Spyro 3 (e.g. bin/cue) is required to run this tool. Only NTSC-U final versions of the game are compatible.\n\n");
	printf("Press ENTER to continue.\n_\n");
	//PlaySound(MAKEINTRESOURCE(IDR_WAVE5), GetModuleHandle(NULL), SND_RESOURCE | /*SND_LOOP | */SND_ASYNC);
	getchar();

	//PlaySound(NULL, NULL, 0);
	//PlaySound(MAKEINTRESOURCE(IDR_WAVE3), GetModuleHandle(NULL), SND_RESOURCE | /*SND_LOOP | */SND_ASYNC);
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

	const char* mkpsxisoFiles[] = { "mkpsxiso.exe", "dumpsxiso.exe" };

	// Check that dumpsxiso and mkpsxiso are present
	if (!fileExists(mkpsxisoFiles[0])) {
		error(7);
	}
	else if (!fileExists(mkpsxisoFiles[1])) {
		error(8);
	}

	// Perform checks for music related files
	if (fileExists("resources/SPEECH2.STR") && fileExists("resources/buildConfigSep14.xml") && fileExists("resources/buildConfigOct31.xml")) {
		musicFilesPresent = true;
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
	if (confirmValidity((char*)inputWad, (char*)(inputExe[exeName]))) {
		//PlaySound(NULL, NULL, 0);
		//PlaySound(MAKEINTRESOURCE(IDR_WAVE4), GetModuleHandle(NULL), SND_RESOURCE | /*SND_LOOP | */SND_ASYNC);
		SetConsoleTextAttribute(hConsole, 14);
		UpdateGame((char*)inputWad, (char*)(inputExe[exeName]));

		//PlaySound(NULL, NULL, 0);
		//PlaySound(MAKEINTRESOURCE(IDR_WAVE4), GetModuleHandle(NULL), SND_RESOURCE | /*SND_LOOP | */SND_ASYNC);
		SetConsoleTextAttribute(hConsole, 15);
		cout << "------------------------------------------------------------------------" << endl;
		printf("Running mkpsxiso.exe...\n");
		system("mkpsxiso -q -o SpyroRttFR.bin -c SpyroRttFR.cue -y buildConfig.xml");
		cout << "------------------------------------------------------------------------" << endl;

		//PlaySound(NULL, NULL, 0);
		//PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE | /*SND_LOOP | */SND_ASYNC);
		SetConsoleTextAttribute(hConsole, 10);
		printf("_\nUpdate complete. (Press ENTER to exit)");
		getchar();
		SetConsoleTextAttribute(hConsole, 15);
		return 0;
	}

	SetConsoleTextAttribute(hConsole, 12);
	printf("ERROR: Unknown error occurred. (Press ENTER to exit)");
	getchar();
	return 0;

}
#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <windows.h>
#include "header.h"
#include "resource.h"
#include "types.cpp"

using namespace std;

struct SpyroColourResource {
	int resource;
	string name;
};

struct vRGB {
	vRGB() :R(0), G(0), B(0) {}
	vRGB(BYTE r, BYTE g, BYTE b) :R(r), G(g), B(b) {}
	BYTE R, G, B;
};

vector<vector<vRGB>> spyroColours;

unsigned int spyroClutStart = 0x40f82; // 0x400 per line

//Pass the handle to the window on which you may want to draw

unsigned short bgr (vRGB colour){
	byte r = (byte)(colour.R >> 3); // Bottom 3 bits are lost
	byte g = (byte)(colour.G >> 3); // Bottom 3 bits are lost
	byte b = (byte)(colour.B >> 3); // Bottom 3 bits are lost

	return (unsigned short)((b << 10) | (g << 5) | (r));
}

void getBitmapPixel(HWND hwnd, vector<vector<vRGB>>& pixel, int resource) {
	BITMAP bi;

	//Load Bitmap from resource file
	HBITMAP hBmp = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(resource)); // IDB_BITMAP1

	//Get the Height and Width
	::GetObject(hBmp, sizeof(bi), &bi);
	int Width = bi.bmWidth; int Height = bi.bmHeight;

	//Allocate and Initialize enough memory for external (Y-dimension) array
	pixel.resize(Height);

	//Create a memory device context and place your bitmap
	HDC hDC = GetDC(hwnd);
	HDC hMemDC = ::CreateCompatibleDC(hDC);
	SelectObject(hMemDC, hBmp);

	DWORD pixelData = 0;

	for (int y = 0; y < Height; ++y) {
		//Reserve memory for each internel (X-dimension) array
		pixel[y].reserve(Width);

		for (int x = 0; x < Width; ++x) {
			//Add the RGB pixel information to array.
			pixelData = GetPixel(hMemDC, x, y);
			pixel[y].push_back(vRGB(GetRValue(pixelData), GetGValue(pixelData), GetBValue(pixelData)));
		}
	}

	//Cleanup device contexts
	DeleteDC(hMemDC);
	ReleaseDC(hwnd, hDC);

	return;
}

void recolorSpyro(char* wadPath) {
	srand(seed + 1997);

	fstream openWad(wadPath, ios_base::in | ios_base::out | ios_base::binary);
	printf("Randomising Spyro's colour...\n");

	bool debugSkin = false; // Hardcoded
	bool lumiSkin = false; // Hardcoded

	// Colours
	vector<SpyroColourResource> resources = {
		{ SPYROCOLOUR_DEFAULT,    "Spyro (Default)" },
		{ SPYROCOLOUR_PETE,       "Pete" },
		{ SPYROCOLOUR_REDSPYRO,   "Spyro (Red)" },
		{ SPYROCOLOUR_PINKISH,    "Spyro (Pinkish)" },
		{ SPYROCOLOUR_BLUE,       "Spyro (Blue)" },
		{ SPYROCOLOUR_METALHEAD,  "Metalhead" },
		{ SPYROCOLOUR_LITEOLD,    "Lite (Old) by Pepper" },
		{ SPYROCOLOUR_BLACK,      "Spyro (Black) by Pepper" },
		{ SPYROCOLOUR_APPLEGREEN, "Spyro (Apple Green) by Pepper" },
		{ SPYROCOLOUR_PINK,       "Spyro (Pink) by Pepper" },
		{ SPYROCOLOUR_RT,         "Spyro Reignited by Pepper" },
		{ SPYROCOLOUR_WIG,        "Wig" },
		{ SPYROCOLOUR_YELLOW,     "Spyro (Yellow)" },
		{ SPYROCOLOUR_LUMI,       "Lumilaura by Pepper" },
		{ SPYROCOLOUR_CYPRIN,     "Cyprin" }
	};

	int colour = rand() % 32;

	if (colour > resources.size()) {
		colour = 0;
	}

#ifdef _DEBUG
	if (colour == 0) {
		colour = 1; // Prioritises Pete :)
	}
#endif

	SpyroColourResource resource;

	if (lumiSkin) {
		resource = { SPYROCOLOUR_LUMI, "Lumilaura by Pepper" };
	}
	else if (debugSkin) {
		// Set it here and goto the end?
		resource = { SPYROCOLOUR_DEBUG, "Debug Skin" };
	}
	else {
		resource = resources[colour];
	}

	if (resource.resource == SPYROCOLOUR_DEFAULT) {
		return; // No update needed
	}

	// Update colours
	HWND hwnd{ 0 };
	getBitmapPixel(hwnd, spyroColours, resource.resource);

	// Error handling
	if (spyroColours.size() < 24) {
		printf("Error changing Spyro's colour.");
		return;
	}
	else for (int i = 0; i < 12; i++) {
		if (spyroColours[i].size() < 15) {
			printf("Error changing Spyro's colour.");
			return;
		}
	}

	//spyroColours[row][column]
	unsigned short writeColour;
	for (int i = 0; i < 37; i++) {
		int wadOffset = wadHeader.entries[97 + 2 * i].offset;
		for (int row = 0; row < 24; row++) {
			for (int column = 0; column < 15; column++) {
				writeColour = bgr(spyroColours[row][column]);
				openWad.seekp(wadOffset + spyroClutStart + row*0x400 + column*2);
				openWad.write((char*)&writeColour, sizeof(writeColour));
			}
		}
	}

	openWad.close();
	return;
}
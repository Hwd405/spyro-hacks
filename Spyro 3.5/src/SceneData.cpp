#include "SceneData.h"

SceneData::SceneData() {}

SceneData::SceneData(int sublevel, unsigned char* buffer, int size) {

	this->sublevel = sublevel;
	auto r = new Reader(buffer, size);

	if (this->sublevel == 0) {

		this->unknown01 = Component<unsigned char>(r);

		this->vram = vramImage(r);

		return;
	}

	// There's something wrong in here but I don't know where - investigate!

	this->mobyModels = Component<unsigned char>(r);

	this->modelOffsets = (int*)r->getPos();
	r->increasePos(64 * 4);

	this->modelClasses = (short*)r->getPos();
	r->increasePos(64 * 2);

	this->unknown11 = Component<unsigned char>(r);
	this->unknown12 = Component<unsigned char>(r);
	this->unknown13 = Component<unsigned char>(r);
	this->unknown14 = Component<unsigned char>(r);

	this->collision = Collision((unsigned int*)r->getPos());
	this->unknown15 = Component<unsigned char>(r);
	// ^^ Collision component - however, as I created Collision in a way which is independent of Components, they're not compatible right now
	// I could fix this but we only have a week left

	this->vram = vramImage(r);

	return;

}

/*
void SceneData::UpdateVram(const char* vramPath) {

	char* vramFromFile = (char*)malloc(0x18000);

	if (!fileExists(vramPath)) {
		printf("ERROR - updating VRAM failed. Provided file could not be found - '%s'.\n", vramPath);
		return;
	}

	// Open file
	fstream openVram(vramPath, ios_base::in | ios_base::out | ios_base::binary);
	openVram.seekg(0x60000, ios::beg); // go to vram + 0x60000 for reading
	openVram.read(vramFromFile, 0x18000); // 2 * 0xC000
	openVram.close();
	
	for (int i = 0; i < 0x60; i++) {
		memcpy(this->vram + i * 0x200, vramFromFile + i * 0x400, 0x200);
	}

	free(vramFromFile);

	return;
}
*/

void SceneData::UpdateVram(const char* vramPath) {

	if (!vram.hasVram) {
#ifdef _DEBUG:
		printf("DEBUG - area has no scene VRAM block.\n");
#endif
		return;
	}

	short* vramFromFile = (short*)malloc(0x80000);

	if (!fileExists(vramPath)) {
		printf("ERROR - updating VRAM failed. Provided file could not be found - '%s'.\n", vramPath);
		return;
	}

	// Open file
	fstream openVram(vramPath, ios_base::in | ios_base::out | ios_base::binary);
	openVram.seekg(0, ios::beg);
	openVram.read((char*)vramFromFile, 0x80000);
	openVram.close();

	// Copy from file
	int x = vram.x - 0x200;
	for (int i = 0; i < vram.h; i++) {
		memcpy(
			(char*)(vram.vram + (vram.w * i)),
			(char*)(vramFromFile + (0x200 * (vram.y + i)) + x),
			vram.w * 2
		);
	}

	free(vramFromFile);

	return;

}
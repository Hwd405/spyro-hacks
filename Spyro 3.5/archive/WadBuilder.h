#pragma once
#include "CommonHeader.h"

/**
* 
* WadBuilder.h
* Possibly unused tools for rebuilding WADs
* They're crude and only fit for the limited purposes they're being created here for
*
*/

class Writer {
public:
	char* address;
	unsigned int sizeLeft;

	int checkLength(unsigned int length) {
		if (length > sizeLeft) {
			printf("ERROR - BUFFER OVERFLOW IN WRITER.\n");
			return -1;
		}
		return 0;
	}
	void increment(unsigned int length) {
		if (checkLength(length) == 0) {
			address += length;
			sizeLeft -= length;
		}
		return;
	}
	void write(char* src, unsigned int length) {
		memcpy(this->address, src, length);
		increment(length);
		return;
	}
	Writer(char* address, unsigned int sizeLeft) {
		this->address = address;
		this->sizeLeft = sizeLeft;
		return;
	}
};

class SceneBuilder {
private:
	char* built = 0;
	unsigned int length = 0;
public:
	vector<unsigned int> offsets;
	vector<short> classes;
	vector<vector<char>> models;
	vector<char> preBlock;
	vector<char> postBlock;

	unsigned int getLength() {
		if (length == 0) {
			printf("ERROR - scene area not built.\n");
		}
		return length;
	}
	char* getBuild() {
		if (built == 0) {
			printf("ERROR - scene area not built.\n");
		}
		return built;
	}
	void build() {
		this->length = preBlock.size() + postBlock.size() + offsets.back();
		this->built = (char*)malloc(this->length);
		
		auto w = new Writer(this->built, this->length);

		w->write(&(preBlock[0]), preBlock.size());
		for (int i = 0; i < this->offsets.size() - 1; i++) {
			w->write(&((models[i])[0]), models[i].size());
		}
		w->write(&(postBlock[0]), postBlock.size());

#ifdef _DEBUG
		printf("DEBUG - scene build successful.\n");
#endif
		return;
	}
	void addModel(char* model, unsigned int length, short mobyClass) {
		vector<char> mdl(model, model + length);
		this->models.push_back(mdl);
		this->classes.push_back(mobyClass);
		this->offsets.push_back(this->offsets.back() + length);
		*(unsigned int*)&((this->preBlock)[0]) += length; // this is the length of the moby models block
		return;
	}
	SceneBuilder(char* scene, unsigned int length, vector<unsigned int> localOffsets, vector<short> classes) {
		if (localOffsets.size() == 0) {
			printf("ERROR - empty localOffsets list.\n");
			return;
		}
		this->preBlock = vector<char>(scene, scene + localOffsets[0]);
		this->postBlock = vector<char>(scene + localOffsets.back(), scene + length);
		this->offsets = localOffsets;
		this->classes = classes;
		for (int i = 0; i < localOffsets.size() - 1; i++) {
			vector<char> mdl(scene + localOffsets[i], scene + localOffsets[i + 1]);
			this->models.push_back(mdl);
		}
		return;
	}
};

// levelbuilder

// wadbuilder

#pragma once
#include "CommonHeader.h"
#include "json.hpp"

using json = nlohmann::json;

class MobyData {
public:
	UUID uuid; // UuidCreate(&uuid)
	Moby moby;
	vector<unsigned char> tag;
	json tagData;
	// Maybe something to identify if / when the tagData / tag have been updated so that I don't accidentally rebuild the tag incorrectly?
	// Really the tag itself shouldn't be modified if I can avoid it
	
	void ParseTagData();
};


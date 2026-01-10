#include "MobyData.h"
#include "TagFormats.h"

json ParseDialogueTag() {
	// example
	return 0;
}

json ParseGenericTag(vector<unsigned char> tag) {
	json data;
	data["rawData"] = tag;
	return data;
}

void MobyData::ParseTagData() {
	// use moby.class
	json data;


	data = ParseGenericTag(tag);
	tagData = data;
	return;
}
#include "CommonHeader.h"

int rev1offset = 0;
vector<int> strStarts;

typedef struct {
    int level;
    char typ;
    char msg;
    int strIndex;
    int channel; // also used to calculate the start
    unsigned int len;
} DialogueInfo;

vector<DialogueInfo> dialogueMsgs;

class Overlay {
public:
    char* start;
    unsigned int len;
    unsigned int startAddr;
    unsigned int key;
    unsigned int mobyAddr;
    unsigned int* mobyCode;
    unsigned int xaAddr;
    DialogueData* xaArray;
    unsigned int xaEntries;

    void decrypt() {
        unsigned int* read = (unsigned int*)(start + 0x1000);
        while ((char*)read < start + (key - startAddr) || (char*)read < start + len) {
            *read ^= key;
            read++;
        }
        return;
    }
    char* getMobyCode(short mobyClass) {
        unsigned int addr = mobyCode[mobyClass];
        return start + (addr - startAddr);
    }
    int getXaIndex(int typ, int msg) {
        for (int i = 0; i < this->xaEntries; i++) {
            if ((this->xaArray[i].typ == typ) && (this->xaArray[i].msg == msg)) {
                return i;
            }
        }
        printf("ERROR - invalid typ and msg passed to getXaIndex.\n");
        return -1;
    }
    void updateXa(DialogueInfo info) {
        DialogueData data = { info.typ, info.msg, info.channel, info.len, strStarts[info.strIndex] - 90000 + info.channel };
        updateXa(data);
        return;
    }
    void updateXa(DialogueData data) {
        int index = getXaIndex(data.typ, data.msg);
        if (index != -1) {
            this->xaArray[index] = data;
        }
        return;
    }
    void updateXa(int len, int start, int typ, int msg) {
        int index = getXaIndex(typ, msg);
        if (index != -1) {
            this->xaArray[index].length = len;
            this->xaArray[index].start = start;
        }
        return;
    }
    void updateXa(int len, int start, short channel, int typ, int msg) {
        int index = getXaIndex(typ, msg);
        if (index != -1) {
            this->xaArray[index].length = len;
            this->xaArray[index].start = start;
            this->xaArray[index].unk = channel;
        }
        return;
    }
    void removeXa(int typ, int msg) {
        int index = getXaIndex(typ, msg);
        if (index != -1) {
            char* start = (char*)&(this->xaArray[index + 1]);
            int count = this->xaEntries - (index + 1);
            if (count <= 0) {
                cout << "ERROR - nothing to move." << endl;
            }
            memmove((char*)&(this->xaArray[index]), (char*)&(this->xaArray[index + 1]), count * sizeof(DialogueData));
        }
        return;
    }
    Overlay(char* ovl, unsigned int len) {
        this->start = ovl;
        this->startAddr = overlayOffsets[gameVersion];
        this->key = *(unsigned int*)(this->start + 4);
        decrypt();
        this->mobyAddr = *(unsigned int*)(this->start + 8);
        this->mobyCode = (unsigned int*)(this->start + (this->mobyAddr - startAddr));
        this->xaAddr = *(unsigned int*)(this->start + 12);
        this->xaArray = (DialogueData*)(this->start + (this->xaAddr - startAddr));
        this->xaEntries = (this->key - this->xaAddr) / sizeof(DialogueData);
        return;
    }
};

bool compareByEggNumber(const EggData& a, const EggData& b) {
    return a.number < b.number;
}

void UpdateAtlasNames(char* atlasOvl) {

    unsigned int eggNamePointers[2] = { 0x8007aa34, 0x8007ab44 };
    unsigned int eggNames[2] = { 0x8007498c - 0xac, 0x80074a6c - 0xac }; // Starts with SBR (unused names), names are all in reverse order, 4-padded // 1.0 is usually 0x8007498c (w/o unused names), 1.1 0x80074a6c
    unsigned int eggNameEnds[2] = { 0x80075550, 0x80075630 };

    unsigned int blockLength = eggNameEnds[gameVersion] - eggNames[gameVersion];
    char* names = atlasOvl + (eggNames[gameVersion] - overlayOffsets[gameVersion]);
    char* nameBlock = (char*)calloc(blockLength, 1); // Copy this into the Atlas when it's confirmed that the buffer doesn't overflow!
    unsigned int* pointers = (unsigned int*)(atlasOvl + (eggNamePointers[gameVersion] - overlayOffsets[gameVersion]));
    unsigned int* pointerBlock = (unsigned int*)calloc(37 * 6 * 4, 1);

    char* writing = nameBlock;

    // Hacky compression implementation
    bool keyEggSet = false;
    int keyEggOffset = 0;
    bool doorEggSet = false;
    int doorEggOffset = 0;

    for (int i = eggData.size() - 1; i >= 0; i--) {

        int eggCount = eggData[i].size();

        vector<EggData> currentEggData = eggData[i];
        sort(currentEggData.begin(), currentEggData.end(), compareByEggNumber);

        for (int j = eggCount - 1; j >= 0; j--) {

            // If compressing, add something here to check if it's a "key" (etc.) string, and if so, either set the pointer to this string if it's not
            // been done before and go about it normally OR just set the pointer and continue
            if (currentEggData[j].atlasName == "Turn the key and charge.") {
                if (!keyEggSet) {
                    keyEggOffset = eggNames[gameVersion] + (writing - nameBlock);
                    keyEggSet = true;
                }
                else {
                    pointerBlock[i * 6 + j] = keyEggOffset;
                    continue;
                }
            }

            if (currentEggData[j].atlasName == "Stand on the doorframe.") {
                if (!doorEggSet) {
                    doorEggOffset = eggNames[gameVersion] + (writing - nameBlock);
                    doorEggSet = true;
                }
                else {
                    pointerBlock[i * 6 + j] = doorEggOffset;
                    continue;
                }
            }

            if (currentEggData[j].atlasName == "???") {
                pointerBlock[i * 6 + j] = eggNameEnds[gameVersion];
                continue;
            }

            pointerBlock[i * 6 + j] = eggNames[gameVersion] + (writing - nameBlock);
            
            if (writing + currentEggData[j].atlasName.size() + 1 /*null*/ > nameBlock + blockLength) {
                cout << "ERROR - could not update Atlas names - block too long - failed on level index / egg " << i << " / " << j << endl;
                return;
            }

            memcpy(writing, &((currentEggData[j].atlasName)[0]), currentEggData[j].atlasName.size());
            writing += currentEggData[j].atlasName.size();
            *writing = 0;
            writing++;
            while ((int)(writing - nameBlock) % 4 != 0) {
                *writing = 0;
                writing++;
            }

        }

    }

    // Copy both blocks in if this succeeded
    memcpy(pointers, pointerBlock, 37 * 6 * 4);
    memcpy(names, nameBlock, blockLength);

    free(nameBlock);
    free(pointerBlock);

    // "Worlds" contents pages

    /*
    TODO

    "Worlds" strings
    1.0	800755f0 - 80075650
    1.1 ?

    "Worlds" pointers
    1.0 8007aa00
    1.1 ?
    */

    return;

}

void SetVariables() {

    rev1offset = (gameVersion == 1) ? (282506 - 253783) : 0;
    strStarts = {
        298047 + rev1offset,
        303295 + rev1offset,
        306239 + rev1offset,
    };
    dialogueMsgs = {
        // SPEECH3 (-16 is pop control, fuck MC32)
        {  0,  0, 0, 0,  0, 298367 - 298047 - 16 },
        {  0,  0, 2, 0,  1, 298688 - 298048 - 16 },
        {  0,  0, 4, 0,  2, 298817 - 298049 - 16 },
        {  0,  1, 0, 0,  3, 303266 - 298050 - 16 },
        {  0,  2, 0, 0,  4, 299075 - 298051 - 16 },
        {  0,  3, 5, 0,  5, 301636 - 298052 - 16 },
        {  0, 10, 0, 0,  6, 300261 - 298053 - 16 },
        {  0, 11, 0, 0,  7, 299494 - 298054 - 16 },
        // 8
        {  0, 11, 1, 0,  8, 298663 - 298055 - 16 },
        {  0, 11, 2, 0,  9, 298792 - 298056 - 16 },
        {  1, 11, 0, 0, 10, 299241 - 298057 - 16 },
        {  1, 12, 0, 0, 11, 298730 - 298058 - 16 },
        {  2,  0, 0, 0, 12, 300107 - 298059 - 16 },
        {  2,  0, 1, 0, 13, 300364 - 298060 - 16 },
        {  2,  0, 3, 0, 14, 299405 - 298061 - 16 },
        {  2,  2, 0, 0, 15, 299278 - 298062 - 16 },
        // 16
        {  2,  6, 0, 0, 16, 299471 - 298063 - 16 },
        {  2,  7, 0, 0, 17, 299120 - 298064 - 16 },
        {  3, 11, 0, 0, 18, 299345 - 298065 - 16 },
        {  4, 11, 0, 0, 19, 299634 - 298066 - 16 },
        {  4, 12, 0, 0, 20, 299699 - 298067 - 16 },
        {  8,  3, 0, 0, 21, 301012 - 298068 - 16 },
        {  9,  2, 0, 0, (int)(stormyMode ? 23 : 22), (unsigned int)(stormyMode ? (300022 - 298070) : (300277 - 298069)) - 16 },
        //{  9,  2, 0, 0, 23, 300022 - 298070 },
        // 24
        {  9,  8, 0, 0, 24, 299095 - 298071 - 16 },
        {  9, 10, 0, 0, 25, 300760 - 298072 - 16 },
        { 10,  1, 0, 0, 26, 299705 - 298073 - 16 },
        { 10,  1, 1, 0, 27, 298810 - 298074 - 16 },
        { 10,  2, 0, 0, 28, 300507 - 298075 - 16 },
        { 10,  2, 2, 0, 29, 300636 - 298076 - 16 },
        { 10,  2, 8, 0, 30, 301181 - 298077 - 16 },
        { 10,  3, 0, 0, 31, 300126 - 298078 - 16 },


        // SPEECH4
        {  1,  5, 0, 1,  0, 303807 - 303295 - 16 },
        {  1,  5, 1, 1,  1, 304032 - 303296 - 16 },
        { 10,  3, 1, 1,  2, 304161 - 303297 - 16 },
        { 10,  3, 2, 1,  3, 304098 - 303298 - 16 },
        { 10,  3, 3, 1,  4, 305571 - 303299 - 16 },
        { 12,  2, 0, 1,  5, 304228 - 303300 - 16 },
        { 12,  2, 1, 1,  6, 305701 - 303301 - 16 },
        { 12,  2, 3, 1,  7, 304326 - 303302 - 16 },
        // 8
        { 12,  5, 0, 1,  8, 303879 - 303303 - 16 },
        { 12,  5, 1, 1,  9, 304264 - 303304 - 16 },
        { 12,  5, 2, 1, 10, 303497 - 303305 - 16 },
        { 14,  0, 2, 1, 11, 304042 - 303306 - 16 },
        { 14,  1, 0, 1, 12, 304171 - 303307 - 16 },
        { 14,  1, 1, 1, 13, 306060 - 303308 - 16 },
        { 15,  0, 0, 1, 14, 305613 - 303309 - 16 },
        { 15,  0, 1, 1, 15, 305070 - 303310 - 16 },
        // 16
        { 18,  0, 0, 1, 16, 304879 - 303311 - 16 },
        { 18,  0, 1, 1, 17, 304432 - 303312 - 16 },
        { 18,  0, 2, 1, 18, 303953 - 303313 - 16 },
        { 18,  0, 3, 1, 19, 304946 - 303314 - 16 },
        { 18, 12, 0, 1, 20, 304595 - 303315 - 16 },
        { 18, 20, 0, 1, 21, 306228 - 303316 - 16 },
        { 19,  0, 0, 1, 22, 305365 - 303317 - 16 },
        { 19,  0, 1, 1, 23, 303798 - 303318 - 16 },
        // 24
        { 19,  0, 2, 1, 24, 303735 - 303319 - 16 },
        { 19,  3, 0, 1, 25, 304216 - 303320 - 16 },
        { 19,  3, 1, 1, 26, 304057 - 303321 - 16 },
        { 19,  4, 0, 1, 27, 303962 - 303322 - 16 },
        { 19,  4, 1, 1, 28, 304091 - 303323 - 16 },
        { 19,  4, 2, 1, 29, 303676 - 303324 - 16 },
        { 19,  5, 0, 1, 30, 305533 - 303325 - 16 },
        { 19,  5, 1, 1, 31, 306014 - 303326 - 16 },


        // SPEECH5
        { 19,  5, 2, 2,  0, 307519 - 306239 - 16 },
        { 19,  5, 3, 2,  1, 308032 - 306240 - 16 },
        { 19,  5, 4, 2,  2, 306497 - 306241 - 16 },
        { 19,  6, 0, 2,  3, 308002 - 306242 - 16 },
        { 22,  1, 0, 2,  4, 309443 - 306243 - 16 },
        { 22,  1, 1, 2,  5, 309252 - 306244 - 16 },
        { 22,  1, 2, 2,  6, 307525 - 306245 - 16 },
        { 22,  1, 3, 2,  7, 306854 - 306246 - 16 },
        // 8
        { 22,  1, 4, 2,  8, 306503 - 306247 - 16 },
        { 23,  0, 1, 2,  9, 307208 - 306248 - 16 },
        { 27,  0, 0, 2, 10, 308841 - 306249 - 16 },
        { 27,  0, 1, 2, 11, 309322 - 306250 - 16 },
        { 27,  0, 2, 2, 12, 307403 - 306251 - 16 },
        { 27,  0, 3, 2, 13, 308588 - 306252 - 16 },
        { 27, 10, 0, 2, 14, 309069 - 306253 - 16 },
        { 28,  7, 0, 2, 15, 309166 - 306254 - 16 },
        // 16
        { 28,  7, 1, 2, 16, 308527 - 306255 - 16 },
        { 28,  7, 3, 2, 17, 308176 - 306256 - 16 },
        { 29,  0, 0, 2, 18, 309489 - 306257 - 16 },
        { 29,  1, 0, 2, 19, 308466 - 306258 - 16 },
        { 29,  1, 1, 2, 20, 307987 - 306259 - 16 },
        { 29,  1, 2, 2, 21, 307732 - 306260 - 16 },
        { 29,  1, 3, 2, 22, 307349 - 306261 - 16 },
        { 29,  1, 4, 2, 23, 306870 - 306262 - 16 },
        // 24
        { 30,  0, 0, 2, 24, 309015 - 306263 - 16 },
        { 30,  0, 1, 2, 25, 308408 - 306264 - 16 },
        { 30,  1, 1, 2, 26, 310169 - 306265 - 16 },
        { 30,  1, 2, 2, 27, 307674 - 306266 - 16 },
        { 30,  1, 3, 2, 28, 307643 - 306267 - 16 },
        {  3,  2, 0, 2, (int)(explorationMode ? 30 : 29), (unsigned int)(explorationMode ? (307613 - 306269) : (307996 - 306268)) - 16 },

        // Duplicates - probably not an ideal implementation as any that are updated above will need updating here too
        {  0,  0, 1, 0,  0, 298367 - 298047 - 16 }, // match 0 0
        {  0,  0, 3, 0,  1, 298688 - 298048 - 16 }, // match 0 2
        { 10,  2, 1, 0, 28, 300507 - 298075 - 16 }, // match 2 0
        { 12,  5, 3, 1, 10, 303497 - 303305 - 16 }, // match 5 2
        { 14,  1, 2, 1, 13, 306060 - 303308 - 16 }, // match 1 1
        { 19,  3, 2, 1, 26, 304057 - 303321 - 16 }, // match 3 1
        { 15,  0, 2, 1, 15, 305070 - 303310 - 16 }, // match 0 1

    };

    return;
}

void UpdateOverlays(char* exePath, char* wadPath) {

    SetVariables();

    // Open file
    fstream openWad(wadPath, ios_base::in | ios_base::out | ios_base::binary);
    streampos fileSize;
    openWad.seekg(0, ios::end);
    fileSize = openWad.tellg();
    openWad.seekg(0, ios::beg);

    // Atlas updates
    auto header = new DataHeader;
    openWad.seekg(178 * 8);
    openWad.read((char*)header, 8);
    char* atlasOvl = (char*)malloc(header->length);
    openWad.seekg(header->start);
    openWad.read(atlasOvl, header->length);
    UpdateAtlasNames(atlasOvl);
    // worlds could be updated here - names are at Atlas + 0x12FC in both versions, not sure about pointers
    openWad.seekp(header->start);
    openWad.write(atlasOvl, header->length);
    free(atlasOvl);

    // maybe make some arrays here containing everything I need plus different versions

    // Loop over the level overlays and make the necessary xa updates for each
    for (int i = 0; i < 37; i++) {

        openWad.seekg((98 + 2 * i) * 8);
        openWad.read((char*)header, 8);
        char* ovl = (char*)malloc(header->length);
        openWad.seekg(header->start);
        openWad.read(ovl, header->length);

        Overlay overlay(ovl, header->length); // decrypts as part of the constructor

        vector<DialogueData> saved;
        if (i == 19) {
            // Save 4-2 for later
            DialogueData d = overlay.xaArray[overlay.getXaIndex(4, 2)];
            saved.push_back(d);
        }

        for (int j = 0; j < dialogueMsgs.size(); j++) {
            if (dialogueMsgs[j].level == i) {
                overlay.updateXa(dialogueMsgs[j]);
            }
        }

        if (i == 1) {
            //overlay.updateXa(8, 300000, 0, 11, 1); // empty Zoe line // this is bad and it breaks things, e.g. stopping the music
            /*
            cout << overlay.xaEntries << " entries.";
            for (int i = 0; i < 50; i++) {
                cout << "Typ " << (int)overlay.xaArray[i].typ << ", msg " << (int)overlay.xaArray[i].msg << ", off " << overlay.xaArray[i].start << ", len " << overlay.xaArray[i].length << ", ch " << overlay.xaArray[i].unk << ";" << endl;
            }
            */
            overlay.removeXa(11, 1); // Zoe
            overlay.removeXa(1, 0);
        }
        else if (i == 2) {
            //overlay.updateXa({ 2, 0, 0, 299287 - 298047, 298047 - 90000 });
        }
        else if (i == 3) {
            overlay.removeXa(2, 1); // Clay second line
        }
        else if (i == 10) {
            overlay.removeXa(2, 6); // empty Nancy line

            // Nancy - done above now
            //DialogueData d = overlay.xaArray[overlay.getXaIndex(2, 0)];
            //d.msg = 1; // typ 2 msg 1 to set to typ 2 msg 0
            //overlay.updateXa(d);
        }
        else if (i == 14) {
            overlay.removeXa(1, 3);
        }
        else if (i == 19) {
            //overlay.updateXa({ 6, 0, 1, 299424 - 298048, 298048 - 90000 });

            // use 4-2 saved above
            DialogueData d = saved[0];
            d.typ = 1;
            d.msg = 0;
            overlay.updateXa(d);

        }
        else if (i == 20) {
            overlay.removeXa(0, 2); // empty Ed line
        }
        else if (i == 21) {
            overlay.removeXa(3, 0);
            overlay.removeXa(3, 1);
        }
        else if (i == 27) {

            // Not bothering with AP correction - it's a homeworld

            // Moby 958 code
            int aEncounters = 0;
            int bEncounters = 0;
            int ints = 0;
            unsigned int addr = overlay.mobyCode[958];
            unsigned int* code = (unsigned int*)overlay.getMobyCode(958);
            while (code < overlay.mobyCode) {

                if (*code == 0x28423a98) {
                    *code = 0x28420fa0;
                    aEncounters++;
                    //cout << "Made edit (958) at " << hex << addr + ints << " (15000)" << endl;
                }
                if (*code == 0x2402001d) {
                    *code = 0x2402001c;
                    bEncounters++;
                    //cout << "Made edit (958) at " << hex << addr + ints << " (1)" << endl;
                }
                if (*code == 0x24020021) {
                    *code = 0x24020020;
                    bEncounters++;
                    //cout << "Made edit (958) at " << hex << addr + ints << " (5)" << endl;
                }
                if (aEncounters > 2 && bEncounters > 1) {
                    //cout << "Escaping after " << ints << " ints" << endl;
                    break;
                }
                code++;
                ints += 4;
                if (code == overlay.mobyCode) {
                    cout << "ERROR - could not update SBR gem toll." << endl;
                }
            }

            // Moby 272 code
            aEncounters = 0;
            bEncounters = 0;
            ints = 0;
            addr = overlay.mobyCode[272];
            code = (unsigned int*)overlay.getMobyCode(272);
            while (code < overlay.mobyCode) {

                if (*code == 0x28423a98) {
                    *code = 0x28420fa0;
                    aEncounters++;
                    //cout << "Made edit (272) at " << hex << addr + ints << " (Gem)" << endl;
                }
                if (*code == 0x28420095) {
                    *code = 0x28420031;
                    bEncounters++;
                    //cout << "Made edit (272) at " << hex << addr + ints << " (Egg)" << endl;
                }
                if (aEncounters > 2 && bEncounters > 1) {
                    //cout << "Escaping after " << ints << " ints" << endl;
                    break;
                }
                code++;
                ints += 4;
                if (code == overlay.mobyCode) {
                    cout << "ERROR - could not update SBR gem toll." << endl;
                }
            }

            // Moby 41 code
            ints = 0;
            addr = overlay.mobyCode[41];
            code = (unsigned int*)overlay.getMobyCode(41);
            while (code < overlay.mobyCode) {

                if (*code == 0x28420064) {
                    *code = 0x28420050;
                    //cout << "Made edit (41) at " << hex << addr + ints << " (Sorc door)" << endl;
                    break;
                }
                code++;
                ints += 4;
                if (code == overlay.mobyCode) {
                    cout << "ERROR - could not update Sorc egg toll." << endl;
                }
            }


        }
        else if (i == 30) {
            overlay.removeXa(1, 0); // empty Ali line
        }

        overlay.decrypt(); // re-encrypts
        openWad.seekp(header->start);
        openWad.write(ovl, header->length);
        free(ovl);

    }

    // Save file strings
    int offsets[2] = { 0x38D, 0x3A5 };
    int startOffsets[2] = { 0x460, 0x478 };
    int offset = offsets[gameVersion];
    const char* newString = (explorationMode ? "SPY3_3" : "SPY3_2"); // just gonna keep the rest of it the same

    // Title updates
    openWad.seekg(1 * 8);
    openWad.read((char*)header, 8);
    openWad.seekp(header->start + offset);
    openWad.write(&(newString[5]), 1);

        // Version string
    string verString = "VER " + subversion + "-" + (explorationMode ? "E" : "C") + (gameVersion ? "1" : "0");
    int len = verString.length();
    if (len > 19) { // 20 bytes of space for the string in both versions
        cout << "ERROR - version string too long" << endl;
    }
    else {
        openWad.seekp(header->start + startOffsets[gameVersion]);
        openWad.write(&(verString[0]), len);
        char* z = (char*)calloc(20 - len, 1);
        openWad.seekp(header->start + startOffsets[gameVersion] + len);
        openWad.write(z, 20 - len);
    }

    // Common updates
    offset = 0x34D; // both versions
    openWad.seekg(69 * 8);
    openWad.read((char*)header, 8);
    openWad.seekp(header->start + offset);
    openWad.write(&(newString[5]), 1);

        // GAME OVER - 0x324
    offset = 0x324; // both versions
    openWad.seekp(header->start + offset);
    const char* gameOver = "SKILL ISSUE";
    openWad.write(&(gameOver[0]), 11);

    // Options updates
    offset = 0x2E1; // both versions
    openWad.seekg(179 * 8);
    openWad.read((char*)header, 8);
    openWad.seekp(header->start + offset);
    openWad.write(&(newString[5]), 1);

    openWad.close();
    return;

}


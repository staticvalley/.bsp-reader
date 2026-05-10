#pragma once

#include <fstream>
#include <iostream>
#include <cstdint>

/*
	using this as my guide:
	https://developer.valvesoftware.com/wiki/BSP_(GoldSrc)
*/

#define BSP_LUMP_COUNT 15

enum LumpType {
	ENTITIES		= 0,
	PLANES			= 1,
	TEXTURES		= 2,
	VERTICES		= 3,
	VISIBILITY		= 4,
	NODES			= 5,
	TEXINFO			= 6,
	FACES			= 7,
	LIGHTING		= 8,
	CLIPNODES		= 9,
	LEAVES			= 10,
	MARKSURFACES	= 11,
	EDGES			= 12,
	SURFEDGES		= 13,
	MODELS			= 14
};

struct BSPLump {
	int32_t offset;
	int32_t length;
};

struct BSPHeader {
	uint32_t versionNo;
	BSPLump lumpTable[BSP_LUMP_COUNT];
};

class BSPReader {
public:

	BSPReader(const char* path);
	~BSPReader();
	
	void printHeader();
	//void printLump(LumpType lump);
	
private:
	const char* filePath;
	std::ifstream fileStream;

	BSPHeader header;
	
	// holds file validity, false if any read fails
	bool isValid = false;
	
	// parse BSP version and lump table data into header struct
	BSPHeader processHeader();
};
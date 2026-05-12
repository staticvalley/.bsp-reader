#pragma once

#include <BSPLumpDefs.hpp>

#include <fstream>
#include <iostream>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

/*
	using this as my guide:
	https://developer.valvesoftware.com/wiki/BSP_(GoldSrc)
*/

class BSPReader {
public:

	BSPReader(const char* path);
	~BSPReader();
	
	void printHeader();
	void processLump(LumpType lump);
	void processAllLumps();
	
	// lumps
	std::vector<BSPEntity> entities;
	std::vector<BSPPlane> planes;
	std::vector<BSPTexture> textures;
	std::vector<BSPVertex> vertices;
	std::vector<BSPNode> nodes;
	std::vector<BSPTextureInfo> textureInfo;
	std::vector<BSPFace> faces;
	std::vector<uint8_t> lightMap;
	std::vector<BSPClipNode> clipNodes;
	std::vector<BSPLeaf> leaves;
	std::vector<BSPMarkSurface> markSurfaces;
	std::vector<BSPEdge> edges;
	std::vector<BSPSurfEdge> surfEdges;
	std::vector<BSPModel> models;

private:
	const char* filePath;
	std::ifstream fileStream;
	BSPHeader header{};
	
	// parse BSP version and lump table data into header struct
	BSPHeader processHeader();

	// special reader for entity lump (ascii)
	std::vector<BSPEntity> processEntityLump();

	// helper function for processEntityLump(), parses ascii for entity key-value pairs
	std::vector<BSPEntity> parseEntityMapPairs(std::string_view blob);

	std::vector<BSPTexture> processTextureLump();

	template<typename T>
	std::vector<T> processBinaryLump(LumpType type) {

		// get lump data (offset and length)
		const BSPLump& binaryLump = header.lump[type];

		// create vector of type
		std::vector<T> lumpData(binaryLump.nLength / sizeof(T));

		// read in types into vector
		fileStream.seekg(binaryLump.nOffset, std::ios::beg);
		fileStream.read((char*)lumpData.data(), binaryLump.nLength);
		if (!fileStream)
			throw std::runtime_error("failed to read lump");

		return lumpData;
	}
};

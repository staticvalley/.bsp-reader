#include <BSPReader.hpp>
#include <string>
#include <fstream>
#include <iostream>

const char* lumpTypeToString(LumpType lump) {
	switch (lump) {
	case ENTITIES:     return "ENTITIES";
	case PLANES:       return "PLANES";
	case TEXTURES:     return "TEXTURES";
	case VERTICES:     return "VERTICES";
	case VISIBILITY:   return "VISIBILITY";
	case NODES:        return "NODES";
	case TEXINFO:      return "TEXINFO";
	case FACES:        return "FACES";
	case LIGHTING:     return "LIGHTING";
	case CLIPNODES:    return "CLIPNODES";
	case LEAVES:       return "LEAVES";
	case MARKSURFACES: return "MARKSURFACES";
	case EDGES:        return "EDGES";
	case SURFEDGES:    return "SURFEDGES";
	case MODELS:       return "MODELS";
	default:           return "NONELUMP";
	}
}

BSPReader::BSPReader(const char* path) 
	: filePath(path)
{

	fileStream = std::ifstream(path, std::ios::binary);
	if (!fileStream.is_open()) {
		std::cerr << "failed to open BSP file:" << filePath << std::endl;
		return;
	}

	// read in header from file
	header = processHeader();

	// only reading bsp30 files
	if (header.versionNo != 30) {
		std::cerr << "invalid BSP version:" << header.versionNo << std::endl;
		return;
	}

	isValid = true;
}

BSPReader::~BSPReader() {

}

void BSPReader::printHeader() {
	std::cout << "filepath: " << filePath << "\n";
	std::cout << "BSP version: " << header.versionNo << "\n";
	for (int i = 0; i < BSP_LUMP_COUNT; i++) {
		std::cout << lumpTypeToString((LumpType)i) << ":\n";
		std::cout << "\tlump offset (bytes): " << header.lumpTable[i].offset << "\n";
		std::cout << "\tlump length (bytes): " << header.lumpTable[i].length << "\n";
	}
}

BSPHeader BSPReader::processHeader() {

	BSPHeader newHeader;

	// read and verify BSP version
	fileStream.read((char*)(&newHeader.versionNo), sizeof(newHeader.versionNo));

	if (!fileStream) {
		std::cerr << "error reading BSP version" << std::endl;
		return {};
	}

	// read in lump information
	fileStream.read((char*)(newHeader.lumpTable), sizeof(BSPLump) * BSP_LUMP_COUNT);

	if (!fileStream) {
		std::cerr << "error reading lump table" << std::endl;
		return {};
	}
	
	return newHeader;
}


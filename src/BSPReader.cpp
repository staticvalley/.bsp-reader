#include <BSPReader.hpp>
#include <BSPLumpDefs.hpp>

#include <string>
#include <fstream>
#include <iostream>
#include <print>
#include <unordered_map>
#include <vector>

BSPReader::BSPReader(const char* path) 
	: filePath(path)
{

	fileStream = std::ifstream(path, std::ios::binary);
	if (!fileStream.is_open())
		throw std::runtime_error(std::format("failed to open BSP file: {}", filePath));

	// read in header from file
	header = processHeader();

	// only reading bsp30 files
	if (header.nVersion != 30)
		throw std::runtime_error(std::format("invalid BSP version: {}", header.nVersion));
}

BSPReader::~BSPReader() {}

void BSPReader::printHeader() {

	auto lumpTypeToString = [&](LumpType lump) {
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
	};

	std::println("filepath: \"{}\" \n.bsp version: {}", filePath, header.nVersion);

	std::println("{:<20} {:<20} {:<20}", "lump type", "lump offset", "lump length");
	for (int i = 0; i < BSP_LUMP_COUNT; i++) {
		std::println("{:<20} {:<20} {:<20}", 
			lumpTypeToString((LumpType)i), 
			header.lump[i].nOffset, 
			header.lump[i].nLength
		);
	}
}

BSPHeader BSPReader::processHeader() {

	BSPHeader newHeader;

	// read and verify BSP version
	fileStream.read((char*)(&newHeader.nVersion), sizeof(newHeader.nVersion));

	if (!fileStream) {
		std::cerr << "error reading BSP version" << std::endl;
		return {};
	}

	// read in lump information
	fileStream.read((char*)(newHeader.lump), sizeof(BSPLump) * BSP_LUMP_COUNT);

	if (!fileStream) {
		std::cerr << "error reading lump table" << std::endl;
		return {};
	}
	
	return newHeader;
}

void BSPReader::processAllLumps() {
	for (int i = 0; i < BSP_LUMP_COUNT; i++)
		processLump((LumpType)i);
}

void BSPReader::processLump(LumpType lump) {
	switch (lump) {
	case ENTITIES:		entities = processEntityLump(); break;
	case PLANES:		planes = processBinaryLump<BSPPlane>(lump); break;
	case TEXTURES:		textures = processTextureLump(); break;
	case VERTICES:		vertices = processBinaryLump<BSPVertex>(lump); break;
	case VISIBILITY:	break; // note: unimplemented
	case NODES:			nodes = processBinaryLump<BSPNode>(lump); break;
	case TEXINFO:		textureInfo = processBinaryLump<BSPTextureInfo>(lump); break;
	case FACES:			faces = processBinaryLump<BSPFace>(lump); break;
	case LIGHTING:		lightMap = processBinaryLump<uint8_t>(lump); break;
	case CLIPNODES:		clipNodes = processBinaryLump<BSPClipNode>(lump); break;
	case LEAVES:		leaves = processBinaryLump<BSPLeaf>(lump); break;
	case MARKSURFACES:	markSurfaces = processBinaryLump<BSPMarkSurface>(lump); break;
	case EDGES:			edges = processBinaryLump<BSPEdge>(lump); break;
	case SURFEDGES:		surfEdges = processBinaryLump<BSPSurfEdge>(lump); break;
	case MODELS:		models = processBinaryLump<BSPModel>(lump); break;
	default:			break;
	}
}

std::vector<BSPEntity> BSPReader::processEntityLump() {

	// get lump data (offset and length)
	const BSPLump& entityLump = header.lump[LumpType::ENTITIES];

	// offset stream to lump start
	fileStream.seekg(entityLump.nOffset, std::ios::beg);

	// read in entire lump to a string
	std::string entityBlob(entityLump.nLength, '\0');
	fileStream.read(entityBlob.data(), entityLump.nLength);
	if (!fileStream)
		throw std::runtime_error("failed to read entity lump");

	return parseEntityMapPairs(entityBlob);
}

std::vector<BSPEntity> BSPReader::parseEntityMapPairs(std::string_view blob) {
	
	std::vector<BSPEntity> processedEntities;
	BSPEntity currentEntity;
	uint32_t index = 0;
	std::string key, value;

	// define lambda functions for reading in hashmap pairs
	auto skipWhitespace = [&]() {
		while (index < blob.size() && std::isspace(blob[index])) index++;
	};

	// reads string within quotes, ie "class_name" would return class_name
	auto readQuotes = [&]() -> std::string {
		// skip first quote
		index++; 
		uint32_t beggining = index;
		// read string
		while (index < blob.size() && blob[index] != '"') index++;
		std::string innerString(blob.substr(beggining, index - beggining));
		// skip end quote
		index++;
		return innerString;
	};

	while (index < blob.size()) {
		
		skipWhitespace();

		if (index >= blob.size()) break;

		switch (blob[index]) {
			case '{':
				// new entity
				index++;
				currentEntity.clear();
				break;
			case '}':
				// end of entity hash pairs
				index++;
				processedEntities.push_back(currentEntity);
				break;
			case '"':
				// read key-value pair for entity
				key = readQuotes();
				skipWhitespace();
				value = readQuotes();
				currentEntity[std::move(key)] = std::move(value);
				break;
			default:
				// advance next character
				index++;
		}
	}

	return processedEntities;
}

std::vector<BSPTexture> BSPReader::processTextureLump() {

	// holds number of mipmap textures
	uint32_t nMipTextures = 0;

	// mipmap texture offsets from lump
	std::vector<BSPMipTextureOffset> offsets;

	// textures
	std::vector<BSPTexture> textureList;

	// get lump data (offset and length)
	const BSPLump& textureLump = header.lump[LumpType::TEXTURES];

	// offset stream to lump start
	fileStream.seekg(textureLump.nOffset, std::ios::beg);

	// read texture count (header)
	fileStream.read((char*)&nMipTextures, sizeof(uint32_t));
	if(!fileStream)
		throw std::runtime_error("failed to read texture lump");

	// read in all offsets of Mipmap Textures
	for (size_t i = 0; i < nMipTextures; i++) {

		BSPMipTextureOffset currentOffset;

		// read offset
		fileStream.read((char*)&currentOffset, sizeof(BSPMipTextureOffset));
		if (!fileStream)
			throw std::runtime_error("failed to read texture lump");

		offsets.push_back(currentOffset);
	}

	// read in all offsets of mipmap Textures
	for (size_t i = 0; i < nMipTextures; i++) {

		// raw parsed data from BSP file
		BSPMipTexture textureParse{};

		// final texture with pixel data attached
		BSPTexture currentTexture{};

		// null texture
		if (offsets[i] < 0)
			throw std::runtime_error("invalid texture offset");

		// bounds check
		if (offsets[i] + (int32_t)sizeof(BSPMipTexture) > textureLump.nLength)
			throw std::runtime_error("invalid texture offset");

		// offset stream to lump start + miptexture offset
		fileStream.seekg(textureLump.nOffset + offsets[i], std::ios::beg);

		// read texture parse struct
		fileStream.read((char*)&textureParse, sizeof(BSPMipTexture));
		if (!fileStream)
			throw std::runtime_error("failed to read texture lump");

		// texture dimension checks
		if (textureParse.nWidth == 0 || textureParse.nHeight == 0 ||
			textureParse.nWidth > 4096 || textureParse.nHeight > 4096)
			throw std::runtime_error("invalid texture dimensions");

		// copy dimensions
		currentTexture.height = textureParse.nHeight;
		currentTexture.width = textureParse.nWidth;

		// safe copy of string
		std::copy(textureParse.szName, textureParse.szName + BSP_MAX_TEXTURE_NAME, currentTexture.name);
		currentTexture.name[BSP_MAX_TEXTURE_NAME - 1] = '\0';

		// external texture
		if (textureParse.nOffsets[0] == 0) {
			currentTexture.external = true;
			textureList.push_back(currentTexture);
			continue;
		}

		uint32_t textureSize = currentTexture.width * currentTexture.height;
		std::vector<uint8_t> pixels(textureSize);

		// pixel data bounds check
		uint32_t pixelStart = offsets[i] + textureParse.nOffsets[0];
		if (pixelStart + textureSize > textureLump.nLength)
			throw std::runtime_error("texture pixel data out of bounds");

		// offset stream to lump start + miptexture offset + pixel data offset
		fileStream.seekg(textureLump.nOffset + offsets[i] + textureParse.nOffsets[0], std::ios::beg);

		// read and move texture pixel data
		fileStream.read((char*)(pixels.data()), sizeof(uint8_t) * textureSize);
		currentTexture.pixels = std::move(pixels);

		// 1/8th mipmap size
		uint32_t mip3Size = (textureParse.nWidth / 8) * (textureParse.nHeight / 8);

		// get offset of palette data
		uint32_t paletteOffset = textureLump.nOffset + offsets[i]+ textureParse.nOffsets[3] + mip3Size;

		// skip by 16 bytes (past palatte type specifier) 
		fileStream.seekg(paletteOffset + sizeof(uint16_t), std::ios::beg);

		// read in palette data
		fileStream.read((char*)currentTexture.palette.data(), 768);

		textureList.push_back(currentTexture);
	}

	return textureList;
}
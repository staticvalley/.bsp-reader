#pragma once

#include <unordered_map>
#include <string>
#include <array>
#include <vector>

#define BSP_LUMP_COUNT 15

enum LumpType {
	ENTITIES = 0,
	PLANES = 1,
	TEXTURES = 2,
	VERTICES = 3,
	VISIBILITY = 4,
	NODES = 5,
	TEXINFO = 6,
	FACES = 7,
	LIGHTING = 8,
	CLIPNODES = 9,
	LEAVES = 10,
	MARKSURFACES = 11,
	EDGES = 12,
	SURFEDGES = 13,
	MODELS = 14
};

struct Vector3D {
	float x, y, z;
};

struct BSPLump {
	int32_t nOffset;
	int32_t nLength;
};

struct BSPHeader {
	uint32_t nVersion;
	BSPLump lump[BSP_LUMP_COUNT];
};

// lump 0: entities

using BSPEntity = std::unordered_map<std::string, std::string>;

// lump 1: planes

enum class PlaneType : uint32_t {
	PLANE_X = 0, // plane is perpendicular to given axis
	PLANE_Y = 1,
	PLANE_Z = 2,
	PLANE_ANYX = 3, // non-axial plane is snapped to the nearest
	PLANE_ANYY = 4,
	PLANE_ANYZ = 5,
};

struct BSPPlane {
	Vector3D vNormal; // the planes normal vector
	float fDist; // plane equation is: vNormal * X = fDist
	PlaneType nType;
};

// lump 2: textures
// note: if < 0, offset is invalid/null
typedef int32_t BSPMipTextureOffset;

#define BSP_MAX_TEXTURE_NAME 16
#define BSP_MIP_LEVELS 4

// for internal processing only
struct BSPMipTexture
{
	char szName[BSP_MAX_TEXTURE_NAME];  // name of texture
	uint32_t nWidth, nHeight; // extends of the texture
	uint32_t nOffsets[BSP_MIP_LEVELS]; // offsets to texture mipmaps BSPMIPTEX;
};

#define MAX_TEXTURE_DIMENSIONS 4096
// struct holding texture data stored in BSP
struct BSPTexture {
	char name[BSP_MAX_TEXTURE_NAME];	// name of texture
	uint32_t width = 0, height = 0;		// dimensions
	std::vector<uint8_t> pixels;		// raw texture pixel data
	std::array<uint8_t, 768> palette{}; // palette data
	bool external = false;				// if texture data is in an external file (ie. ".wad")
};

// lump 3: vertices
typedef Vector3D BSPVertex;

// lump 4: visibility
// note: unimplemented

// lump 5: nodes
struct BSPNode {
	uint32_t iPlane;            // index into Planes lump
	int16_t iChildren[2];       // if >= 0, then indices into nodes // otherwise bitwise inverse indices into leafs
	int16_t nMins[3], nMaxs[3]; // defines bounding box
	uint16_t firstFace, nFaces; // index and count into Faces
};

// lump 6: texture information
struct BSPTextureInfo {
	Vector3D vS;
	float fSShift;    // texture shift in s direction
	Vector3D vT;
	float fTShift;    // texture shift in t direction
	uint32_t iMiptex; // index into textures array
	uint32_t nFlags;  // texture flags
};

// lump 7: faces
struct BSPFace {
	uint16_t iPlane;          // plane the face is parallel to
	uint16_t nPlaneSide;      // set if different normals orientation
	uint32_t iFirstEdge;      // index of the first surfedge
	uint16_t nEdges;          // number of consecutive surfedges
	uint16_t iTextureInfo;    // index of the texture info structure
	uint8_t nStyles[4];       // specify lighting styles
	int32_t nLightmapOffset;  // offsets into the raw lightmap data; if less than zero, then a lightmap was not baked for the given face.
};

// lump 8: lightmap
// implemented as a single uint_8 vector for rgb values

// lump 9: clip nodes
struct BSPClipNode {
	int32_t iPlane;       // index into planes
	int16_t iChildren[2]; // negative numbers are contents
};

// lump 10: leaf nodes
struct BSPLeaf {
	int32_t nContents;                         // contents enumeration
	int32_t nVisOffset;                        // offset into the visibility lump
	int16_t nMins[3], nMaxs[3];                // defines bounding box
	uint16_t iFirstMarkSurface, nMarkSurfaces; // index and count into marksurfaces array
	uint8_t nAmbientLevels[4];                 // ambient sound levels
};

// lump 11: marksurfaces
typedef uint16_t BSPMarkSurface;

// lump 12: edges
struct BSPEdge {
	uint16_t iVertex[2]; // indices into vertex array
};

// lump 13: surfedges
typedef int32_t BSPSurfEdge;

// lump 14: models

#define BSP_MAX_MAP_HULLS 4
struct BSPModel {
	float nMins[3], nMaxs[3];				// defines bounding box
	Vector3D vOrigin;						// coordinates to move the // coordinate system
	int32_t iHeadnodes[BSP_MAX_MAP_HULLS];	// index into nodes array
	int32_t nVisLeafs;						// ???
	int32_t iFirstFace, nFaces;				// index and count into faces
};
#pragma once

#include <engine/core/common.hpp>
#include <engine/core/memory.hpp>

#include <engine/math/matrix.hpp>

class IndexedModel;
class VertexArray;
class Material;

struct BlockInfo {
	enum BlockType {
		TYPE_BASIC_CUBE,
		TYPE_BASIC_TETRA,
		TYPE_BASIC_PYRAMID,
		TYPE_BASIC_WEDGE,
		TYPE_BASIC_FIVE_SIXTH,
		TYPE_BASIC_WEDGE_2X1,
		TYPE_BASIC_WEDGE_2X2,

		NUM_TYPES
	};

	enum Flags {
		FLAG_OCCLUDES	= 0b1
	};

	inline BlockInfo()
			: type((enum BlockType)0)
			, flags(0)
			, model(nullptr)
			, vertexArray(nullptr)
			, material(nullptr) {}

	inline BlockInfo(enum BlockType type, uint32 flags, IndexedModel* model,
				Material* material,
				Memory::SharedPointer<VertexArray> vertexArray)
			: type(type)
			, flags(flags)
			, model(model)
			, vertexArray(vertexArray)
			, material(material) {}

	enum BlockType type;
	uint32 flags;

	IndexedModel* model;
	Material* material;
	Memory::SharedPointer<VertexArray> vertexArray;
};

struct Block {
	enum BlockInfo::BlockType type;
	Vector3i position;
	Vector2i rotation;
	uint32 renderIndex;
};


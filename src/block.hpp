#pragma once

#include <engine/core/common.hpp>
#include <engine/core/array-list.hpp>

#include <engine/math/matrix.hpp>

class IndexedModel;
class VertexArray;
class Material;

class BlockInfo {
	public:
		static constexpr const float OFFSET_SCALE = 2.f;

		enum BlockType {
			TYPE_BASIC_CUBE,
			TYPE_BASIC_TETRA,
			TYPE_BASIC_PYRAMID,
			TYPE_BASIC_WEDGE,
			TYPE_BASIC_FIVE_SIXTH,

			NUM_TYPES
		};

		static void registerType(enum BlockType type, const Vector3i& occupancy,
				IndexedModel& model, VertexArray& vertexArray, Material& material);

		inline static const BlockInfo& getInfo(enum BlockType type) { return blockInfo[type]; }

		const enum BlockType type;
		const Vector3i occupancy;
		const IndexedModel* model;
		const VertexArray* vertexArray;
		const Material* material;
	private:
		static ArrayList<BlockInfo> blockInfo;

		inline BlockInfo()
				: type((enum BlockType)0)
				, occupancy(0, 0, 0)
				, model(nullptr)
				, vertexArray(nullptr)
				, material(nullptr) {}

		inline BlockInfo(enum BlockType type, const Vector3i& occupancy,
					IndexedModel* model, VertexArray* vertexArray,
					Material* material)
				: type(type)
				, occupancy(occupancy)
				, model(model)
				, vertexArray(vertexArray)
				, material(material) {}
};

struct Block {
	enum BlockInfo::BlockType type;
	Matrix4f offset;
};


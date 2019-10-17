#pragma once

#include <engine/core/common.hpp>
#include <engine/core/memory.hpp>
#include <engine/core/array-list.hpp>

#include <engine/math/quaternion.hpp>

class IndexedModel;
class VertexArray;
class Material;

class RenderContext;

class BlockInfo {
	public:
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

		static void registerType(enum BlockInfo::BlockType type, uint32 flags,
				IndexedModel* model, Material* material, float mass,
				float volume);

		static void initVertexArrays(RenderContext& renderContext,
				ArrayList<Memory::SharedPointer<VertexArray>>& vaos);

		static inline BlockInfo& getInfo(enum BlockInfo::BlockType type) {
			return blockInfo[(uintptr)type];
		}

		enum BlockType type;
		uint32 flags;

		IndexedModel* model;
		Material* material;

		float mass;
		float volume;
	private:
		inline BlockInfo()
				: type((enum BlockType)0)
				, flags(0)
				, model(nullptr)
				, material(nullptr)
				, mass(0.f)
				, volume(0.f) {}

		inline BlockInfo(enum BlockType type, uint32 flags,
					IndexedModel* model, Material* material, float mass,
					float volume)
				: type(type)
				, flags(flags)
				, model(model)
				, material(material)
				, mass(mass)
				, volume(volume) {}

		static ArrayList<BlockInfo> blockInfo;
};

struct Block {
	enum BlockInfo::BlockType type;
	Vector3i position;
	Quaternion rotation;
	uint32 renderIndex;
};


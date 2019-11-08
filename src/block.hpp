#pragma once

#include <engine/core/common.hpp>
#include <engine/core/memory.hpp>
#include <engine/core/string.hpp>
#include <engine/core/array-list.hpp>

#include <engine/math/quaternion.hpp>

class Game;

class IndexedModel;
class VertexArray;
class Material;

class RenderContext;

class BlockInfo {
	public:
		enum Flags {
			FLAG_OCCLUDES	= 0b1
		};

		static void loadBlockInfo(Game& game, const String& fileName);

		static void registerType(uint32 type, uint32 flags,
				IndexedModel* model, Material* material, float mass,
				float volume);

		static void initVertexArrays(RenderContext& renderContext,
				ArrayList<Memory::SharedPointer<VertexArray>>& vaos);

		static inline BlockInfo& getInfo(uint32 type) {
			return blockInfo[(uintptr)type];
		}

		static inline uint32 getNumTypes() { return blockInfo.size(); }

		uint32 type;
		uint32 flags;

		IndexedModel* model;
		Material* material;

		float mass;
		float volume;
	private:
		inline BlockInfo()
				: type((uint32)0)
				, flags(0)
				, model(nullptr)
				, material(nullptr)
				, mass(0.f)
				, volume(0.f) {}

		inline BlockInfo(uint32 type, uint32 flags,
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
	uint32 type;
	Vector3i position;
	Quaternion rotation;
	uint32 renderIndex;
};


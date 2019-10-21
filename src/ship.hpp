#pragma once

#include <engine/core/memory.hpp>
#include <engine/core/array-list.hpp>
#include <engine/core/hash-set.hpp>
#include <engine/core/hash-map.hpp>

#include "block.hpp"
#include "block-tree.hpp"

class Game;

struct HashBlockPosition {
	inline std::size_t operator()(const Vector3i& v) const {
		//return ( ((std::size_t)(v.x) & 0xFF) << 16 )
		//		| ( ((std::size_t)(v.y) & 0xFF) << 8 )
		//		| ( ((std::size_t)(v.z) & 0xFF) );
		return (v.x * 31) + (v.y * 37) + (v.z * 41);
	}
};

class Ship {
	public:
		inline Ship()
				: totalMass(0.f)
				, localCenterSum(0.f, 0.f, 0.f)
				, inertiaSum(0.f)
				, massChanged(false) {}

		void addBlock(enum BlockInfo::BlockType type,
				const Vector3i& position, const Quaternion& rotation);
		void removeBlock(Block& block);

		HashMap<Vector3i, Block, HashBlockPosition> blocks;
		HashMap<enum BlockInfo::BlockType, ArrayList<Matrix4f>> offsets;
		HashMap<enum BlockInfo::BlockType, ArrayList<Block*>> offsetIndices;
		BlockTreeNode blockTree;

		ArrayList<Memory::SharedPointer<VertexArray>> blockArrays;
		
		HashSet<enum BlockInfo::BlockType> changedBuffers;

		float totalMass;
		Vector3f localCenterSum;
		Matrix3f inertiaSum;
		bool massChanged;
};

void rayShipIntersection(const Matrix4f& shipTransform, const Ship& ship,
		const Vector3f& origin, const Vector3f& direction, Block*& block,
		Vector3f* hitPosition, Vector3f* hitNormal);

void shipBuoyancySystem(Game& game, float deltaTime);

void shipUpdateMassSystem(Game& game, float deltaTime);

void shipUpdateVAOSystem(Game& game, float deltaTime);
void shipRenderSystem(Game& game, float deltaTime);


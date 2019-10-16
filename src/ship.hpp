#pragma once

#include <engine/core/array-list.hpp>
#include <engine/core/hash-map.hpp>

#include "block.hpp"
#include "ray-tree-node.hpp"

class Game;

struct HashBlockPosition {
	inline std::size_t operator()(const Vector3i& v) const {
		//return ( ((std::size_t)(v.x) & 0xFF) << 16 )
		//		| ( ((std::size_t)(v.y) & 0xFF) << 8 )
		//		| ( ((std::size_t)(v.z) & 0xFF) );
		return (v.x * 31) + (v.y * 37) + (v.z * 41);
	}
};

struct Ship {
	HashMap<Vector3i, Block, HashBlockPosition> blocks;
	HashMap<enum BlockInfo::BlockType, ArrayList<Matrix4f>> offsets;
	HashMap<enum BlockInfo::BlockType, ArrayList<Block*>> offsetIndices;
	RayTreeNode hitTree;

	ArrayList<BlockInfo> blockInfo;
};

void rayShipIntersection(const Matrix4f& shipTransform, const Ship& ship,
		const Vector3f& origin, const Vector3f& direction, Block*& block,
		Vector3f* hitPosition, Vector3f* hitNormal);

void calcMassData(const Ship& ship, float& mass, float& invMass,
		Vector3f& localCenter, Matrix3f& inertia);

void shipRenderSystem(Game& game, float deltaTime);


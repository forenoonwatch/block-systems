#pragma once

#include <engine/core/hash-map.hpp>

#include "block.hpp"

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
};

void shipRenderSystem(Game& game, float deltaTime);


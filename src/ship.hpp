#pragma once

#include <engine/core/memory.hpp>
#include <engine/core/array-list.hpp>
#include <engine/core/hash-set.hpp>
#include <engine/core/hash-map.hpp>

#include <engine/ecs/ecs-system.hpp>

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
				: blockTree(5, 4)
				, totalMass(0.f)
				, localCenterSum(0.f, 0.f, 0.f)
				, inertiaSum(0.f)
				, massChanged(false) {}

		bool load(const String& fileName);

		void addBlock(uint32 type,
				const Vector3i& position, const Quaternion& rotation);
		void removeBlock(Block& block);

		HashMap<Vector3i, Block, HashBlockPosition> blocks;
		HashMap<uint32, ArrayList<Matrix4f>> offsets;
		HashMap<uint32, ArrayList<Block*>> offsetIndices;
		
		BlockTree blockTree;

		ArrayList<Memory::SharedPointer<VertexArray>> blockArrays;
		
		HashSet<uint32> changedBuffers;

		float totalMass;
		Vector3f localCenterSum;
		Matrix3f inertiaSum;
		bool massChanged;
};

void rayShipIntersection(const Matrix4f& shipTransform, const Ship& ship,
		const Vector3f& origin, const Vector3f& direction, Block*& block,
		Vector3f* hitPosition, Vector3f* hitNormal);

class ShipBuoyancySystem : public ECS::System {
	public:
		virtual void operator()(Game& game, float deltaTime) override;
};

class ShipUpdateMassSystem : public ECS::System {
	public:
		virtual void operator()(Game& game, float deltaTime) override;
};

class ShipUpdateVAOSystem : public ECS::System {
	public:
		virtual void operator()(Game& game, float deltaTime) override;
};

class ShipRenderSystem : public ECS::System {
	public:
		virtual void operator()(Game& game, float deltaTime) override;
};


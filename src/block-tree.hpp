#pragma once

#include <engine/core/common.hpp>
#include <engine/core/array-list.hpp>

#include <engine/math/aabb.hpp>

class Block;

class BlockTreeNode {
	public:
		static constexpr const uint32 MAX_DEPTH = 5;
		static constexpr const uint32 MAX_OBJECTS = 20;

		BlockTreeNode(const Vector3f& minExtents = Vector3f(-100, -100, -100),
				const Vector3f& maxExtents = Vector3f(100, 100, 100),
				uint32 level = 0, BlockTreeNode* parent = nullptr);

		bool intersectsRay(const Vector3f& origin, const Vector3f& direction,
				Vector3i* intersectCoord, Vector3f* intersectPos) const;

		bool belowPlane(const Vector3f& position, const Vector3f& normal,
				float& volumeBelow, Vector3f& centerSumBelow,
				float& numBlocksBelow) const;

		bool add(const Block& block);
		bool remove(const Block& block);

		inline float getTotalVolume() const { return totalVolume; }
		inline float getTotalBlocks() const { return totalBlocks; }
		inline const Vector3f& getCenterSum() const { return centerSum; }

		~BlockTreeNode();
	private:
		AABB aabb;

		uint32 level;

		BlockTreeNode* parent;
		BlockTreeNode* children[8];

		AABB childAABBs[8];

		bool limitReached;

		float totalVolume;
		float totalBlocks;
		Vector3f centerSum;

		ArrayList<const Block*> blocks;

		bool childrenBelowPlane(const Vector3f& position,
				const Vector3f& normal, float& volumeBelow,
				Vector3f& centerSumBelow, float& numBlocksBelow) const;
};


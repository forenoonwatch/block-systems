#pragma once

#include <engine/core/common.hpp>
#include <engine/core/array-list.hpp>

#include <engine/math/aabb.hpp>

class Block;

class BlockTreeNode {
	public:
		BlockTreeNode(const Vector3f& minExtents, const Vector3f& maxExtents,
				uint32 maxDepth, uint32 maxObjects, uint32 level = 0,
				BlockTreeNode* parent = nullptr);

		bool intersectsRay(const Vector3f& origin, const Vector3f& direction,
				Vector3i* intersectCoord, Vector3f* intersectPos) const;

		bool belowPlane(const Vector3f& position, const Vector3f& normal,
				Vector3f& centerSumBelow, float& volumeBelow,
				float& massBelow) const;

		bool add(const Block& block);
		bool remove(const Block& block);

		inline const AABB& getAABB() const { return aabb; }

		inline BlockTreeNode** getChildren() { return children; }

		inline float getTotalVolume() const { return totalVolume; }
		inline float getTotalMass() const { return totalMass; }
		inline const Vector3f& getCenterSum() const { return centerSum; }

		~BlockTreeNode();
	private:
		AABB aabb;

		uint32 level;

		uint32 maxDepth;
		uint32 maxObjects;

		BlockTreeNode* parent;
		BlockTreeNode* children[8];

		AABB childAABBs[8];

		bool limitReached;

		float totalVolume;
		float totalMass;
		Vector3f centerSum;

		ArrayList<const Block*> blocks;

		bool childrenBelowPlane(const Vector3f& position,
				const Vector3f& normal, Vector3f& centerSumBelow,
				float& volumeBelow, float& massBelow) const;
};


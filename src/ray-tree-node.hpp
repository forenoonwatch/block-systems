#pragma once

#include <engine/core/common.hpp>
#include <engine/core/array-list.hpp>

#include <engine/math/aabb.hpp>

class RayTreeNode {
	public:
		static constexpr const uint32 MAX_DEPTH = 5;
		static constexpr const uint32 MAX_OBJECTS = 20;

		RayTreeNode(const Vector3f& minExtents = Vector3f(0, 0, 0),
				const Vector3f& maxExtents = Vector3f(100, 100, 100),
				uint32 level = 0, RayTreeNode* parent = nullptr);

		bool intersectsRay(const Vector3f& origin, const Vector3f& direction,
				Vector3i* intersectCoord, Vector3f* intersectPos) const;

		bool addObject(const Vector3i& coord);
		bool removeObject(const Vector3i& coord);

		~RayTreeNode();
	private:
		AABB aabb;

		uint32 level;

		RayTreeNode* parent;
		RayTreeNode* children[8];

		AABB childAABBs[8];

		bool limitReached;

		ArrayList<Vector3i> coords;
};


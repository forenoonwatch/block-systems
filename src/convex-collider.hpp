#pragma once

#include "collision-hull.hpp"

#include <engine/core/array-list.hpp>
#include <engine/math/vector.hpp>

namespace Physics {
	class ConvexCollider : public CollisionHull {
		public:
			ConvexCollider();

			virtual AABB computeAABB(const Transform& tf) const override;
		private:
			ArrayList<Vector3f> vertices;
			ArrayList<Vector3f> normals;
			ArrayList<Vector3f> edges;
	};
};


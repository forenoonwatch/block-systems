#pragma once

#include "collision-hull.hpp"

namespace Physics {
	class ConvexCollider : public CollisionHull {
		public:
			inline ConvexCollider()
					: CollisionHull(CollisionHull::TYPE_CONVEX_HULL) {}

			virtual AABB computeAABB(const Transform& tf) const override;
		private:
	};
};


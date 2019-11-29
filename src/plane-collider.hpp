#pragma once

#include "collision-hull.hpp"

namespace Physics {
	class PlaneCollider : public CollisionHull {
		public:
			inline PlaneCollider()
					: CollisionHull(CollisionHull::TYPE_PLANE) {}

			virtual AABB computeAABB(const Transform& tf) const override;
		private:
	};
};


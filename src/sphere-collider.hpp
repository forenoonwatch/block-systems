#pragma once

#include "collision-hull.hpp"

namespace Physics {
	class SphereCollider : public CollisionHull {
		public:
			inline SphereCollider()
					: CollisionHull(CollisionHull::TYPE_SPHERE) {}

			float radius;
	};
};


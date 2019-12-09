#pragma once

#include "collision-hull.hpp"

namespace Physics {
	class SphereCollider : public CollisionHull {
		public:
			inline SphereCollider(float radius)
					: CollisionHull(CollisionHull::TYPE_SPHERE)
					, radius(radius) {}

			virtual AABB computeAABB(const Transform& tf) const override;

			inline void setRadius(float radius) {
				this->radius = radius;
			}

			inline float getRadius() const { return radius; }
		private:
			NULL_COPY_AND_ASSIGN(SphereCollider);

			float radius;
	};
};


#pragma once

#include "collider.hpp"

namespace Physics {
	class SphereCollider : public Collider {
		public:
			inline SphereCollider(const ColliderHints& hints)
					: Collider(hints)
					, radius(hints.getRadius()) {}

			virtual AABB computeAABB() const override;
			
			virtual void calcMassData(float& mass, Matrix3f& inertia,
					Vector3f& centerOfMass) const override;

			inline void setRadius(float radius) {
				this->radius = radius;
			}

			inline float getRadius() const { return radius; }
		private:
			NULL_COPY_AND_ASSIGN(SphereCollider);

			float radius;
	};
};


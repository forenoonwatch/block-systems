#pragma once

#include "collider.hpp"

namespace Physics {
	class PlaneCollider : public Collider {
		public:
			inline PlaneCollider(const ColliderHints& hints)
					: Collider(hints) {}

			virtual AABB computeAABB() const override;

			virtual void calcMassData(float& mass, Matrix3f& inertia,
					Vector3f& centerOfMass) const override;
		private:
	};
};


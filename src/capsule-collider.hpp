#pragma once

#include "collider.hpp"

namespace Physics {
	class CapsuleCollider : public Collider {
		public:
			CapsuleCollider(const ColliderHints& hints);

			virtual AABB computeAABB() const override;

			virtual void calcMassData(float& mass, Matrix3f& inertia,
					Vector3f& centerOfMass) const override;

			inline const Vector3f getP0() const { return points[0]; }
			inline const Vector3f getP1() const { return points[1]; }

			inline const Vector3f* getPoints() const { return points; }

			inline float getRadius() const { return radius; }
		private:
			NULL_COPY_AND_ASSIGN(CapsuleCollider);
			
			Vector3f points[2];

			float radius;

			AABB baseAABB;
	};
};


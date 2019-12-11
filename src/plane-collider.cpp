#include "plane-collider.hpp"

#define EXTENT_DIST	1e6f
#define THICKNESS	0.1f

AABB Physics::PlaneCollider::computeAABB() const {
	return AABB(worldTransform.getPosition()
			- Vector3f(EXTENT_DIST, THICKNESS, EXTENT_DIST),
			worldTransform.getPosition()
			+ Vector3f(EXTENT_DIST, THICKNESS, EXTENT_DIST));
}

void Physics::PlaneCollider::calcMassData(float& mass, Matrix3f& inertia,
		Vector3f& centerOfMass) const {
	mass = 0.f;
	inertia = Matrix3f(0.f);
	centerOfMass = localTransform.getPosition();
}


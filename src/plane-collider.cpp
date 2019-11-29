#include "plane-collider.hpp"

#define EXTENT_DIST	1e6f
#define THICKNESS	0.1f

AABB Physics::PlaneCollider::computeAABB(const Transform& tf) const {
	return AABB(tf.getPosition()
			- Vector3f(EXTENT_DIST, THICKNESS, EXTENT_DIST), tf.getPosition()
			+ Vector3f(EXTENT_DIST, THICKNESS, EXTENT_DIST));
}


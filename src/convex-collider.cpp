#include "convex-collider.hpp"

#define EXTENT_DIST	1e6f
#define THICKNESS	0.1f

AABB Physics::ConvexCollider::computeAABB(const Transform& tf) const {
	// TODO: make this actually reflect a convex collider and not a plane
	
	return AABB(tf.getPosition()
			- Vector3f(EXTENT_DIST, THICKNESS, EXTENT_DIST), tf.getPosition()
			+ Vector3f(EXTENT_DIST, THICKNESS, EXTENT_DIST));
}


#include "sphere-collider.hpp"

AABB Physics::SphereCollider::computeAABB(const Transform& tf) const {
	return AABB(tf.getPosition() - Vector3f(radius),
			tf.getPosition() + Vector3f(radius));
}


#include "capsule-collider.hpp"

Physics::CapsuleCollider::CapsuleCollider(const Vector3f& p0,
			const Vector3f& p1, float radius)
		: Collider(Collider::TYPE_CAPSULE)
		, points({p0, p1})
		, radius(radius) {
	Vector3f f = Math::normalize(p1 - p0);
	Vector3f r, u;
	Math::computeBasis(f, r, u);
	
	Vector3f offset = (r + u + f) * radius;

	baseAABB = AABB(Math::min(p0 - offset, p1 + offset),
			Math::max(p0 - offset, p1 + offset));
}

// TODO: generate basic AABB in constructor and transform it
AABB Physics::CapsuleCollider::computeAABB(const Transform& tf) const {
	return baseAABB.transform(tf.toMatrix());	
}


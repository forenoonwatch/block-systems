#include "collider.hpp"

#include "sphere-collider.hpp"
#include "plane-collider.hpp"
#include "capsule-collider.hpp"
#include "convex-collider.hpp"

void Physics::ColliderHints::initSphere(float radius) {
	type = ColliderType::TYPE_SPHERE;
	this->radius = radius;
}

void Physics::ColliderHints::initPlane() {
	type = ColliderType::TYPE_PLANE;
}

void Physics::ColliderHints::initCapsule(const Vector3f& p0,
		const Vector3f& p1, float radius) {
	type = ColliderType::TYPE_CAPSULE;

	this->radius = radius;
	this->p0 = p0;
	this->p1 = p1;
}

void Physics::ColliderHints::initConvexHull(const IndexedModel& model) {
	type = ColliderType::TYPE_CONVEX_HULL;

	this->model = &model;
}

Physics::Collider* Physics::Collider::make(const ColliderHints& hints) {
	switch (hints.type) {
		case ColliderType::TYPE_SPHERE:
			return new SphereCollider(hints);
		case ColliderType::TYPE_PLANE:
			return new PlaneCollider(hints);
		case ColliderType::TYPE_CAPSULE:
			return new CapsuleCollider(hints);
		case ColliderType::TYPE_CONVEX_HULL:
			return new ConvexCollider(hints);
		default:
			DEBUG_LOG(LOG_ERROR, "Collider Type", "Invalid collider type: %d",
					hints.type);
	}

	return nullptr;
}


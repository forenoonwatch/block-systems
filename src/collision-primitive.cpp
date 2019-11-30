#include "collision.hpp"

#include "body.hpp"
#include "manifold.hpp"

#include "sphere-collider.hpp"
#include "plane-collider.hpp"

#include <engine/math/math.hpp>

void Physics::collisionSphereSphere(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	Body* bodyA = a.body;
	Body* bodyB = b.body;
	
	SphereCollider* sphereA = (SphereCollider*)bodyA->getCollisionHull();
	SphereCollider* sphereB = (SphereCollider*)bodyB->getCollisionHull();

	Vector3f ds = bodyB->getTransform().getPosition()
			- bodyA->getTransform().getPosition();

	float d = Math::dot(ds, ds);
	float rSum = sphereA->radius + sphereB->radius;

	if (d < rSum * rSum) {
		FeaturePair fp;
		fp.key = 0;

		manifold.setNormal(Math::normalize(ds));
		manifold.addContact((bodyA->getTransform().getPosition()
				+ bodyB->getTransform().getPosition()) * 0.5f,
				rSum - Math::sqrt(d), fp);
	}
}

void Physics::collisionSpherePlane(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	Body* bodyA = a.body;
	Body* bodyB = b.body;

	SphereCollider* sphere = (SphereCollider*)&a;

	Matrix4f tf = bodyB->getTransform().toMatrix();
	Vector3f normal(tf[1]);

	float d = Math::dot(normal, bodyA->getTransform().getPosition()
			- bodyB->getTransform().getPosition());

	if (d < sphere->radius) {
		FeaturePair fp;
		fp.key = 0;

		manifold.setNormal(-normal);
		manifold.addContact(bodyA->getTransform().getPosition()
				- normal * d, sphere->radius - d, fp);
	}
}

void Physics::collisionPlaneSphere(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	collisionSpherePlane(manifold, b, a);
	manifold.setNormal(-manifold.getNormal());
	// TODO: flip feature pairs
}


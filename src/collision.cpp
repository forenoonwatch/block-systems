#include "collision.hpp"

#include "body.hpp"
#include "manifold.hpp"

#include "sphere-collider.hpp"
#include "convex-collider.hpp"

#include <engine/math/math.hpp>

void Physics::collisionSphereSphere(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	Body* bodyA = a.body;
	Body* bodyB = b.body;
	
	SphereCollider* sphereA = (SphereCollider*)bodyA->collisionHull;
	SphereCollider* sphereB = (SphereCollider*)bodyB->collisionHull;

	Vector3f ds = bodyB->transform.getPosition()
			- bodyA->transform.getPosition();

	float d = Math::dot(ds, ds);
	float rSum = sphereA->radius + sphereB->radius;

	if (d < rSum * rSum) {
		manifold.setNormal(Math::normalize(ds));

		manifold.addContact((bodyA->transform.getPosition()
				+ bodyB->transform.getPosition()) * 0.5f,
				Math::sqrt(d));
	}
}

void Physics::collisionConvexConvex(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {

}

void Physics::collisionSphereConvex(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	Body* bodyA = a.body;
	Body* bodyB = b.body;

	SphereCollider* sphere = (SphereCollider*)&a;

	Matrix4f tf = bodyB->transform.toMatrix();
	Vector3f normal(tf[1]);

	float d = Math::dot(normal, bodyA->transform.getPosition()
			- bodyB->transform.getPosition());

	if (d < sphere->radius) {
		if (bodyA == manifold.getBodyA()) {
			normal = -normal;
		}

		manifold.setNormal(normal);

		manifold.addContact(bodyA->transform.getPosition()
				- Vector3f(tf[1]) * ((d + d - sphere->radius) * 0.5f),
				sphere->radius - d);
	}
}

void Physics::collisionConvexSphere(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	collisionSphereConvex(manifold, b, a);
}


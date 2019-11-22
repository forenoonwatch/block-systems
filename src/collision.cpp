#include "collision.hpp"

#include "contact.hpp"
#include "body.hpp"

#include "sphere-collider.hpp"
#include "convex-collider.hpp"


void Physics::collisionSphereSphere(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {

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

	float d = Math::dot(Vector3f(tf[1]), bodyA->transform.getPosition()
			- bodyB->transform.getPosition());

	if (d < sphere->radius) {
		manifold.normal = Vector3f(tf[1]);

		manifold.contacts[0].position = bodyB->transform.getPosition()
				- Vector3f(tf[1]) * d;
		manifold.contacts[0].penetration = sphere->radius - d;

		manifold.numContacts = 1;

		if (manifold.a != &a) {
			manifold.normal = -manifold.normal;
		}
	}
}

void Physics::collisionConvexSphere(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	collisionSphereConvex(manifold, b, a);
}


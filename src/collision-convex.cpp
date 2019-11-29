#include "collision.hpp"

#include "body.hpp"
#include "manifold.hpp"

#include "sphere-collider.hpp"
#include "plane-collider.hpp"
#include "convex-collider.hpp"

#include <engine/math/math.hpp>

#include <cfloat>

void Physics::collisionConvexConvex(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	// TODO: implement
}

// TODO: make these actually sphere-convex collisions
void Physics::collisionSphereConvex(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	Body* bodyA = a.body;
	Body* bodyB = b.body;

	SphereCollider* sphere = (SphereCollider*)&a;

	Matrix4f tf = bodyB->getTransform().toMatrix();
	Vector3f normal(tf[1]);

	float d = Math::dot(normal, bodyA->getTransform().getPosition()
			- bodyB->getTransform().getPosition());

	if (d < sphere->radius) {
		manifold.setNormal(-normal);
		manifold.addContact(bodyA->getTransform().getPosition()
				- normal * ((d + d - sphere->radius) * 0.5f),
				sphere->radius - d);
	}
}

void Physics::collisionConvexSphere(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	collisionSphereConvex(manifold, b, a);
	manifold.setNormal(-manifold.getNormal());
}

void Physics::collisionConvexPlane(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	Body* bodyA = a.body;
	Body* bodyB = b.body;

	ConvexCollider* conv = (ConvexCollider*)&a;

	Matrix4f tf = bodyB->getTransform().toMatrix();
	Vector3f normal(tf[1]);

	manifold.setNormal(-normal);
}

void Physics::collisionPlaneConvex(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	collisionConvexPlane(manifold, b, a);
	manifold.setNormal(-manifold.getNormal());
}


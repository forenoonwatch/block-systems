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
	Body* bodyA = a.body;
	Body* bodyB = b.body;

	ConvexCollider* hullA = (ConvexCollider*)&a;
	ConvexCollider* hullB = (ConvexCollider*)&b;

	Matrix4f tfA = bodyA->getTransform().toMatrix();
	Matrix4f tfB = bodyB->getTransform().toMatrix();

	Matrix4f tfBtoA = Math::inverse(tfA) * tfB; // A^-1B -> B into A space
	Matrix4f tfAtoB = Math::inverse(tfB) * tfA; // B^-1A -> A into B space

	// TODO: verify correctness of converting things into the right space

	for (const Axis& a : hullA->getFaceAxes()) {
		// project all a.vertices onto a.axis
		// given: a.axis is in A-space
		// need: a.axis in B-space
		// B^-1A * a.axis
		// project all b.vertices onto B^-1A * a.axis
	}

	for (const Axis& a : hullB->getFaceAxes()) {
		// project all a.vertices onto A^-1B * a.axis
		// project all b.vertices onto a.axis
	}

	for (const Axis& ea : hullA->getEdgeAxes()) {
		for (const Axis& eb : hullB->getEdgeAxes()) {
			// axis = cross(ea.axis, A^-1B * eb.axis)
			// do cross product parallelism check

			// project all a.vertices onto axis
			// project all b.vertices onto A^-1B * axis
		}
	}
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
		FeaturePair fp;
		fp.key = 0;

		manifold.setNormal(-normal);
		manifold.addContact(bodyA->getTransform().getPosition()
				- normal * d, sphere->radius - d, fp);
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

	for (uint32 i = 0; i < conv->getVertices().size(); ++i) {
		Vector3f v = bodyA->getTransform().transform(conv->getVertices()[i],
				1.f);

		float d = Math::dot(normal, v - bodyB->getTransform().getPosition());

		if (d < 0.f) {
			FeaturePair fp;
			fp.key = i;

			manifold.addContact(v - normal * d, -d, fp);
		}
	}
}

void Physics::collisionPlaneConvex(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	collisionConvexPlane(manifold, b, a);
	manifold.setNormal(-manifold.getNormal());
	// TODO: flip feature pairs
}


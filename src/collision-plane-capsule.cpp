#include "collision.hpp"

#include "body.hpp"
#include "manifold.hpp"

#include "plane-collider.hpp"
#include "capsule-collider.hpp"

#include <engine/math/math.hpp>

void Physics::collisionPlaneCapsule(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	Body* bodyA = a.getBody();
	Body* bodyB = b.getBody();
	
	CapsuleCollider* capsule = (CapsuleCollider*)bodyB->getCollisionHull();

	Matrix4f tf = bodyA->getTransform().toMatrix();
	Vector3f normal(tf[1]);

	manifold.setNormal(normal);

	for (uint32 i = 0; i < 2; ++i) {
		Vector3f p = bodyB->getTransform().transform(capsule->getPoints()[i],
				1.f);

		float d = Math::dot(normal, p - bodyA->getTransform().getPosition());

		if (d < capsule->getRadius()) {
			FeaturePair fp;
			fp.key = i;

			manifold.addContact(p - normal * d, capsule->getRadius() - d, fp);
		}
	}
}

void Physics::collisionCapsulePlane(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	collisionPlaneCapsule(manifold, b, a);
	manifold.setNormal(-manifold.getNormal());
}


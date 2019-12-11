#include "collision.hpp"

#include "body.hpp"
#include "manifold.hpp"

#include "plane-collider.hpp"
#include "capsule-collider.hpp"

#include <engine/math/math.hpp>

void Physics::collisionPlaneCapsule(Manifold& manifold, Collider& a,
		Collider& b) {
	Body* bodyA = a.getBody();
	Body* bodyB = b.getBody();
	
	CapsuleCollider* capsule = (CapsuleCollider*)&b;

	Matrix4f tf = a.getWorldTransform().toMatrix();
	Vector3f normal(tf[1]);

	manifold.setNormal(normal);

	for (uint32 i = 0; i < 2; ++i) {
		Vector3f p = b.getWorldTransform().transform(capsule->getPoints()[i],
				1.f);

		float d = Math::dot(normal, p - a.getWorldTransform().getPosition());

		if (d < capsule->getRadius()) {
			FeaturePair fp;
			fp.key = i;

			manifold.addContact(p - normal * d, capsule->getRadius() - d, fp);
		}
	}
}

void Physics::collisionCapsulePlane(Manifold& manifold, Collider& a,
		Collider& b) {
	collisionPlaneCapsule(manifold, b, a);
	manifold.setNormal(-manifold.getNormal());
}


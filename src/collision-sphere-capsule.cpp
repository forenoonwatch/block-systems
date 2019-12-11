#include "collision.hpp"

#include "body.hpp"
#include "manifold.hpp"

#include "sphere-collider.hpp"
#include "capsule-collider.hpp"

#include <engine/math/math.hpp>

#include "distance-gjk.hpp"

#define EPSILON 1e-6f

void Physics::collisionSphereCapsule(Manifold& manifold, Collider& a,
		Collider& b) {
	Body* bodyA = a.getBody();
	Body* bodyB = b.getBody();
	
	SphereCollider* sphere = (SphereCollider*)&a;
	CapsuleCollider* capsule = (CapsuleCollider*)&b;

	Vector3f resA, resB;
	distanceGJK(Math::value_ptr(Vector3f()), 1, capsule->getPoints(), 2,
			a.getWorldTransform(), b.getWorldTransform(), resA, resB);

	Vector3f normal = resB - resA;
	float dist = Math::dot(normal, normal);
	
	float rSum = sphere->getRadius() + capsule->getRadius();

	if (dist > EPSILON && dist < rSum * rSum) {
		dist = Math::sqrt(dist);

		FeaturePair fp;
		fp.key = 0;

		manifold.setNormal(normal / dist);
		manifold.addContact((resA + resB) * 0.5f, rSum - dist, fp);
	}
}

void Physics::collisionCapsuleSphere(Manifold& manifold, Collider& a,
		Collider& b) {
	collisionSphereCapsule(manifold, b, a);
	manifold.setNormal(-manifold.getNormal());
}


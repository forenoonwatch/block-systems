#include "collision.hpp"

#include "body.hpp"
#include "manifold.hpp"

#include "capsule-collider.hpp"
#include "convex-collider.hpp"

#include <engine/math/math.hpp>

#include "distance-gjk.hpp"

#define EPSILON 1e-6f

void Physics::collisionCapsuleConvex(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	Body* bodyA = a.getBody();
	Body* bodyB = b.getBody();
	
	CapsuleCollider* capsule = (CapsuleCollider*)bodyA->getCollisionHull();
	ConvexCollider* convex = (ConvexCollider*)bodyB->getCollisionHull();

	Vector3f resA, resB;
	distanceGJK(capsule->getPoints(), 2, &convex->getVertices()[0],
			convex->getVertices().size(), bodyA->getTransform(),
			bodyB->getTransform(), resA, resB);

	Vector3f normal = resB - resA;
	float dist = Math::dot(normal, normal);

	if (dist > EPSILON) {
		if (dist < capsule->getRadius() * capsule->getRadius()) {
			dist = Math::sqrt(dist);
			manifold.setNormal(normal / dist);

			FeaturePair fp;
			fp.key = 0;
			manifold.addContact(resB, capsule->getRadius() - dist, fp);
		}
	}
}

void Physics::collisionConvexCapsule(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	collisionCapsuleConvex(manifold, b, a);
	manifold.setNormal(-manifold.getNormal());
}


#include "collision.hpp"

#include "body.hpp"
#include "manifold.hpp"

#include "sphere-collider.hpp"
#include "plane-collider.hpp"

#include <engine/math/math.hpp>

void Physics::collisionSphereSphere(Manifold& manifold, Collider& a,
		Collider& b) {
	Body* bodyA = a.getBody();
	Body* bodyB = b.getBody();
	
	SphereCollider* sphereA = (SphereCollider*)&a;
	SphereCollider* sphereB = (SphereCollider*)&b;

	Vector3f ds = b.getWorldTransform().getPosition()
			- a.getWorldTransform().getPosition();

	float d = Math::dot(ds, ds);
	float rSum = sphereA->getRadius() + sphereB->getRadius();

	if (d < rSum * rSum) {
		FeaturePair fp;
		fp.key = 0;

		manifold.setNormal(Math::normalize(ds));
		manifold.addContact((a.getWorldTransform().getPosition()
				+ b.getWorldTransform().getPosition()) * 0.5f,
				rSum - Math::sqrt(d), fp);
	}
}

void Physics::collisionSpherePlane(Manifold& manifold, Collider& a,
		Collider& b) {
	Body* bodyA = a.getBody();
	Body* bodyB = b.getBody();

	SphereCollider* sphere = (SphereCollider*)&a;

	Matrix4f tf = b.getWorldTransform().toMatrix();
	Vector3f normal(tf[1]);

	float d = Math::dot(normal, a.getWorldTransform().getPosition()
			- b.getWorldTransform().getPosition());

	if (d < sphere->getRadius()) {
		FeaturePair fp;
		fp.key = 0;

		manifold.setNormal(-normal);
		manifold.addContact(a.getWorldTransform().getPosition()
				- normal * d, sphere->getRadius() - d, fp);
	}
}

void Physics::collisionPlaneSphere(Manifold& manifold, Collider& a,
		Collider& b) {
	collisionSpherePlane(manifold, b, a);
	manifold.setNormal(-manifold.getNormal());
}


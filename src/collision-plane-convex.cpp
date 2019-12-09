#include "collision.hpp"

#include "body.hpp"
#include "manifold.hpp"

#include "plane-collider.hpp"
#include "convex-collider.hpp"

void Physics::collisionConvexPlane(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	Body* bodyA = a.getBody();
	Body* bodyB = b.getBody();

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
}


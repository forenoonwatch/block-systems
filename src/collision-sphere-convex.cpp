#include "collision.hpp"

#include "body.hpp"
#include "manifold.hpp"

#include "sphere-collider.hpp"
#include "convex-collider.hpp"

#include <engine/math/math.hpp>
	
#include "distance-gjk.hpp"

#include <cfloat>
#include <utility>

#define EPSILON 1e-6f

using namespace Physics;

static bool findDeepPenetration(const SphereCollider& sphere,
		const ConvexCollider& convex, const Vector3f& spherePos,
		float& minPenetration, uint32& minFaceID);

void Physics::collisionSphereConvex(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	Body* bodyA = a.body;
	Body* bodyB = b.body;

	SphereCollider* sphere = (SphereCollider*)&a;
	ConvexCollider* convex = (ConvexCollider*)&b;

	Vector3f spherePos = bodyA->getTransform().getPosition();
	Vector3f resA, resB;

	distanceGJK(&spherePos, 1, &convex->getVertices()[0],
			convex->getVertices().size(), Transform(),
			bodyB->getTransform(), resA, resB);

	Vector3f normal = resB - resA;
	float dist = Math::length(normal);

	if (dist > EPSILON) {
		if (dist < sphere->radius) {
			FeaturePair fp;
			fp.key = 0;

			manifold.setNormal(Math::normalize(normal));
			manifold.addContact(resB, sphere->radius - dist, fp);
		}
	}
	else {
		float minPenetration;
		uint32 minFaceID;

		Vector3f acInB = bodyB->getTransform().inverseTransform(
				bodyA->getTransform().getPosition(), 1.f);

		if (findDeepPenetration(*sphere, *convex, acInB, minPenetration,
				minFaceID)) {
			const Face& face = convex->getFaces()[minFaceID];

			normal = -bodyB->getTransform().transform(face.normal, 0.f);

			FeaturePair fp;
			fp.key = minFaceID;

			manifold.setNormal(normal);
			manifold.addContact(bodyA->getTransform().getPosition()
					+ normal * minPenetration,
					sphere->radius - minPenetration, fp);
		}
	}
}

void Physics::collisionConvexSphere(Manifold& manifold, CollisionHull& a,
		CollisionHull& b) {
	DEBUG_LOG_TEMP2("UNO REVERSE CARD");
	collisionSphereConvex(manifold, b, a);
	manifold.setNormal(-manifold.getNormal());
}

inline static bool findDeepPenetration(const SphereCollider& sphere,
		const ConvexCollider& convex, const Vector3f& spherePos,
		float& minPenetration, uint32& minFaceID) {
	minPenetration = -FLT_MAX;
	minFaceID = 0;

	for (uint32 i = 0; i < convex.getFaces().size(); ++i) {
		const Face& f = convex.getFaces()[i];
		float d = Math::dot(spherePos - f.centroid, f.normal);

		if (d > sphere.radius) {
			return false;
		}

		// minimum penetration is actually the greatest number
		if (d > minPenetration) {
			minPenetration = d;
			minFaceID = i;
		}
	}

	return true;
}


#include "collision.hpp"

#include "body.hpp"
#include "manifold.hpp"

#include "sphere-collider.hpp"
#include "plane-collider.hpp"
#include "convex-collider.hpp"

#include <engine/math/math.hpp>

#include <cfloat>

#define EPSILON 1e-6f

// TODO: move all this to math in the core engine
static constexpr bool isZero(float n) {
	return n > -EPSILON && n < EPSILON;
}

static constexpr bool isZeroVector(const Vector3f& v) {
	return isZero(v.x) && isZero(v.y) && isZero(v.z);
}

static bool checkAxisPenetration(const Physics::ConvexCollider& hullA,
		const Physics::ConvexCollider& hullB, const Vector3f& axisA,
		const Vector3f& axisB, uint32 axisID, float& minPenetration,
		uint32& minAxisID);

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

	float minPenetration = 0.f;
	uint32 minAxisID = 0;

	uint32 axisID = 0;

	for (const Axis& a : hullA->getFaceAxes()) {
		Vector3f axisA = a.axis;
		Vector3f axisB(tfAtoB * Vector4f(axisA, 0.f));

		if (!checkAxisPenetration(*hullA, *hullB, axisA, axisB, axisID,
				minPenetration, minAxisID)) {
			return;
		}

		++axisID;
	}

	for (const Axis& a : hullB->getFaceAxes()) {
		Vector3f axisB = a.axis;
		Vector3f axisA(tfBtoA * Vector4f(axisB, 0.f));

		if (!checkAxisPenetration(*hullA, *hullB, axisA, axisB, axisID,
				minPenetration, minAxisID)) {
			return;
		}

		++axisID;
	}

	for (const Axis& ea : hullA->getEdgeAxes()) {
		for (const Axis& eb : hullB->getEdgeAxes()) {
			Vector3f axisA = Math::cross(ea.axis,
					Vector3f(tfBtoA * Vector4f(eb.axis, 0.f)));

			// axis parallelism check
			if (isZeroVector(axisA)) {
				// ea.axis x eb.exis are very near parallel and in the plane P
				// try an axis perpendicular to ea.axis that lies in P
				Vector3f n = Math::cross(ea.axis, axisA);
				axisA = Math::cross(ea.axis, n);

				// legitimately still parallel, continue
				if (isZeroVector(axisA)) {
					continue;
				}
			}

			axisA = Math::normalize(axisA);
			Vector3f axisB(tfAtoB * Vector4f(axisA, 0.f));

			if (!checkAxisPenetration(*hullA, *hullB, axisA, axisB, axisID,
					minPenetration, minAxisID)) {
				return;
			}

			++axisID;
		}
	}

	// TODO: classify plane or edge collision
	
	// TODO: find reference and incident planes
	
	// TODO: generate manifold info
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

static bool checkAxisPenetration(const Physics::ConvexCollider& hullA,
		const Physics::ConvexCollider& hullB, const Vector3f& axisA,
		const Vector3f& axisB, uint32 axisID, float& minPenetration,
		uint32& minAxisID) {
	float sA = -FLT_MAX;
	float sB = FLT_MAX;

	for (const Vector3f& v : hullA.getVertices()) {
		float s = Math::dot(v, axisA);

		if (s > sA) {
			sA = s;
		}
	}
	
	for (const Vector3f& v : hullB.getVertices()) {
		float s = Math::dot(v, axisB);

		if (s < sB) {
			sB = s;
		}
	}

	if (sA > sB) {
		float pen = sA - sB;

		if (pen < minPenetration) {
			minPenetration = pen;
			minAxisID = axisID;
		}

		return true;
	}

	return false;
}


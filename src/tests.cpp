#include "tests.hpp"

#include "contact.hpp"
#include "convex-collider.hpp"

#include <engine/math/math.hpp>
#include <engine/math/transform.hpp>

#include <engine/core/asset-loader.hpp>

#include <cfloat>

#define EPSILON 1e-6f

// TODO: move all this to math in the core engine
static constexpr bool isZero(float n) {
	return n > -EPSILON && n < EPSILON;
}

static constexpr bool isZeroVector(const Vector3f& v) {
	return isZero(v.x) && isZero(v.y) && isZero(v.z);
}

static void generateManifold(Vector3f& normal,
		ArrayList<Physics::Contact>& contacts,
		const Physics::ConvexCollider* hullA,
		const Physics::ConvexCollider* hullB, const Transform& tfA,
		const Transform& tfB);

static bool checkAxisPenetration(const Physics::ConvexCollider& hullA,
		const Physics::ConvexCollider& hullB, float tDotL,
		const Vector3f& axisA, const Vector3f& axisB, uint32 axisID,
		float& minPenetration, uint32& minAxisID);

static void calcEdgeContact(const Physics::ConvexCollider& hullA,
		const Physics::ConvexCollider& hullB, const Transform& tfA,
		const Transform& tfB, const Vector3f& abA, const Vector3f& abB,
		uint32 minAxisID, Vector3f& normal, Vector3f& contactPoint);

static void closestSegmentToEdges(const Vector3f& e00, const Vector3f& e01,
		const Vector3f& e10, const Vector3f& e11, Vector3f& p0, Vector3f& p1,
		Vector3f& normal);

static const Physics::Face* findSupportFace(
		const Physics::ConvexCollider& hull, const Vector3f& ab,
		uint32 minAxisID);
static const Physics::Face* findIncidentFace(
		const Physics::ConvexCollider& hull, const Vector3f& normal);

static void runSATTest();

void runTests() {
	runSATTest();
}

static void runSATTest() {
	IndexedModel::AllocationHints hints;
	hints.elementSizes.push_back(3);
	hints.elementSizes.push_back(2);
	hints.elementSizes.push_back(3);
	hints.elementSizes.push_back(3);
	hints.elementSizes.push_back(16);
	hints.instancedElementStartIndex = 4;
	
	ArrayList<IndexedModel> models;
	if (!AssetLoader::loadAssets("./res/cube.obj", hints, models)) {
		DEBUG_LOG_TEMP2("Failed to load cube");
	}
	
	Physics::ConvexCollider hullA(models[0]);
	Physics::ConvexCollider hullB(models[0]);

	// edge contact
	/*Quaternion q = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(45.f), Vector3f(0.f, 0.f, 1.f)));

	Quaternion q2 = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(45.f), Vector3f(0.f, 1.f, 0.f)));

	Transform tfA(Vector3f(-0.5f, 0.f, 0.f), q, Vector3f(1.f));
	Transform tfB(Vector3f(0.5f, 0.f, 0.f), q2, Vector3f(1.f));*/

	// face contact
	Quaternion q = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(45.f), Vector3f(1.f, 0.f, 0.f)));

	Quaternion q2 = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(0.f), Vector3f(0.f, 1.f, 0.f)));

	Transform tfA(Vector3f(-0.45f, 0.f, 0.f), q, Vector3f(1.f));
	Transform tfB(Vector3f(0.45f, 0.f, 0.f), q2, Vector3f(1.f));

	Vector3f normal;
	ArrayList<Physics::Contact> contacts;

	generateManifold(normal, contacts, &hullA, &hullB, tfA, tfB);

	if (contacts.size() > 0) {
		DEBUG_LOG_TEMP2("HULLS INTERSECT:");

		DEBUG_LOG_TEMP("N: (%.2f, %.2f, %.2f)", normal.x, normal.y, normal.z);

		for (auto& contact : contacts) {
			DEBUG_LOG_TEMP("C: P = %.2f, Key = %d", contact.penetration,
					contact.fp.key);
		}
	}
	else {
		DEBUG_LOG_TEMP2("Hulls don't intersect");
	}
}

static void generateManifold(Vector3f& normal,
		ArrayList<Physics::Contact>& contacts,
		const Physics::ConvexCollider* hullA,
		const Physics::ConvexCollider* hullB, const Transform& tfA,
		const Transform& tfB) {
	using namespace Physics;

	Matrix4f mA = tfA.toMatrix();
	Matrix4f mB = tfB.toMatrix();

	Matrix4f mAI = Math::inverse(mA);
	Matrix4f mBI = Math::inverse(mB);

	Matrix4f tfBtoA = mAI * mB; // A^-1B -> B into A space
	Matrix4f tfAtoB = mBI * mA; // B^-1A -> A into B space

	Vector3f abA(mAI * Vector4f(tfB.getPosition() - tfA.getPosition(), 0.f));
	Vector3f abB(mBI * Vector4f(tfB.getPosition() - tfA.getPosition(), 0.f));

	float minPenetration = FLT_MAX;
	uint32 minAxisID = 0;

	uint32 axisID = 0;

	float abDotL;

	for (const Axis& a : hullA->getFaceAxes()) {
		Vector3f axisA = a.axis;

		abDotL = Math::dot(axisA, abA);

		if (abDotL < 0.f) {
			axisA = -axisA;
		}

		Vector3f axisB(tfAtoB * Vector4f(axisA, 0.f));

		if (!checkAxisPenetration(*hullA, *hullB, abDotL, axisA, axisB, axisID,
				minPenetration, minAxisID)) {
			return;
		}

		++axisID;
	}

	for (const Axis& a : hullB->getFaceAxes()) {
		Vector3f axisB = a.axis;

		abDotL = Math::dot(axisB, abB);

		if (abDotL < 0.f) {
			axisB = -axisB;
		}
		
		Vector3f axisA(tfBtoA * Vector4f(axisB, 0.f));

		if (!checkAxisPenetration(*hullA, *hullB, abDotL, axisA, axisB, axisID,
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
					++axisID;
					continue;
				}
			}
			
			abDotL = Math::dot(axisA, abA);

			if (abDotL < 0.f) {
				axisA = -axisA;
			}

			axisA = Math::normalize(axisA);
			Vector3f axisB(tfAtoB * Vector4f(axisA, 0.f));

			if (!checkAxisPenetration(*hullA, *hullB, abDotL, axisA, axisB,
					axisID, minPenetration, minAxisID)) {
				return;
			}

			++axisID;
		}
	}

	if (minAxisID < hullA->getFaceAxes().size()
			+ hullB->getFaceAxes().size()) {
		const Face* referenceFace;
		const Face* incidentFace;

		if (minAxisID < hullA->getFaceAxes().size()) {
			DEBUG_LOG_TEMP2("REFERENCE FACE IS ON HULL A");
			referenceFace = findSupportFace(*hullA, abA, minAxisID);
			Vector3f rN(tfAtoB * Vector4f(referenceFace->normal, 0.f));

			incidentFace = findIncidentFace(*hullB, rN);
		}
		else {
			//minAxisID -= hullA->getFaceAxes().size();
			DEBUG_LOG_TEMP2("REFERENCE FACE IS ON HULL B");
			referenceFace = findSupportFace(*hullB, -abB,
					minAxisID - hullA->getFaceAxes().size());
			Vector3f rN(tfBtoA * Vector4f(referenceFace->normal, 0.f));

			incidentFace = findIncidentFace(*hullA, rN);
			// TODO: see if anything needs to be flagged as flipped
		}

		// TODO:
		// - project reference edge-normals into incident-space
		// - clip incident hull vertices against reference edge normals
		// - planar project clipped points onto reference face
		// - move points into world space
		// - generate contact points
		// - TODO: figure out how to match pairs

		DEBUG_LOG_TEMP("RC: (%.2f, %.2f, %.2f)", referenceFace->centroid.x,
				referenceFace->centroid.y, referenceFace->centroid.z);
		DEBUG_LOG_TEMP("RN: (%.2f, %.2f, %.2f)", referenceFace->normal.x,
				referenceFace->normal.y, referenceFace->normal.z);

		DEBUG_LOG_TEMP("IC: (%.2f, %.2f, %.2f)", incidentFace->centroid.x,
				incidentFace->centroid.y, incidentFace->centroid.z);
		DEBUG_LOG_TEMP("IN: (%.2f, %.2f, %.2f)", incidentFace->normal.x,
				incidentFace->normal.y, incidentFace->normal.z);
		
		// TODO: clip incident face by orthogonal edges of reference face

		// TODO: planar project clipped points onto reference face
		// bring into world space and set as contact points
	}
	else {
		Vector3f contactPoint;

		calcEdgeContact(*hullA, *hullB, tfA, tfB, abA, abB, minAxisID,
				normal, contactPoint);
		
		Contact c;
		c.rA = contactPoint - tfA.getPosition();
		c.rB = contactPoint - tfB.getPosition();
		c.penetration = minPenetration;
		c.fp.key = minAxisID;

		contacts.push_back(c);
	}
}

static bool checkAxisPenetration(const Physics::ConvexCollider& hullA,
		const Physics::ConvexCollider& hullB, float tDotL,
		const Vector3f& axisA, const Vector3f& axisB, uint32 axisID,
		float& minPenetration, uint32& minAxisID) {
	float sA = -FLT_MAX;
	float sB = FLT_MAX;
	float s;

	for (const Vector3f& v : hullA.getVertices()) {
		s = Math::dot(v, axisA);

		if (s > sA) {
			sA = s;
		}
	}
	
	for (const Vector3f& v : hullB.getVertices()) {
		s = Math::dot(v, axisB);

		if (s < sB) {
			sB = s;
		}
	}
	
	s = Math::abs(tDotL) - (sA - sB);

	if (s < 0.f) {
		if (-s < minPenetration) {
			minPenetration = -s;
			minAxisID = axisID;
		}

		return true;
	}

	return false;
}

inline static void calcEdgeContact(const Physics::ConvexCollider& hullA,
		const Physics::ConvexCollider& hullB, const Transform& tfA,
		const Transform& tfB, const Vector3f& abA, const Vector3f& abB,
		uint32 minAxisID, Vector3f& normal, Vector3f& contactPoint) {
	using namespace Physics;

	uint32 aID = minAxisID - (hullA.getFaceAxes().size()
			+ hullB.getFaceAxes().size());

	uint32 edgeIDA = aID / hullA.getEdgeAxes().size();
	uint32 edgeIDB = aID % hullB.getEdgeAxes().size();

	float aMax = -FLT_MAX;
	float bMin = FLT_MAX;

	const Edge* e0;
	const Edge* e1;

	for (uint32 i : hullA.getEdgeAxes()[edgeIDA].indices) {
		const Edge* e = &hullA.getEdges()[i];
		Vector3f c = (e->v0 + e->v1) * 0.5f;
		float d = Math::dot(c, abA);

		if (d > aMax) {
			aMax = d;
			e0 = e;
		}
	}

	for (uint32 i : hullB.getEdgeAxes()[edgeIDB].indices) {
		const Edge* e = &hullB.getEdges()[i];
		Vector3f c = (e->v0 + e->v1) * 0.5f;
		float d = Math::dot(c, abB);

		if (d < bMin) {
			bMin = d;
			e1 = e;
		}
	}

	Vector3f p1;
	closestSegmentToEdges(tfA.transform(e0->v0, 1.f),
			tfA.transform(e0->v1, 1.f), tfB.transform(e1->v0, 1.f),
			tfB.transform(e1->v1, 1.f), contactPoint, p1, normal);

	contactPoint = (contactPoint + p1) * 0.5f;
}

inline static void closestSegmentToEdges(const Vector3f& e00,
		const Vector3f& e01, const Vector3f& e10, const Vector3f& e11,
		Vector3f& p0, Vector3f& p1, Vector3f& normal) {
	const Vector3f d1 = e01 - e00;
	const Vector3f d2 = e11 - e10;
	const Vector3f r = e00 - e10;

	const Vector3f n = Math::cross(d1, d2);

	const float b = Math::dot(d1, d2);
	const float c = Math::dot(d1, r);
	const float f = Math::dot(d2, r);

	const float d = 1.f / Math::dot(n, n);

	// result
	p0 = e00 + d1 * ((b * f - c * Math::dot(d2, d2)) * d);
	p1 = e10 + d2 * ((Math::dot(d1, d1) * f - b * c) * d);
	normal = Math::normalize(n);
}

inline static const Physics::Face* findSupportFace(
		const Physics::ConvexCollider& hull, const Vector3f& ab,
		uint32 minAxisID) {
	float sMax = -FLT_MAX;
	const Physics::Face* fOut;

	for (uint32 i : hull.getFaceAxes()[minAxisID].indices) {
		const Physics::Face* f = &hull.getFaces()[i];
		float s = Math::dot(f->centroid, ab);

		if (s > sMax) {
			sMax = s;
			fOut = f;
		}
	}

	return fOut;
}

inline static const Physics::Face* findIncidentFace(
		const Physics::ConvexCollider& hull, const Vector3f& normal) {
	float sMin = FLT_MAX;
	const Physics::Face* fOut;

	for (const Physics::Face& f : hull.getFaces()) {
		float s = Math::dot(f.normal, normal);

		if (s < sMin) {
			sMin = s;
			fOut = &f;
		}
	}

	return fOut;
}


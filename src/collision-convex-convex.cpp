#include "collision.hpp"

#include "body.hpp"
#include "manifold.hpp"

#include "convex-collider.hpp"

#include <engine/math/math.hpp>

#include <cfloat>

#define EPSILON 1e-6f

#define PEN_BIAS_REL	-0.95f // default: 0.95f
#define PEN_BIAS_ABS	-0.01f

// TODO: move all this to math in the core engine
static constexpr bool isZero(float n) {
	return n > -EPSILON && n < EPSILON;
}

static constexpr bool isZeroVector(const Vector3f& v) {
	return isZero(v.x) && isZero(v.y) && isZero(v.z);
}

namespace {
	struct VertexData {
		inline VertexData() {
			fp.key = ~0;
		}

		inline VertexData(const Physics::FaceVertex& fv)
				: v(fv.v) {
			fp.key = ~0;
			fp.inI = fp.outI = fv.edgeID;
		}

		Vector3f v;
		Physics::FeaturePair fp;
		float depth;
	};
};

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

static void calcContactPoints(const Physics::Face& referenceFace,
		const Physics::Face& incidentFace, const Matrix4f& refToInc,
		const Matrix4f& incToWorld, ArrayList<VertexData>& points);

static void clip(const ArrayList<VertexData>& inVertices,
		ArrayList<VertexData>& outVertices, 
		const ArrayList<Physics::EdgePlane>& planes);

void Physics::collisionConvexConvex(Manifold& manifold, Collider& a,
		Collider& b) {
	Body* bodyA = a.getBody();
	Body* bodyB = b.getBody();

	ConvexCollider* hullA = (ConvexCollider*)&a;
	ConvexCollider* hullB = (ConvexCollider*)&b;
	
	Matrix4f mA = a.getWorldTransform().toMatrix();
	Matrix4f mB = b.getWorldTransform().toMatrix();

	Matrix4f mAI = Math::inverse(mA);
	Matrix4f mBI = Math::inverse(mB);

	Matrix4f tfBtoA = mAI * mB; // A^-1B -> B into A space
	Matrix4f tfAtoB = mBI * mA; // B^-1A -> A into B space

	Vector3f abA(mAI * Vector4f(b.getWorldTransform().getPosition()
				- a.getWorldTransform().getPosition(), 0.f));
	Vector3f abB(mBI * Vector4f(b.getWorldTransform().getPosition()
				- a.getWorldTransform().getPosition(), 0.f));

	float minFacePen = FLT_MAX;
	float minEdgePen = FLT_MAX;

	uint32 minFaceAxisID = 0;
	uint32 minEdgeAxisID = 0;

	uint32 axisID = 0;

	float abDotL;

	for (const Axis& a : hullA->getFaceAxes()) {
		Vector3f axisA = a.axis;

		abDotL = Math::dot(axisA, abA);

		if (abDotL < 0.f) {
			axisA = -axisA;
			abDotL = -abDotL;
		}

		Vector3f axisB(tfAtoB * Vector4f(axisA, 0.f));

		if (!checkAxisPenetration(*hullA, *hullB, abDotL, axisA, axisB, axisID,
				minFacePen, minFaceAxisID)) {
			return;
		}

		++axisID;
	}

	for (const Axis& a : hullB->getFaceAxes()) {
		Vector3f axisB = a.axis;

		abDotL = Math::dot(axisB, abB);

		if (abDotL < 0.f) {
			axisB = -axisB;
			abDotL = -abDotL;
		}
		
		Vector3f axisA(tfBtoA * Vector4f(axisB, 0.f));

		if (!checkAxisPenetration(*hullA, *hullB, abDotL, axisA, axisB, axisID,
				minFacePen, minFaceAxisID)) {
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
			
			axisA = Math::normalize(axisA);
			abDotL = Math::dot(axisA, abA);

			if (abDotL < 0.f) {
				axisA = -axisA;
				abDotL = -abDotL;
			}

			Vector3f axisB(tfAtoB * Vector4f(axisA, 0.f));

			if (!checkAxisPenetration(*hullA, *hullB, abDotL, axisA, axisB,
					axisID, minEdgePen, minEdgeAxisID)) {
				return;
			}

			++axisID;
		}
	}

	float minPenetration = minFacePen;
	uint32 minAxisID = minFaceAxisID;

	if (PEN_BIAS_REL * minEdgePen > minFacePen + PEN_BIAS_ABS) {
		minPenetration = minEdgePen;
		minAxisID = minEdgeAxisID;
	}

	if (minAxisID < hullA->getFaceAxes().size()
			+ hullB->getFaceAxes().size()) {
		const Face* referenceFace;
		const Face* incidentFace;

		ArrayList<VertexData> contactPoints;
		bool flip = false;

		if (minAxisID < hullA->getFaceAxes().size()) {
			referenceFace = findSupportFace(*hullA, abA, minAxisID);
			Vector3f rN(tfAtoB * Vector4f(referenceFace->normal, 0.f));

			incidentFace = findIncidentFace(*hullB, rN);

			manifold.setNormal(Vector3f(mA
					* Vector4f(referenceFace->normal, 0.f)));
			calcContactPoints(*referenceFace, *incidentFace, tfAtoB, mB,
					contactPoints);
		}
		else {
			referenceFace = findSupportFace(*hullB, -abB,
					minAxisID - hullA->getFaceAxes().size());
			Vector3f rN(tfBtoA * Vector4f(referenceFace->normal, 0.f));

			incidentFace = findIncidentFace(*hullA, rN);

			// TODO: double check if negating is necessary
			manifold.setNormal(Vector3f(mB
					* Vector4f(-referenceFace->normal, 0.f)));
			calcContactPoints(*referenceFace, *incidentFace, tfBtoA, mA,
					contactPoints);
			flip = true;
		}
		
		for (VertexData& cp : contactPoints) {
			if (flip) {
				std::swap(cp.fp.inI, cp.fp.inR);
				std::swap(cp.fp.outI, cp.fp.outR);
			}

			manifold.addContact(cp.v, cp.depth, cp.fp);
		}
	}
	else {
		Vector3f contactPoint, normal;

		calcEdgeContact(*hullA, *hullB, a.getWorldTransform(),
				b.getWorldTransform(), abA, abB, minAxisID, normal,
				contactPoint);
		
		manifold.setNormal(normal);

		FeaturePair fp;
		fp.key = minAxisID;
		manifold.addContact(contactPoint, minPenetration, fp);
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

	s = tDotL - (sA - sB);

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

	uint32 edgeIDA = aID / hullB.getEdgeAxes().size();
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

inline static void clip(const ArrayList<Physics::FaceVertex>& inVertices,
		ArrayList<VertexData>& outVertices, 
		const ArrayList<Physics::EdgePlane>& planes) {
	ArrayList<VertexData> temp;
	temp.insert(temp.begin(), inVertices.begin(), inVertices.end());

	clip(temp, outVertices, planes);
}

inline static void calcContactPoints(const Physics::Face& referenceFace,
		const Physics::Face& incidentFace, const Matrix4f& refToInc,
		const Matrix4f& incToWorld, ArrayList<VertexData>& points) {
	ArrayList<Physics::EdgePlane> incPlanes;

	for (const auto& ep : referenceFace.edgePlanes) {
		Physics::EdgePlane ip;
		ip.position = Vector3f(refToInc * Vector4f(ep.position, 1.f));
		ip.normal = Vector3f(refToInc * Vector4f(ep.normal, 0.f));

		incPlanes.push_back(ip);
	}

	clip(incidentFace.vertices, points, incPlanes);

	Vector3f refCentroid(refToInc * Vector4f(referenceFace.centroid, 1.f));
	Vector3f refNormal(refToInc * Vector4f(referenceFace.normal, 0.f));

	for (uint32 i = 0; i < points.size(); ++i) {
		float d = Math::dot(refNormal, points[i].v - refCentroid);

		// discard point if it is above plane
		if (d > 0.f) {
			points[i] = points.back();
			points.pop_back();
			--i;

			continue;
		}
		
		points[i].depth = -d;

		// planar-project point onto reference face
		points[i].v -= refNormal * d;

		// bring into world space
		points[i].v = Vector3f(incToWorld * Vector4f(points[i].v, 1.f));
	}
}

// TODO: get rid of these
#define BELOW(a)	((a) < 0.f)
#define ABOVE(a)	((a) >= 0.f)
#define ON(a)		((a) > -0.005f && (a) < 0.005f)

inline static void planeClip(const ArrayList<VertexData>& inVertices,
		ArrayList<VertexData>& outVertices, const Physics::EdgePlane& plane) {
	if (inVertices.empty()) {
		return;
	}

	// TODO: figure out why there would be less than 2 vertices

	VertexData a = inVertices.back();

	for (const VertexData& b : inVertices) {
		float da = Math::dot(plane.normal, a.v - plane.position);
		float db = Math::dot(plane.normal, b.v - plane.position);

		if ((BELOW(da) && BELOW(db)) || ON(db)) {
			outVertices.push_back(b);
		}
		else if (BELOW(da) && ABOVE(db)) {
			VertexData cv;
			cv.v = a.v + (b.v - a.v) * (da / (da - db));
			cv.fp = b.fp;
			cv.fp.outR = plane.edgeID;
			cv.fp.outI = 0;

			outVertices.push_back(cv);
		}
		else if (ABOVE(da) && BELOW(db)) {
			VertexData cv;
			cv.v = a.v + (b.v - a.v) * (da / (da - db));
			cv.fp = a.fp;
			cv.fp.inR = plane.edgeID;
			cv.fp.inI = 0;

			outVertices.push_back(cv);
			outVertices.push_back(b);
		}

		a = b;
	}
}

inline static void clip(const ArrayList<VertexData>& inVertices,
		ArrayList<VertexData>& outVertices, 
		const ArrayList<Physics::EdgePlane>& planes) {
	planeClip(inVertices, outVertices, planes.front());

	// TODO: potentially don't even need this check because we know
	// that all convex polygons have >1 edges
	if (planes.size() > 1) {
		ArrayList<VertexData> buffer2;

		ArrayList<VertexData>* readBuffer = &outVertices;
		ArrayList<VertexData>* writeBuffer = &buffer2;

		for (uint32 i = 1; i < planes.size(); ++i) {
			writeBuffer->clear();
			planeClip(*readBuffer, *writeBuffer, planes[i]);

			if (readBuffer == &outVertices) {
				readBuffer = &buffer2;
				writeBuffer = &outVertices;
			}
			else {
				readBuffer = &outVertices;
				writeBuffer = &buffer2;
			}
		}

		if (writeBuffer == &outVertices) {
			outVertices.clear();
			outVertices.insert(outVertices.begin(), buffer2.begin(),
					buffer2.end());
		}
	}
}


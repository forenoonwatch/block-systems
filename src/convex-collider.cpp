#include "convex-collider.hpp"

#include <engine/rendering/indexed-model.hpp>

#define EPSILON 1e-6f

// TODO: get rid of all these ugly macros
#define IS_ZERO(n) ((n) > -EPSILON && (n) < EPSILON)

#define DOT_PARALLEL(d) (d < -(1.f - EPSILON) || d > (1.f - EPSILON))
#define DOT_ORTHOGONAL(d) IS_ZERO(d)

namespace {
	struct FaceData {
		Vector3f centroid;
		Vector3f normal;
		ArrayList<Physics::Edge> edges;
	};

	// TODO: move this to math class
	constexpr bool v3Equal(const Vector3f& a, const Vector3f& b) {
		const Vector3f& vd = b - a;

		return IS_ZERO(vd.x) && IS_ZERO(vd.y) && IS_ZERO(vd.z);
	}
};

using namespace Physics;

static void mergeCoplanarFaces(ArrayList<FaceData>& faces);

static void initializeFaces(const ArrayList<FaceData>& faceData,
		ArrayList<Face>& faces);
static void initializeEdges(const ArrayList<FaceData>& faceData,
		ArrayList<Edge>& edges);

static void mergeFaces(FaceData& f0, FaceData& f1);

static void generateFaceVertices(const ArrayList<Edge>& edgeData,
		ArrayList<FaceVertex>& vertices, uint32& edgeID);

static bool isCoplanar(const FaceData& f0, const FaceData& f1);

static bool isCollinear(const Edge& e0, const Edge& e1);

Physics::ConvexCollider::ConvexCollider(const IndexedModel& model)
		: CollisionHull(CollisionHull::TYPE_CONVEX_HULL) {
	const uint32* indices = model.getIndices();

	for (uint32 i = 0; i < model.getElementArraySize(0); i += 3) {
		vertices.push_back(model.getElement3f(0, i));
	}

	ArrayList<FaceData> faceData;

	for (uint32 i = 0; i < model.getNumIndices(); i += 3) {
		uint32 i0 = 3 * indices[i];
		uint32 i1 = 3 * indices[i + 1];
		uint32 i2 = 3 * indices[i + 2];

		const Vector3f& v0 = model.getElement3f(0, i0);
		const Vector3f& v1 = model.getElement3f(0, i1);
		const Vector3f& v2 = model.getElement3f(0, i2);

		Vector3f centroid = (v0 + v1 + v2) / 3.f;
		Vector3f normal = (model.getElement3f(2, i0)
				+ model.getElement3f(2, i1) + model.getElement3f(2, i2)) / 3.f;
		
		FaceData fd;
		fd.centroid = centroid;
		fd.normal = normal;

		Edge e;

		e.v0 = v0;
		e.v1 = v1;
		fd.edges.push_back(e);

		e.v0 = v0;
		e.v1 = v2;
		fd.edges.push_back(e);

		e.v0 = v1;
		e.v1 = v2;
		fd.edges.push_back(e);

		faceData.push_back(fd);
	}

	mergeCoplanarFaces(faceData);

	initializeFaces(faceData, faces);
	initializeEdges(faceData, edges);

	initializeFaceAxes();
	initializeEdgeAxes();

	removeDuplicateVertices();
}

AABB Physics::ConvexCollider::computeAABB(const Transform& tf) const {
	return AABB(&vertices[0], vertices.size()).transform(tf.toMatrix());
}

inline void Physics::ConvexCollider::initializeFaceAxes() {
	for (uint32 i = 0; i < faces.size(); ++i) {
		Face& face = faces[i];

		int32 axisIndex = -1;

		for (int32 j = 0; j < faceAxes.size(); ++j) {
			float d = Math::dot(faceAxes[j].axis, face.normal);

			if (DOT_PARALLEL(d)) {
				axisIndex = j;
				break;
			}
		}

		if (axisIndex != -1) {
			faceAxes[axisIndex].indices.push_back(i);
		}
		else {
			Axis a;
			a.axis = face.normal;
			a.indices.push_back(i);

			faceAxes.push_back(a);
		}
	}

	faceAxes.shrink_to_fit();
}

inline void Physics::ConvexCollider::initializeEdgeAxes() {
	for (uint32 i = 0; i < edges.size(); ++i) {
		Edge& edge = edges[i];
		Vector3f en = Math::normalize(edge.v1 - edge.v0);

		int32 axisIndex = -1;

		for (int32 j = 0; j < edgeAxes.size(); ++j) {
			float d = Math::dot(edgeAxes[j].axis, en);

			if (DOT_PARALLEL(d)) {
				axisIndex = j;
				break;
			}
		}

		if (axisIndex != -1) {
			edgeAxes[axisIndex].indices.push_back(i);
		}
		else {
			Axis a;
			a.axis = en;
			a.indices.push_back(i);

			edgeAxes.push_back(a);
		}
	}

	edgeAxes.shrink_to_fit();
}

inline void Physics::ConvexCollider::removeDuplicateVertices() {
	ArrayList<Vector3f> uniqueVertices;

	for (uint32 i = 0; i < vertices.size(); ++i) {
		bool unique = true;

		for (const Vector3f& v : uniqueVertices) {
			Vector3f vd = v - vertices[i];

			if (vd.x > -EPSILON && vd.x < EPSILON && vd.y > -EPSILON
					&& vd.y < EPSILON && vd.z > -EPSILON && vd.z < EPSILON) {
				unique = false;
				break;
			}
		}

		if (unique) {
			uniqueVertices.push_back(vertices[i]);
		}
	}

	vertices.clear();
	vertices.insert(vertices.begin(), uniqueVertices.begin(),
			uniqueVertices.end());
	vertices.shrink_to_fit();
}

inline static void mergeCoplanarFaces(ArrayList<FaceData>& faces) {
	for (uint32 i = 0; i < faces.size(); ++i) {
		FaceData& f0 = faces[i];

		for (uint32 j = i + 1; j < faces.size(); ++j) {
			FaceData& f1 = faces[j];

			if (isCoplanar(f0, f1)) {
				mergeFaces(f0, f1);

				faces[j] = faces.back();
				faces.pop_back();
				--j;
			}
		}
	}
}

inline static void initializeFaces(const ArrayList<FaceData>& faceData,
		ArrayList<Face>& faces) {
	EdgePlane ep;

	uint32 faceEdgeID = 0;
	uint32 vertexEdgeID = 0;
	
	for (const FaceData& fd : faceData) {
		Face f;
		f.centroid = fd.centroid;
		f.normal = fd.normal;

		// flip inward-facing normal
		if (Math::dot(f.centroid, f.normal) < 0.f) {
			f.normal = -f.normal;
		}

		for (const Edge& e : fd.edges) {
			ep.position = e.v0;
			ep.normal = Math::normalize(Math::cross(f.normal, e.v1 - e.v0));
			ep.edgeID = faceEdgeID++;

			// flip inward-facing normal
			if (Math::dot(ep.normal, ep.position - f.centroid) < 0.f) {
				ep.normal = -ep.normal;
			}

			f.edgePlanes.push_back(ep);
		}

		generateFaceVertices(fd.edges, f.vertices, vertexEdgeID);

		faces.push_back(f);
	}

	faces.shrink_to_fit();
}

inline static void initializeEdges(const ArrayList<FaceData>& faceData,
		ArrayList<Edge>& edges) {
	for (const FaceData& fd : faceData) {
		for (const Edge& ed : fd.edges) {
			Edge e;
			e.v0 = ed.v0;
			e.v1 = ed.v1;

			edges.push_back(e);
		}
	}

	for (uint32 i = 0; i < edges.size(); ++i) {
		Edge& e0 = edges[i];

		for (uint32 j = i + 1; j < edges.size(); ++j) {
			Edge& e1 = edges[j];

			if (isCollinear(e0, e1)) {
				edges[j] = edges.back();
				edges.pop_back();

				--j;
			}
		}
	}

	edges.shrink_to_fit();
}

inline static void generateFaceVertices(const ArrayList<Edge>& edgeData,
		ArrayList<FaceVertex>& vertices, uint32& edgeID) {
	const Vector3f stopPoint = edgeData.front().v0;
	Vector3f searchPoint = edgeData.front().v1;

	uint32 baseEdgeID = edgeID;
	vertices.emplace_back(edgeData.front().v1, edgeID);
	++edgeID;

	ArrayList<bool> visited(edgeData.size(), false);

	do {
		for (uint32 i = 1; i < edgeData.size(); ++i) {
			if (visited[i]) {
				continue;
			}

			if (v3Equal(edgeData[i].v0, searchPoint)) {
				vertices.emplace_back(edgeData[i].v1, edgeID);
				searchPoint = edgeData[i].v1;
				visited[i] = true;

				++edgeID;

				break;
			}
			else if (v3Equal(edgeData[i].v1, searchPoint)) {
				vertices.emplace_back(edgeData[i].v0, edgeID);
				searchPoint = edgeData[i].v0;
				visited[i] = true;

				++edgeID;

				break;
			}
		}
	}
	while (!v3Equal(searchPoint, stopPoint));

	//vertices.back().edgeID = baseEdgeID;
	//--edgeID;
}

inline static void mergeFaces(FaceData& f0, FaceData& f1) {
	for (uint32 i = 0; i < f0.edges.size(); ++i) {
		Edge& e0 = f0.edges[i];

		for (uint32 j = 0; j < f1.edges.size(); ++j) {
			Edge& e1 = f1.edges[j];

			if (isCollinear(e0, e1)) {
				f0.edges[i] = f0.edges.back();
				f0.edges.pop_back();

				f1.edges[j] = f1.edges.back();
				f1.edges.pop_back();

				--i;
				--j;
			}
		}
	}

	f0.centroid = (f0.centroid + f1.centroid) * 0.5f;

	f0.edges.insert(f0.edges.end(), f1.edges.begin(),
			f1.edges.end());
}

inline static bool isCoplanar(const FaceData& f0, const FaceData& f1) {
	float d = Math::dot(f0.normal, f1.normal);

	if (DOT_PARALLEL(d)) {
		d = Math::dot(f0.normal, Math::normalize(f1.centroid - f0.centroid));
		return DOT_ORTHOGONAL(d);
	}

	return false;
}

inline static bool isCollinear(const Edge& e0, const Edge& e1) {
	const Vector3f v0 = Math::normalize(e0.v1 - e0.v0);
	const Vector3f v1 = Math::normalize(e1.v1 - e1.v0);

	float d = Math::dot(v0, v1);

	if (DOT_PARALLEL(d)) {
		Vector3f n;

		if (v3Equal(e0.v0, e1.v0)) {
			if (v3Equal(e0.v1, e1.v1)) {
				return true;
			}

			n = e1.v1 - e0.v1;
		}
		else if (v3Equal(e0.v1, e1.v0)) {
			if (v3Equal(e0.v0, e1.v1)) {
				return true;
			}

			n = e1.v1 - e0.v0;
		}
		else {
			n = e1.v1 - e0.v0;
		}
		
		d = Math::dot(v0, Math::normalize(n));
		return DOT_PARALLEL(d);
	}

	return false;
}


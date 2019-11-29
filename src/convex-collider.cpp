#include "convex-collider.hpp"

#include <engine/rendering/indexed-model.hpp>

#define EPSILON 1e-6f

namespace {
	struct VertexIndex {
		inline VertexIndex(const Vector3f& vertex, uint32 index)
				: vertex(vertex)
				, index(index) {}

		Vector3f vertex;
		uint32 index;
	};

	struct Edge {
		inline Edge(uint32 i0 = 0, uint32 i1 = 0)
				: i0(i0)
				, i1(i1) {}

		uint32 i0;
		uint32 i1;
	};


	struct Face {
		Vector3f centroid;
		Vector3f normal;
		Edge edges[3];
		uint32 numEdges;
	};
};

static void removeRedundantEdges(const ArrayList<Vector3f>& vertices,
		ArrayList<Face>& faces);
static void removeDuplicateEdges(const ArrayList<Vector3f>& vertices,
		ArrayList<Edge>& edges);

static bool isCoplanar(const Face& f0, const Face& f1);
static bool isCollinear(const Vector3f& e00, const Vector3f& e01,
		const Vector3f& e10, const Vector3f& e11);

Physics::ConvexCollider::ConvexCollider(const IndexedModel& model)
		: CollisionHull(CollisionHull::TYPE_CONVEX_HULL) {
	const uint32* indices = model.getIndices();

	ArrayList<Face> faces;

	for (uint32 i = 0; i < model.getElementArraySize(0); i += 3) {
		vertices.push_back(model.getElement3f(0, i));
	}

	for (uint32 i = 0; i < model.getNumIndices(); i += 3) {
		uint32 i0 = 3 * indices[i];
		uint32 i1 = 3 * indices[i + 1];
		uint32 i2 = 3 * indices[i + 2];

		Vector3f centroid = (model.getElement3f(0, i0)
				+ model.getElement3f(0, i1) + model.getElement3f(0, i2)) / 3.f;

		Vector3f normal = (model.getElement3f(2, i0)
				+ model.getElement3f(2, i1) + model.getElement3f(2, i2)) / 3.f;

		normals.push_back(normal);
		
		Face f;
		f.centroid = centroid;
		f.normal = normal;

		f.edges[0] = Edge(indices[i], indices[i + 1]);
		f.edges[1] = Edge(indices[i], indices[i + 2]);
		f.edges[2] = Edge(indices[i + 1], indices[i + 2]);
		f.numEdges = 3;

		faces.push_back(f);
	}

	removeRedundantEdges(vertices, faces);

	ArrayList<Edge> candidateEdges;

	for (const Face& face : faces) {
		for (uint32 i = 0; i < face.numEdges; ++i) {
			candidateEdges.push_back(face.edges[i]);
		}
	}

	removeDuplicateEdges(vertices, candidateEdges);

	for (const Edge& e : candidateEdges) {
		edges.push_back(Math::normalize(vertices[e.i1] - vertices[e.i0]));
	}

	removeDuplicateNormals();
	removeDuplicateVertices();
}

AABB Physics::ConvexCollider::computeAABB(const Transform& tf) const {
	return AABB(&vertices[0], vertices.size()).transform(tf.toMatrix());
}

inline void Physics::ConvexCollider::removeDuplicateNormals() {
	for (uint32 i = 0; i < normals.size(); ++i) {
		Vector3f n0 = normals[i];

		for (uint32 j = i + 1; j < normals.size(); ++j) {
			float d = Math::dot(n0, normals[j]);

			if (d < -(1.f - EPSILON) || d > (1.f - EPSILON)) {
				normals[j] = normals.back();
				normals.pop_back();
				--j;
			}
		}
	}
}

inline void Physics::ConvexCollider::removeDuplicateVertices() {
	ArrayList<VertexIndex> uniqueVertices;

	for (uint32 i = 0; i < vertices.size(); ++i) {
		bool unique = true;

		for (const VertexIndex& vi : uniqueVertices) {
			Vector3f vd = vi.vertex - vertices[i];

			if (vd.x > -EPSILON && vd.x < EPSILON && vd.y > -EPSILON
					&& vd.y < EPSILON && vd.z > -EPSILON && vd.z < EPSILON) {
				unique = false;
				break;
			}
		}

		if (unique) {
			uniqueVertices.emplace_back(vertices[i], i);
		}
	}

	vertices.clear();

	for (VertexIndex& vi : uniqueVertices) {
		vertices.push_back(vi.vertex);
	}
}

inline static void removeRedundantEdges(const ArrayList<Vector3f>& vertices,
		ArrayList<Face>& faces) {
	for (uint32 i = 0; i < faces.size(); ++i) {
		Face& f0 = faces[i];

		for (uint32 j = i + 1; j < faces.size(); ++j) {
			Face& f1 = faces[j];

			if (!isCoplanar(f0, f1)) {
				continue;
			}

			for (uint32 k = 0; k < f0.numEdges; ++k) {
				Edge& e0 = f0.edges[k];

				for (uint32 l = 0; l < f1.numEdges; ++l) {
					Edge& e1 = f1.edges[l];
					
					if (isCollinear(vertices[e0.i0], vertices[e0.i1],
							vertices[e1.i0], vertices[e1.i1])) {
						--f0.numEdges;
						--f1.numEdges;

						f0.edges[k] = f0.edges[f0.numEdges];
						f1.edges[l] = f1.edges[f1.numEdges];

						--k;
						--l;
					}
				}
			}
		}
	}
}

inline static void removeDuplicateEdges(const ArrayList<Vector3f>& vertices,
		ArrayList<Edge>& edges) {
	for (uint32 i = 0; i < edges.size(); ++i) {
		Edge& e0 = edges[i];
		Vector3f v0 = Math::normalize(vertices[e0.i1] - vertices[e0.i0]);

		for (uint32 j = i + 1; j < edges.size(); ++j) {
			Edge& e1 = edges[j];

			if ((e0.i0 == e1.i0 && e0.i1 == e1.i1)
					|| (e0.i0 == e1.i1 && e0.i1 == e1.i0)) {
				edges[j] = edges.back();
				edges.pop_back();
				--j;

				continue;
			}

			Vector3f v1 = Math::normalize(vertices[e1.i1] - vertices[e1.i0]);
			float d = Math::dot(v0, v1);

			if (d < -(1.f - EPSILON) || d > (1.f - EPSILON)) {
				edges[j] = edges.back();
				edges.pop_back();
				--j;
			}
		}
	}
}

inline static bool isCoplanar(const Face& f0, const Face& f1) {
	float d = Math::dot(f0.normal, f1.normal);

	if (d < -(1.f - EPSILON) || d > (1.f - EPSILON)) {
		d = Math::dot(f0.normal, Math::normalize(f1.centroid - f0.centroid));

		return d > -EPSILON && d < EPSILON;
	}

	return false;
}

inline static bool isCollinear(const Vector3f& e00, const Vector3f& e01,
		const Vector3f& e10, const Vector3f& e11) {
	Vector3f n0 = Math::normalize(e01 - e00);
	Vector3f n1 = Math::normalize(e11 - e10);

	float d = Math::dot(n0, n1);

	if (d < -(1.f - EPSILON) || d > (1.f - EPSILON)) {
		d = Math::dot(n0, Math::normalize(e11 - e00));
		return d < -(1.f - EPSILON) || d > (1.f - EPSILON);
	}

	return false;
}


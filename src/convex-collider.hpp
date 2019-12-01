#pragma once

#include "collision-hull.hpp"

#include <engine/core/array-list.hpp>
#include <engine/math/vector.hpp>

class IndexedModel;

namespace Physics {
	struct Edge { // TODO: potentially excise the primitives to another file
		Vector3f v0;
		Vector3f v1;
	};

	struct EdgePlane { // TODO: potentially replace with an abcd plane
		Vector3f position;
		Vector3f normal;
	};

	struct Face {
		Vector3f centroid;
		Vector3f normal;
		ArrayList<EdgePlane> edgePlanes;
	};

	struct Axis {
		Vector3f axis;
		ArrayList<uint32> indices;
	};

	class ConvexCollider : public CollisionHull {
		public:
			ConvexCollider(const IndexedModel& model);

			virtual AABB computeAABB(const Transform& tf) const override;

			inline const ArrayList<Vector3f>& getVertices() const;
			inline const ArrayList<Face>& getFaces() const;
			inline const ArrayList<Edge>& getEdges() const;

			inline const ArrayList<Axis>& getFaceAxes() const;
			inline const ArrayList<Axis>& getEdgeAxes() const;
		private:
			ArrayList<Vector3f> vertices;
			ArrayList<Edge> edges;
			ArrayList<Face> faces;

			ArrayList<Axis> faceAxes;
			ArrayList<Axis> edgeAxes;

			void initializeFaceAxes();
			void initializeEdgeAxes();

			void removeDuplicateVertices();
	};
};

inline const ArrayList<Vector3f>& Physics::ConvexCollider::getVertices()
		const {
	return vertices;
}

inline const ArrayList<Physics::Face>& Physics::ConvexCollider::getFaces()
		const {
	return faces;
}

inline const ArrayList<Physics::Edge>& Physics::ConvexCollider::getEdges()
		const {
	return edges;
}

inline const ArrayList<Physics::Axis>&
		Physics::ConvexCollider::getFaceAxes() const {
	return faceAxes;
}

inline const ArrayList<Physics::Axis>&
		Physics::ConvexCollider::getEdgeAxes() const {
	return edgeAxes;
}


#pragma once

#include "collision-hull.hpp"

#include <engine/core/array-list.hpp>
#include <engine/math/vector.hpp>

class IndexedModel;

namespace Physics {
	class ConvexCollider : public CollisionHull {
		public:
			ConvexCollider(const IndexedModel& model);

			virtual AABB computeAABB(const Transform& tf) const override;

			inline const ArrayList<Vector3f>& getVertices() const;
			inline const ArrayList<Vector3f>& getNormals() const;
			inline const ArrayList<Vector3f>& getEdges() const;
		private:
			ArrayList<Vector3f> vertices;
			ArrayList<Vector3f> normals;
			ArrayList<Vector3f> edges;

			void removeDuplicateNormals();
			void removeDuplicateVertices();
	};
};

inline const ArrayList<Vector3f>& Physics::ConvexCollider::getVertices()
		const {
	return vertices;
}

inline const ArrayList<Vector3f>& Physics::ConvexCollider::getNormals() const {
	return normals;
}

inline const ArrayList<Vector3f>&
		Physics::ConvexCollider::getEdges() const {
	return edges;
}


#pragma once

#include "collider.hpp"

namespace Physics {
	class PlaneCollider : public Collider {
		public:
			inline PlaneCollider()
					: Collider(Collider::TYPE_PLANE) {}

			virtual AABB computeAABB(const Transform& tf) const override;
		private:
	};
};


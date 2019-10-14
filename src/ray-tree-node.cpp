#include "ray-tree-node.hpp"

#include "block.hpp"

#include <cfloat>

RayTreeNode::RayTreeNode(const Vector3f& minExtents,
			const Vector3f& maxExtents, uint32 level, RayTreeNode* parent)
		: aabb(minExtents, maxExtents)
		, level(level)
		, parent(parent)
		, children{0}
		, limitReached(false) {
	uint32 i = 0;

	const Vector3f center = aabb.getCenter();
	const Vector3f extents = aabb.getExtents();

	for (float z = -1.f; z <= 1.f; z += 2.f) {
		for (float y = -1.f; y <= 1.f; y += 2.f) {
			for (float x = -1.f; x <= 1.f; x += 2.f) {
				const Vector3f corner = center + extents * Vector3f(x, y, z);

				childAABBs[i++] = AABB(Math::min(center, corner),
						Math::max(center, corner));
			}
		}
	}
}

bool RayTreeNode::intersectsRay(const Vector3f& origin,
		const Vector3f& direction, Vector3i* intersectCoord,
		Vector3f* intersectPos) const {
	float p1, p2;

	if (aabb.intersectRay(origin, direction, p1, p2)) {
		if (!limitReached) {
			float minDist = FLT_MAX;
			const Vector3i* minCoord = nullptr;

			for (auto& c : coords) {
				const Vector3f b0 = Vector3f(c) - Vector3f(0.5f, 0.5f, 0.5f);
				const Vector3f b1 = Vector3f(c) + Vector3f(0.5f, 0.5f, 0.5f);
				const AABB box(Math::min(b0, b1), Math::max(b0, b1));

				if (box.intersectRay(origin, direction, p1, p2)) {
					if (p1 < minDist) {
						minDist = p1;
						minCoord = &c;
					}
				}
			}

			if (minCoord != nullptr) {
				*intersectCoord = *minCoord;
				*intersectPos = origin + direction * minDist;

				return true;
			}
		}
		else {
			Vector3i tempCoord;
			Vector3f tempPos;

			float minDist = FLT_MAX;
			RayTreeNode* minChild = nullptr;

			for (RayTreeNode* child : children) {
				if (child != nullptr && child->intersectsRay(origin, direction,
						&tempCoord, &tempPos)) {
					const float len2 = Math::dot(tempPos - origin,
							tempPos - origin);

					if (len2 < minDist) {
						minDist = len2;
						minChild = child;

						*intersectCoord = tempCoord;
						*intersectPos = tempPos;
					}
				}
			}

			if (minChild != nullptr) {
				return true;
			}
		}
	}

	return false;
}

bool RayTreeNode::addObject(const Vector3i& coord) {
	if (!limitReached) {
		coords.push_back(coord);

		if (coords.size() >= MAX_OBJECTS && level < MAX_DEPTH) {
			limitReached = true;

			for (auto& c : coords) {
				const Vector3f p(c);

				for (uint32 i = 0; i < 8; ++i) {
					if (childAABBs[i].contains(p)) {
						if (children[i] == nullptr) {
							children[i] = new RayTreeNode(
									childAABBs[i].getMinExtents(),
									childAABBs[i].getMaxExtents(),
									level + 1, this);
						}

						children[i]->addObject(c);
						break;
					}
				}
			}

			coords.clear();
		}
		
		return true;
	}
	else {
		const Vector3f p(coord);

		for (uint32 i = 0; i < 8; ++i) {
			if (childAABBs[i].contains(p)) {
				if (children[i] == nullptr) {
					children[i] = new RayTreeNode(
							childAABBs[i].getMinExtents(),
							childAABBs[i].getMaxExtents(), level + 1, this);
				}

				return children[i]->addObject(coord);
			}
		}
	}

	return false;
}

bool RayTreeNode::removeObject(const Vector3i& coord) {
	if (limitReached) {
		const Vector3f p(coord);

		for (uint32 i = 0; i < 8; ++i) {
			if (children[i] != nullptr && childAABBs[i].contains(p)) {
				return children[i]->removeObject(coord);
			}
		}
	}
	else {
		for (uint32 i = 0; i < coords.size(); ++i) {
			if (coords[i] == coord) {
				coords[i] = coords.back();
				coords.pop_back();

				return true;
			}
		}
	}

	return false;
}

RayTreeNode::~RayTreeNode() {
	for (uint32 i = 0; i < 8; ++i) {
		if (children[i] != nullptr) {
			delete children[i];
		}
	}
}


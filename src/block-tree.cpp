#include "block-tree.hpp"

#include "block.hpp"

#include <engine/math/math.hpp>

#include <engine/rendering/indexed-model.hpp>

#include <cfloat>

BlockTreeNode::BlockTreeNode(const Vector3f& minExtents,
			const Vector3f& maxExtents, uint32 maxDepth, uint32 maxObjects,
			uint32 level, BlockTreeNode* parent)
		: aabb(minExtents, maxExtents)
		, level(level)
		, maxDepth(maxDepth)
		, maxObjects(maxObjects)
		, parent(parent)
		, children{0}
		, limitReached(false)
		, totalVolume(0.f)
		, totalMass(0.f)
		, centerSum(0.f, 0.f, 0.f) {
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

bool BlockTreeNode::intersectsRay(const Vector3f& origin,
		const Vector3f& direction, Vector3i* intersectCoord,
		Vector3f* intersectPos) const {
	float p1, p2;

	if (aabb.intersectsRay(origin, direction, p1, p2)) {
		if (!limitReached) {
			float minDist = FLT_MAX;
			const Vector3i* minCoord = nullptr;

			for (auto* blk : blocks) {
				const AABB box(Vector3f(blk->position)
						- Vector3f(0.5f, 0.5f, 0.5f),
						Vector3f(blk->position) + Vector3f(0.5f, 0.5f, 0.5f));

				if (box.intersectsRay(origin, direction, p1, p2)) {
					if (p1 < minDist) {
						minDist = p1;
						minCoord = &blk->position;
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
			BlockTreeNode* minChild = nullptr;

			for (BlockTreeNode* child : children) {
				if (child != nullptr && child->intersectsRay(origin, direction,
						&tempCoord, &tempPos)) {
					const float len = Math::length(tempPos - origin);

					if (len < minDist) {
						minDist = len;
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

bool BlockTreeNode::belowPlane(const Vector3f& position,
		const Vector3f& normal, Vector3f& centerSumBelow, float& volumeBelow,
		float& massBelow) const {
	float r, s;

	if (aabb.belowPlane(position, normal, r, s)) {
		if (s > -r) {
			return childrenBelowPlane(position, normal, centerSumBelow,
					volumeBelow, massBelow);
		}
		else {
			volumeBelow += totalVolume;
			massBelow += totalMass;
			centerSumBelow += centerSum;

			return true;
		}
	}

	return false;
}

bool BlockTreeNode::add(const Block& block) {
	if (!aabb.contains(Vector3f(block.position))) {
		return false;
	}

	totalVolume += BlockInfo::getInfo(block.type).volume;
	totalMass += BlockInfo::getInfo(block.type).mass;
	centerSum += Vector3f(block.position);

	if (!limitReached) {
		blocks.push_back(&block);

		if (blocks.size() >= maxObjects && level < maxDepth) {
			limitReached = true;

			for (auto* blk : blocks) {
				for (uint32 i = 0; i < 8; ++i) {
					if (childAABBs[i].contains(Vector3f(blk->position))) {
						if (children[i] == nullptr) {
							children[i] = new BlockTreeNode(
									childAABBs[i].getMinExtents(),
									childAABBs[i].getMaxExtents(),
									maxDepth, maxObjects, level + 1, this);
						}

						children[i]->add(*blk);
					}
				}
			}

			blocks.clear();
		}
		
		return true;
	}
	else {
		for (uint32 i = 0; i < 8; ++i) {
			if (childAABBs[i].contains(Vector3f(block.position))) {
				if (children[i] == nullptr) {
					children[i] = new BlockTreeNode(
							childAABBs[i].getMinExtents(),
							childAABBs[i].getMaxExtents(), maxDepth,
							maxObjects, level + 1, this);
				}

				return children[i]->add(block);
			}
		}
	}

	return false;
}

bool BlockTreeNode::remove(const Block& block) {
	bool removed = false;

	if (limitReached) {
		for (uint32 i = 0; i < 8; ++i) {
			if (children[i] != nullptr
					&& childAABBs[i].contains(Vector3f(block.position))) {
				removed = children[i]->remove(block);
				break;
			}
		}
	}
	else {
		for (uint32 i = 0; i < blocks.size(); ++i) {
			if (blocks[i]->position == block.position) {
				blocks[i] = blocks.back();
				blocks.pop_back();

				removed = true;
				break;
			}
		}
	}

	if (removed) {
		totalVolume -= BlockInfo::getInfo(block.type).volume;
		totalMass -= BlockInfo::getInfo(block.type).mass;
		centerSum -= Vector3f(block.position);
	}

	return removed;
}

BlockTreeNode::~BlockTreeNode() {
	for (uint32 i = 0; i < 8; ++i) {
		if (children[i] != nullptr) {
			delete children[i];
		}
	}
}

inline bool BlockTreeNode::childrenBelowPlane(const Vector3f& position,
		const Vector3f& normal, Vector3f& centerSumBelow, float& volumeBelow,
		float& massBelow) const {
	bool below = false;

	if (limitReached) {
		for (uint32 i = 0; i < 8; ++i) {
			if (children[i] != nullptr && children[i]->belowPlane(position,
					normal, centerSumBelow, volumeBelow, massBelow)) {
				below = true;
			}
		}

		return below;
	}

	float r, s;

	for (auto* blk : blocks) {
		const Vector3f p(blk->position);
		AABB box(p - Vector3f(0.5f, 0.5f, 0.5f),
				p + Vector3f(0.5f, 0.5f, 0.5f));

		if (box.belowPlane(position, normal, r, s)) {
			//const float submerged = (r - s) / (r + r);

			if (s > -r) {
				const IndexedModel& model =
						*BlockInfo::getInfo(blk->type).model;

				Vector3f cSum;
				float submergedV = model.calcSubmergedVolume(position - p,
						normal, cSum);
				float partialMass = BlockInfo::getInfo(blk->type).mass
						* submergedV / BlockInfo::getInfo(blk->type).volume;

				cSum += p * submergedV;
				
				volumeBelow += submergedV;
				massBelow += partialMass;
				centerSumBelow += cSum;
			}
			else {
				volumeBelow += BlockInfo::getInfo(blk->type).volume;
				massBelow += BlockInfo::getInfo(blk->type).mass;
				centerSumBelow += box.getCenter();
			}
			
			below = true;
		}
	}

	return below;
}

BlockTree::BlockTree(uint32 maxDepth, uint32 bplIn)
		: blocksPerLength((uint32)Math::pow(2, maxDepth) * bplIn)
		, maxDepth(maxDepth)
		, maxObjects(bplIn * bplIn * bplIn)
		, root(-Vector3f(blocksPerLength, blocksPerLength, blocksPerLength)
				- Vector3f(0.5f, 0.5f, 0.5f),
				Vector3f(blocksPerLength, blocksPerLength, blocksPerLength)
				- Vector3f(0.5f, 0.5f, 0.5f), maxDepth, maxObjects) {
}

bool BlockTree::calcSubmergedVolume(const Vector3f& planePosition,
		const Vector3f& planeNormal, Vector3f& centroid,
		float& submergedVolume, float& submergedMass) const {
	submergedVolume = 0.f;
	submergedMass = 0.f;
	centroid = Vector3f(0.f, 0.f, 0.f);

	if (!root.belowPlane(planePosition, planeNormal, centroid,
			submergedVolume, submergedMass) || submergedVolume == 0.f) {
		return false;
	}

	centroid /= submergedVolume;

	return true;
}


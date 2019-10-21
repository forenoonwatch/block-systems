#include "block-tree.hpp"

#include "block.hpp"

#include <cfloat>

BlockTreeNode::BlockTreeNode(const Vector3f& minExtents,
			const Vector3f& maxExtents, uint32 level, BlockTreeNode* parent)
		: aabb(minExtents, maxExtents)
		, level(level)
		, parent(parent)
		, children{0}
		, limitReached(false)
		, totalVolume(0.f)
		, totalBlocks(0.f)
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
		const Vector3f& normal, float& volumeBelow,
		Vector3f& centerSumBelow, float& numBlocksBelow) const {
	float r, s;

	if (aabb.belowPlane(position, normal, r, s)) {
		if (r - s < r + r) {
			return childrenBelowPlane(position, normal, volumeBelow,
					centerSumBelow, numBlocksBelow);
		}
		else {
			volumeBelow += totalVolume;
			centerSumBelow += centerSum;
			numBlocksBelow += totalBlocks;

			return true;
		}
	}

	return false;
}

bool BlockTreeNode::add(const Block& block) {
	const AABB blkBox(Vector3f(block.position)
			- Vector3f(0.5f, 0.5f, 0.5f),
			Vector3f(block.position) + Vector3f(0.5f, 0.5f, 0.5f));
	const AABB olap = aabb.overlap(blkBox);

	const float containedVolume = olap.getVolume()
			* BlockInfo::getInfo(block.type).volume;

	totalVolume += containedVolume;
	centerSum += olap.getCenter();
	totalBlocks += olap.getVolume();

	if (!limitReached) {
		blocks.push_back(&block); // TODO: account for mass change

		if (blocks.size() >= MAX_OBJECTS && level < MAX_DEPTH) {
			limitReached = true;

			for (auto* blk : blocks) {
				const Vector3f p(blk->position);
				const AABB box(p - Vector3f(0.5f, 0.5f, 0.5f),
						p + Vector3f(0.5f, 0.5f, 0.5f));

				for (uint32 i = 0; i < 8; ++i) {
					if (childAABBs[i].intersects(box)) {
						if (children[i] == nullptr) {
							children[i] = new BlockTreeNode(
									childAABBs[i].getMinExtents(),
									childAABBs[i].getMaxExtents(),
									level + 1, this);
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
		const Vector3f p(block.position);
		const AABB box(p - Vector3f(0.5f, 0.5f, 0.5f),
						p + Vector3f(0.5f, 0.5f, 0.5f));

		bool added = false;

		for (uint32 i = 0; i < 8; ++i) {
			if (childAABBs[i].intersects(box)) {
				if (children[i] == nullptr) {
					children[i] = new BlockTreeNode(
							childAABBs[i].getMinExtents(),
							childAABBs[i].getMaxExtents(), level + 1, this);
				}

				added |= children[i]->add(block);
			}
		}

		return added;
	}

	return false;
}

bool BlockTreeNode::remove(const Block& block) {
	bool removed = false;

	if (limitReached) {
		const Vector3f p(block.position);
		const AABB box(p - Vector3f(0.5f, 0.5f, 0.5f),
				p + Vector3f(0.5f, 0.5f, 0.5f));

		for (uint32 i = 0; i < 8; ++i) {
			if (children[i] != nullptr && childAABBs[i].intersects(box)) {
				removed |= children[i]->remove(block);
			}
		}
	}
	else {
		for (uint32 i = 0; i < blocks.size(); ++i) {
			if (blocks[i]->position == block.position) { // TODO: account for mass change
				blocks[i] = blocks.back();
				blocks.pop_back();

				removed = true;
				break;
			}
		}
	}

	if (removed) {
		const AABB blkBox(Vector3f(block.position)
				- Vector3f(0.5f, 0.5f, 0.5f),
				Vector3f(block.position) + Vector3f(0.5f, 0.5f, 0.5f));
		const AABB olap = aabb.overlap(blkBox);

		const float containedVolume = olap.getVolume()
				* BlockInfo::getInfo(block.type).volume;

		totalVolume -= containedVolume;
		centerSum -= olap.getCenter();
		totalBlocks -= olap.getVolume();
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
		const Vector3f& normal, float& volumeBelow,
		Vector3f& centerSumBelow, float& numBlocksBelow) const {
	bool below = false;

	if (limitReached) {
		for (uint32 i = 0; i < 8; ++i) {
			if (children[i] != nullptr) {
				below |= children[i]->belowPlane(position, normal,
						volumeBelow, centerSumBelow, numBlocksBelow);
			}
		}

		return below;
	}

	float r, s;

	for (auto* blk : blocks) {
		const Vector3f p(blk->position);
		AABB box(p - Vector3f(0.5f, 0.5f, 0.5f),
				p + Vector3f(0.5f, 0.5f, 0.5f));

		box = aabb.overlap(box);

		if (box.belowPlane(position, normal, r, s)) {
			const float submerged = (r - s) / (r + r);

			if (submerged < 1.f) {
				volumeBelow += submerged * BlockInfo::getInfo(blk->type).volume
						* box.getVolume();
				centerSumBelow += box.getCenter() + normal * ((-r * submerged)
						- s);
			}
			else {
				volumeBelow += BlockInfo::getInfo(blk->type).volume
						* box.getVolume();
				centerSumBelow += box.getCenter();
			}
				
			numBlocksBelow += box.getVolume();
			
			below = true;
		}
	}

	return below;
}


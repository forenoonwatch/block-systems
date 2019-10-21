#pragma once

#include "block-tree-node.hpp"

class BlockTree {
	public:
		BlockTree(uint32 maxDepth, uint32 blocksPerLength);

		inline bool intersectsRay(const Vector3f& origin,
				const Vector3f& direction, Vector3i* intersectCoord,
				Vector3f* intersectPos) const;

		bool calcSubmergedVolume(const Vector3f& planePosition,
				const Vector3f& planeNormal, Vector3f& centroid,
				float& submergedVolume, float& submergedMass) const;

		inline bool add(const Block& block) { return root.add(block); }
		inline bool remove(const Block& block) { return root.remove(block); }

		inline BlockTreeNode** getChildren() { return root.getChildren(); }

		inline BlockTreeNode& getRoot() { return root; }
	private:
		uint32 blocksPerLength;
		uint32 maxDepth;
		uint32 maxObjects;

		BlockTreeNode root;
};

inline bool BlockTree::intersectsRay(const Vector3f& origin,
		const Vector3f& direction, Vector3i* intersectCoord,
		Vector3f* intersectPos) const {
	return root.intersectsRay(origin, direction, intersectCoord, intersectPos);
}


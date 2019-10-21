#include "block-tree.hpp"

#include <engine/math/math.hpp>

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


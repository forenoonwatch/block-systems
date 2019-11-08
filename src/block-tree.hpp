#pragma once

#include "block-tree-node.hpp"
#include "physics.hpp"

class Transform;

class BlockTree {
	public:
		BlockTree(uint32 maxDepth, uint32 blocksPerLength);

		inline bool intersectsRay(const Vector3f& origin,
				const Vector3f& direction, Vector3i* intersectCoord,
				Vector3f* intersectPos) const;

		bool calcSubmergedVolume(const Vector3f& planePosition,
				const Vector3f& planeNormal, Vector3f& centroid,
				float& submergedVolume, float& submergedMass) const;

		void applyBuoyantForce(Physics::Body& body, Transform& transform,
				const Vector3f& planePosition,
				const Vector3f& planeNormal) const;

		inline bool add(const Block& block) { return root.add(block); }
		inline bool remove(const Block& block) { return root.remove(block); }

		inline BlockTreeNode** getChildren() { return root.getChildren(); }

		inline BlockTreeNode& getRoot() { return root; }
	private:
		uint32 blocksPerLength;
		uint32 maxDepth;
		uint32 maxObjects;

		BlockTreeNode root;

		static void applyBuoyancyToNode(const BlockTreeNode* node,
				Physics::Body& body, Transform& transform,
				const Vector3f& pLocal, const Vector3f& nLocal,
				Vector3f& centroidSum, float& volumeSum, float& massSum,
				const Vector3f& localCenter,
				const Vector3f& localVelocity, const Vector3f& localAngularVelocity,
				Vector3f& netForce, Vector3f& netTorque);
		static void applyBuoyancyToChildren(const BlockTreeNode* node,
				Physics::Body& body, Transform& transform,
				const Vector3f& pLocal, const Vector3f& nLocal,
				Vector3f& centroidSum, float& volumeSum, float& massSum,
				const Vector3f& localCenter,
				const Vector3f& localVelocity, const Vector3f& localAngularVelocity,
				Vector3f& netForce, Vector3f& netTorque);
};

inline bool BlockTree::intersectsRay(const Vector3f& origin,
		const Vector3f& direction, Vector3i* intersectCoord,
		Vector3f* intersectPos) const {
	return root.intersectsRay(origin, direction, intersectCoord, intersectPos);
}


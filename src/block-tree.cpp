#include "block-tree.hpp"

#include "block.hpp"
#include "ocean.hpp"

#include <engine/math/math.hpp>
#include <engine/math/transform.hpp>

#include <engine/rendering/indexed-model.hpp>

static void applyBuoyancy(const Vector3f& localCenter,
		const Vector3f& localVelocity,
		const Vector3f& localAngularVelocity,
		const Vector3f& localGravity,
		Vector3f& netForce, Vector3f& netTorque,
		const Vector3f& centerOfBuoyancy, float submergedVolume,
		float partialMass);

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

void BlockTree::applyBuoyantForce(Physics::Body& body, Transform& transform,
		const Vector3f& planePosition, const Vector3f& planeNormal) const {
	const Vector3f pLocal = transform.inverseTransform(planePosition, 1.f);
	const Vector3f nLocal = transform.inverseTransform(planeNormal, 0.f);
	
	const Vector3f vLocal = transform.inverseTransform(body.velocity, 0.f);
	const Vector3f avLocal = transform.inverseTransform(body.angularVelocity, 0.f);

	Vector3f centroidSum(0.f, 0.f, 0.f);
	float volumeSum = 0.f;
	float massSum = 0.f;

	Vector3f netForce(0.f, 0.f, 0.f);
	Vector3f netTorque(0.f, 0.f, 0.f);

	applyBuoyancyToNode(&root, body, transform, pLocal, nLocal,
			centroidSum, volumeSum, massSum,
			body.localCenter, vLocal, avLocal,
			netForce, netTorque);

	if (volumeSum > 0.f) {
		centroidSum /= volumeSum;

		applyBuoyancy(body.localCenter, vLocal, avLocal,
				nLocal * Physics::GRAVITY.y,
				netForce, netTorque,
				centroidSum, volumeSum, massSum);
	}

	netForce = transform.transform(netForce, 0.f);
	netTorque = transform.transform(netTorque, 0.f);

	body.force += netForce;
	body.torque += netTorque;
}

inline void BlockTree::applyBuoyancyToNode(const BlockTreeNode* node,
		Physics::Body& body, Transform& transform, const Vector3f& pLocal,
		const Vector3f& nLocal,
		Vector3f& centroidSum, float& volumeSum, float& massSum,
		const Vector3f& localCenter,
		const Vector3f& localVelocity, const Vector3f& localAngularVelocity,
		Vector3f& netForce, Vector3f& netTorque) {
	float r, s;

	if (node->aabb.belowPlane(pLocal, nLocal, r, s)) {
		if (s > -r) {
			applyBuoyancyToChildren(node, body, transform, pLocal, nLocal,
					centroidSum, volumeSum, massSum,
					localCenter,
					localVelocity, localAngularVelocity,
					netForce, netTorque);
		}
		else if (node->totalVolume > 0.f) {
			centroidSum += node->centerSum;
			volumeSum += node->totalVolume;
			massSum += node->totalMass;
		}
	}
}

inline void BlockTree::applyBuoyancyToChildren(const BlockTreeNode* node,
		Physics::Body& body, Transform& transform, const Vector3f& pLocal,
		const Vector3f& nLocal,
		Vector3f& centroidSum, float& volumeSum, float& massSum,
		const Vector3f& localCenter,
		const Vector3f& localVelocity, const Vector3f& localAngularVelocity,
		Vector3f& netForce, Vector3f& netTorque) {
	if (node->limitReached) {
		for (uint32 i = 0; i < 8; ++i) {
			if (node->children[i] != nullptr) {
				applyBuoyancyToNode(node->children[i], body, transform, pLocal,
						nLocal,
						centroidSum, volumeSum, massSum,
						localCenter,
						localVelocity, localAngularVelocity,
						netForce, netTorque);
			}
		}

		return;
	}

	float r, s;

	for (auto* blk : node->blocks) {
		const Vector3f p(blk->position);
		AABB box(p - Vector3f(0.5f, 0.5f, 0.5f),
				p + Vector3f(0.5f, 0.5f, 0.5f));

		if (box.belowPlane(pLocal, nLocal, r, s)) {
			if (s > -r) {
				const IndexedModel& model =
						*BlockInfo::getInfo(blk->type).model;

				Vector3f centroid;
				float submergedV = model.calcSubmergedVolume(pLocal - p,
						nLocal, centroid);
				float partialMass = BlockInfo::getInfo(blk->type).mass
						* submergedV / BlockInfo::getInfo(blk->type).volume;

				if (submergedV > 0.f) {
					centroid /= submergedV;
					centroid += p;

					applyBuoyancy(localCenter, localVelocity, localAngularVelocity,
							nLocal * Physics::GRAVITY.y,
							netForce, netTorque,
							centroid, submergedV, partialMass);
				}
			}
			else {
				//applyBuoyancy(localCenter, localVelocity, localAngularVelocity,
				//		nLocal * Physics::GRAVITY.y,
				//		netForce, netTorque,
				//		p,
				//		BlockInfo::getInfo(blk->type).volume,
				//		BlockInfo::getInfo(blk->type).mass);

				centroidSum += p;
				volumeSum += BlockInfo::getInfo(blk->type).volume;
				massSum += BlockInfo::getInfo(blk->type).mass;
			}
		}
	}
}

inline static void applyBuoyancy(const Vector3f& localCenter,
		const Vector3f& localVelocity,
		const Vector3f& localAngularVelocity,
		const Vector3f& localGravity,
		Vector3f& netForce, Vector3f& netTorque,
		const Vector3f& centerOfBuoyancy, float submergedVolume,
		float partialMass) {
	Vector3f rc = centerOfBuoyancy - localCenter;

	Vector3f buoyantForce = (-localGravity 
			* submergedVolume * Ocean::DENSITY);

	Vector3f vc = localVelocity
			+ Math::cross(localAngularVelocity, rc);
	Vector3f dragForce = (partialMass * Ocean::LINEAR_DRAG)
			* (Ocean::VELOCITY - vc); // TODO: note that velocity is in world space but it's 0 so who cares

	Vector3f totalForce = buoyantForce + dragForce;

	netForce += totalForce;
	netTorque += Math::cross(rc, totalForce);

	// drag torque
	netTorque += (-partialMass * Ocean::ANGULAR_DRAG)
			* localAngularVelocity;
}


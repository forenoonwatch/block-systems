#include "broadphase.hpp"

#include "contact-manager.hpp"
#include "collision-hull.hpp"

#include <algorithm>

Physics::Broadphase::Broadphase(ContactManager& contactManager)
		: contactManager(&contactManager) {}

void Physics::Broadphase::insert(CollisionHull& hull, const AABB& aabb) {
	int32 id = tree.insert(aabb, &hull);
	hull.broadphaseIndex = id;
	moveBuffer.push_back(id);
}

void Physics::Broadphase::update(int32 id, const AABB& aabb) {
	if (tree.update(id, aabb)) {
		moveBuffer.push_back(id);
	}
}

void Physics::Broadphase::remove(CollisionHull& hull) {
	tree.remove(hull.broadphaseIndex);
}

void Physics::Broadphase::updatePairs() {
	pairBuffer.clear();

	for (uint32 i = 0; i < moveBuffer.size(); ++i) {
		currentIndex = moveBuffer[i];
		tree.query(*this, tree.getFatAABB(currentIndex));
	}

	moveBuffer.clear();

	std::sort(pairBuffer.begin(), pairBuffer.end(), [](auto& a, auto& b) {
		if (a.a < b.a) {
			return true;
		}
		else if (a.a == b.a) {
			return a.b < b.b;
		}
		else {
			return false;
		}
	});

	for (uint32 i = 0; i < pairBuffer.size(); ++i) {
		ContactPair& cp = pairBuffer[i];
		CollisionHull* hullA = (CollisionHull*)tree.getUserData(cp.a);
		CollisionHull* hullB = (CollisionHull*)tree.getUserData(cp.b);

		contactManager->addContact(*hullA, *hullB);

		for (; i < pairBuffer.size(); ++i) {
			if (cp != pairBuffer[i]) {
				break;
			}
		}
	}
}

bool Physics::Broadphase::testOverlap(int32 a, int32 b) const {
	return tree.getFatAABB(a).intersects(tree.getFatAABB(b));	
}


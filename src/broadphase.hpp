#pragma once

#include "dynamic-aabb-tree.hpp"

#include <engine/core/array-list.hpp>

namespace Physics {
	class ContactManager;
	class CollisionHull;

	class Broadphase {
		public:
			Broadphase(ContactManager& contactManager);

			void insert(CollisionHull& hull, const AABB& aabb);
			void update(int32 id, const AABB& aabb);
			void remove(CollisionHull& hull);

			void updatePairs();

			bool testOverlap(int32 a, int32 b) const;

			bool treeCallback(int32);
		private:
			NULL_COPY_AND_ASSIGN(Broadphase);

			struct ContactPair {
				inline ContactPair(int32 a, int32 b)
						: a(a)
						, b(b) {}
				
				inline bool operator==(const ContactPair& cp) const {
					return a == cp.a && b == cp.b;
				}

				inline bool operator!=(const ContactPair& cp) const {
					return !(*this == cp);
				}

				int32 a;
				int32 b;
			};

			ContactManager* contactManager;

			DynamicAABBTree tree;
			int32 currentIndex;

			ArrayList<ContactPair> pairBuffer;
			ArrayList<int32> moveBuffer;
	};
};

#include "broadphase.inl"


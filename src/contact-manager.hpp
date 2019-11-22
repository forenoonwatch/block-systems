#pragma once

#include "contact.hpp"

#include <engine/core/common.hpp>
#include <engine/core/array-list.hpp>

namespace Physics {
	class PhysicsEngine;
	class CollisionHull;

	class ContactManager {
		public:
			ContactManager(PhysicsEngine& physicsEngine);

			void testCollisions();

			void addContact(CollisionHull& a, CollisionHull& b);
			void clearContacts();
		private:
			PhysicsEngine* physicsEngine;
	};
};


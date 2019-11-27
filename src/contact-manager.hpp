#pragma once

#include "contact-constraint.hpp"

#include <engine/core/array-list.hpp>

namespace Physics {
	class PhysicsEngine;
	class CollisionHull;

	class ContactManager {
		public:
			ContactManager(PhysicsEngine& physicsEngine);

			void findNewContacts();
			void testCollisions();

			void addContact(CollisionHull& a, CollisionHull& b);
			void removeContact(ContactConstraint& constraint);
		private:
			NULL_COPY_AND_ASSIGN(ContactManager);

			PhysicsEngine* physicsEngine;

			ArrayList<ContactConstraint> contactList;

			void removeContact(uint32 i);

			friend class ContactSolver;
	};
};


#pragma once

#include "contact-constraint.hpp"
#include "broadphase.hpp"

#include <engine/core/array-list.hpp>

namespace Physics {
	class PhysicsEngine;
	class Collider;

	class ContactManager {
		public:
			ContactManager(PhysicsEngine& physicsEngine);

			void findNewContacts();
			void testCollisions();

			void addContact(Collider& a, Collider& b);
			void removeContact(ContactConstraint& constraint);

			inline Broadphase& getBroadphase() { return broadphase; }
		private:
			NULL_COPY_AND_ASSIGN(ContactManager);

			PhysicsEngine* physicsEngine;

			Broadphase broadphase;

			ArrayList<ContactConstraint> contactList;

			void removeContact(uint32 i);
	};
};


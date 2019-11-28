#pragma once

#include <engine/core/array-list.hpp>

namespace Physics {
	class Body;
	class ContactConstraint;

	class Island {
		public:
			void solve(float deltaTime);

			inline void add(Body& body);
			inline void add(ContactConstraint& constraint);
		private:
			ArrayList<Body*> bodies;
			ArrayList<ContactConstraint*> contactList;

			void clearIslandFlags();
	};
};

inline void Physics::Island::add(Physics::Body& body) {
	bodies.push_back(&body);
}

inline void Physics::Island::add(Physics::ContactConstraint& constraint) {
	contactList.push_back(&constraint);
}


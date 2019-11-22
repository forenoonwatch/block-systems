#include "contact-finder.hpp"

#include "contact-manager.hpp"
#include "body.hpp"
#include "physics.hpp"

#include "util-components.hpp"

#include <engine/core/array-list.hpp>

#include <engine/game/game.hpp>

Physics::ContactFinder::ContactFinder(Physics::PhysicsEngine& physicsEngine)
		: physicsEngine(&physicsEngine) {}

void Physics::ContactFinder::operator()(Game& game, float deltaTime) {
	physicsEngine->getContactManager().clearContacts();

	for (uint32 i = 0; i < physicsEngine->getBodies().size(); ++i) {
		for (uint32 j = i + 1; j < physicsEngine->getBodies().size(); ++j) {
			Body& bI = physicsEngine->getBodies()[i];
			Body& bJ = physicsEngine->getBodies()[j];

			//DEBUG_LOG_TEMP("Adding contact %d -> %d", i, j);
			physicsEngine->getContactManager().addContact(*bI.collisionHull,
					*bJ.collisionHull);
		}
	}
}


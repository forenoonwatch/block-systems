#include "physics.hpp"

#include "util-components.hpp"

#include "contact-constraint.hpp"
#include "island.hpp"

#include <engine/game/game.hpp>

void Physics::GravitySystem::operator()(Game& game, float deltaTime) {
	game.getECS().view<Physics::BodyHandle>().each([&](
			Physics::BodyHandle& handle) {
		if (handle.body->isDynamic()) {
			handle.body->applyForce(Physics::GRAVITY);
		}
		else {
			// TODO: FIXME: TEMP: remove this
			handle.body->velocity = Vector3f(1.f, 0.f, 0.f);
		}
	});
}

Physics::PhysicsEngine::PhysicsEngine()
		: contactManager(*this) {}

Physics::Body* Physics::PhysicsEngine::addBody() {
	Body* body = new Body();
	body->index = bodies.size();
	bodies.push_back(body);
	
	return body;
}

void Physics::PhysicsEngine::operator()(Game& game, float deltaTime) {
	contactManager.testCollisions();
	
	game.getECS().view<TransformComponent, Physics::BodyHandle>().each([&](
			TransformComponent& tf, Physics::BodyHandle& handle) {
		handle.body->transform = tf.transform;
		handle.body->worldCenter = tf.transform.transform(
				handle.body->localCenter, 1.f);
	});

	for (Body* body : bodies) {
		body->flags &= ~Body::FLAG_ISLAND;
	}

	ArrayList<Body*> bodyStack;

	for (uint32 i = 0; i < bodies.size(); ++i) {
		Body* seed = bodies[i];

		if (seed->isInIsland() || !seed->isAwake() || seed->isStatic()) {
			continue;
		}

		Island island;

		seed->setInIsland();

		bodyStack.clear();
		bodyStack.push_back(seed);

		while (!bodyStack.empty()) {
			Body* body = bodyStack.back();
			bodyStack.pop_back();

			island.add(*body);

			body->setToAwake();

			if (body->isStatic()) {
				continue;
			}

			for (ContactEdge* edge : body->contactList) {
				ContactConstraint* cc = edge->constraint;

				if (cc->isInIsland() || !cc->isColliding()) {
					continue;
				}

				// TODO: skip sensors

				cc->setInIsland();
				island.add(*cc);

				if (edge->other->isInIsland()) {
					continue;
				}

				bodyStack.push_back(edge->other);
				edge->other->setInIsland();
			}
		}

		island.solve(deltaTime);
	}
	
	contactManager.findNewContacts();

	for (Body* body : bodies) {
		body->force = Vector3f();
		body->torque = Vector3f();
	}

	game.getECS().view<TransformComponent, Physics::BodyHandle>().each([&](
			TransformComponent& tf, Physics::BodyHandle& handle) {
		tf.transform = handle.body->transform;
	});
}

Physics::PhysicsEngine::~PhysicsEngine() {
	for (Body* body : bodies) {
		delete body;
	}
}


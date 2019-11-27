#include "physics.hpp"

#include "util-components.hpp"

#include <engine/game/game.hpp>

#define NUM_ITERATIONS 10

static Quaternion integrateAngularVelocity(const Quaternion& rot,
		const Vector3f& angularVelocity, float deltaTime);

void Physics::GravitySystem::operator()(Game& game, float deltaTime) {
	game.getECS().view<Physics::BodyHandle>().each([&](
			Physics::BodyHandle& handle) {
		if (!(handle.body->flags & Physics::Body::FLAG_STATIC)) {
			handle.body->applyForce(Physics::GRAVITY);
		}
		else {
			// TODO: FIXME: TEMP: remove this
			handle.body->velocity = Vector3f(1.f, 0.f, 0.f);
		}
	});
}

Physics::PhysicsEngine::PhysicsEngine() {}

Physics::Body* Physics::PhysicsEngine::addBody() {
	Body* body = new Body();
	body->index = bodies.size();
	bodies.push_back(body);
	
	return body;
}

void Physics::PhysicsEngine::operator()(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, Physics::BodyHandle>().each([&](
			TransformComponent& tf, Physics::BodyHandle& handle) {
		handle.body->transform = tf.transform;
		handle.body->worldCenter = tf.transform.transform(
				handle.body->localCenter, 1.f);

		const Matrix3f rot = Math::quatToMat3(tf.transform.getRotation());
		handle.body->invInertiaWorld = rot * handle.body->invInertiaLocal
				* Math::transpose(rot);
	});

	contacts.clear();

	for (uint32 i = 0; i < bodies.size(); ++i) {
		Body& a = *bodies[i];

		for (uint32 j = i + 1; j < bodies.size(); ++j) {
			Body& b = *bodies[j];

			if ((a.flags & Body::FLAG_STATIC)
					&& (b.flags & Body::FLAG_STATIC)) {
				continue;
			}

			Manifold m(a, b);
			
			if (m.testCollision()) {
				contacts.emplace_back(m);
			}
		}
	}

	for (Body* body : bodies) {
		// integrate velocities
		body->velocity += body->force * body->invMass * deltaTime;
		body->angularVelocity += body->invInertiaWorld * body->torque
				* deltaTime;
	}

	for (auto& contact : contacts) {
		contact.preSolve(deltaTime);
	}

	for (uint32 i = 0; i < NUM_ITERATIONS; ++i) {
		for (auto& contact : contacts) {
			contact.solve();
		}
	}

	for (uint32 i = 0; i < bodies.size(); ++i) {
		Body& body = *bodies[i];

		// integrate positions
		body.transform.setPosition(body.transform.getPosition()
				+ body.velocity * deltaTime);
		body.transform.setRotation(integrateAngularVelocity(
				body.transform.getRotation(), body.angularVelocity,
				deltaTime));
	}

	for (Body* body : bodies) {
		body->force = Vector3f();
		body->torque = Vector3f();
	}

	game.getECS().view<TransformComponent, Physics::BodyHandle>().each([&](
			TransformComponent& tf, Physics::BodyHandle& handle) {
		tf.transform = handle.body->transform;
	});

	contacts.clear();
}

Physics::PhysicsEngine::~PhysicsEngine() {
	for (Body* body : bodies) {
		delete body;
	}
}

inline static Quaternion integrateAngularVelocity(const Quaternion& rot,
		const Vector3f& angularVelocity, float deltaTime) {
	// wxyz
	Quaternion q(0.f, angularVelocity.x * deltaTime,
			angularVelocity.y * deltaTime, angularVelocity.z * deltaTime);

	q *= rot;
	q = rot + q * 0.5f;

	return Math::normalize(q);
}


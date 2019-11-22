#include "physics.hpp"

#include "contact-solver.hpp"

#include "util-components.hpp"

#include <engine/game/game.hpp>

#define NUM_ITERATIONS 10

static Quaternion integrateAngularVelocity(const Quaternion& rot,
		const Vector3f& angularVelocity, float deltaTime);

void Physics::GravitySystem::operator()(Game& game, float deltaTime) {
	game.getECS().view<Physics::BodyHandle>().each([&](
			Physics::BodyHandle handle) {
		if (!(handle.body->flags & Physics::Body::FLAG_STATIC)) {
			handle.body->applyForce(Physics::GRAVITY);
		}
	});
}

Physics::PhysicsEngine::PhysicsEngine()
		: contactManager(*this) {}

Physics::Body* Physics::PhysicsEngine::addBody() {
	bodies.emplace_back();
	bodies.back().index = bodies.size() - 1;
	
	return &bodies.back();
}

void Physics::PhysicsEngine::operator()(Game& game, float deltaTime) {
	contactManager.testCollisions();
	initConstraintStates();

	velocityStates.clear();

	game.getECS().view<TransformComponent, Physics::BodyHandle>().each([&](
			TransformComponent& tf, Physics::BodyHandle& handle) {
		handle.body->transform = tf.transform;
	});

	for (auto& body : bodies) {
		// integrate velocities
		body.worldCenter = body.transform.transform(body.localCenter, 1.f);

		const Matrix3f rot = Math::quatToMat3(body.transform.getRotation());
		body.invInertiaWorld = rot * body.invInertiaLocal
				* Math::transpose(rot);
			
		body.velocity += body.force * body.invMass * deltaTime;
		body.angularVelocity += body.invInertiaWorld * body.torque
				* deltaTime;

		// TODO: damping

		// add to velocity states
		velocityStates.emplace_back(body.velocity, body.angularVelocity);
	}

	ContactSolver contactSolver(*this);

	contactSolver.preSolve(deltaTime);

	for (uint32 i = 0; i < NUM_ITERATIONS; ++i) {
		contactSolver.solve();
	}

	contactSolver.flush();

	for (uint32 i = 0; i < bodies.size(); ++i) {
		// copy velocity states
		Body& body = bodies[i];
		VelocityState& vs = velocityStates[i];

		body.velocity = vs.v;
		body.angularVelocity = vs.w;

		// integrate positions
		body.transform.setPosition(body.transform.getPosition()
				+ body.velocity * deltaTime);
		body.transform.setRotation(integrateAngularVelocity(
				body.transform.getRotation(), body.angularVelocity,
				deltaTime));
	}

	// TODO: sleep

	for (auto& body : bodies) {
		body.force = Vector3f();
		body.torque = Vector3f();
	}

	game.getECS().view<TransformComponent, Physics::BodyHandle>().each([&](
			TransformComponent& tf, Physics::BodyHandle& handle) {
		tf.transform = handle.body->transform;
	});

	contacts.clear();
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

inline void Physics::PhysicsEngine::initConstraintStates() {
	contactStates.clear();

	for (uint32 i = 0; i < contacts.size(); ++i) {
		ContactConstraint& cc = contacts[i];
		
		contactStates.emplace_back();
		ContactConstraintState& ccs = contactStates.back();

		ccs.centerA = cc.bodyA->worldCenter;
		ccs.centerB = cc.bodyB->worldCenter;
		
		ccs.iA = cc.bodyA->invInertiaWorld;
		ccs.iB = cc.bodyB->invInertiaWorld;
		ccs.mA = cc.bodyA->invMass;
		ccs.mB = cc.bodyB->invMass;
		
		ccs.restitution = cc.restitution;
		ccs.friction = cc.friction;
		
		ccs.indexA = cc.bodyA->index;
		ccs.indexB = cc.bodyB->index;

		ccs.normal = cc.manifold.normal;
		
		ccs.tangents[0] = cc.manifold.tangents[0];
		ccs.tangents[1] = cc.manifold.tangents[1];
		
		ccs.numContacts = cc.manifold.numContacts;

		for (uint32 j = 0; j < ccs.numContacts; ++j) {
			ContactState& cs = ccs.contacts[j];
			Contact& cp = cc.manifold.contacts[j];

			cs.rA = cp.position - ccs.centerA;
			cs.rB = cp.position - ccs.centerB;

			cs.penetration = cp.penetration;
			
			cs.normalImpulse = cp.normalImpulse;
			cs.tangentImpulse[0] = cp.tangentImpulse[0];
			cs.tangentImpulse[1] = cp.tangentImpulse[1];
		}
	}
}


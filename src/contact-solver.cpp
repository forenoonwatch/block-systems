#include "contact-solver.hpp"

#include "physics.hpp"
#include "body.hpp"

#include <engine/math/math.hpp>

#define BAUMGARTE 0.2f
#define PENETRATION_SLOP 0.5f

Physics::ContactSolver::ContactSolver(Physics::PhysicsEngine& physicsEngine)
		: physicsEngine(&physicsEngine)
		, frictionEnabled(true) {

}

void Physics::ContactSolver::preSolve(float deltaTime) {
	ArrayList<Physics::VelocityState>& velocities
			= physicsEngine->getVelocityStates();
	ArrayList<Physics::ContactConstraintState>& contacts
			= physicsEngine->getContactStates();

	for (auto& ccs : contacts) {
		Vector3f& vA = velocities[ccs.indexA].v;
		Vector3f& wA = velocities[ccs.indexA].w;
		Vector3f& vB = velocities[ccs.indexB].v;
		Vector3f& wB = velocities[ccs.indexB].w;

		for (uint32 j = 0; j < ccs.numContacts; ++j) {
			Physics::ContactState& cs = ccs.contacts[j];

			// pre-calculate JM^-1JT for contact and friction constraints
			Vector3f raCn = Math::cross(cs.rA, ccs.normal);
			Vector3f rbCn = Math::cross(cs.rB, ccs.normal);
			
			float nm = ccs.mA + ccs.mB;
			float tm[2] = {nm, nm};

			nm += Math::dot(raCn, ccs.iA * raCn)
					+ Math::dot(rbCn, ccs.iB * rbCn);
			cs.normalMass = nm == 0.f ? 0.f : 1.f / nm;

			for (uint32 i = 0; i < 2; ++i) {
				Vector3f raCt = Math::cross(ccs.tangents[i], cs.rA);
				Vector3f rbCt = Math::cross(ccs.tangents[i], cs.rB);

				tm[i] += Math::dot(raCt, ccs.iA * raCt)
						+ Math::dot(rbCt, ccs.iB * rbCt);
				cs.tangentMass[i] = tm[i] == 0.f ? 0.f : 1.f / tm[i];
			}

			// pre-calculate bias factor
			cs.bias = -BAUMGARTE * (1.f / deltaTime)
					* Math::min(0.f, cs.penetration + PENETRATION_SLOP);

			// add in restitution bias
			float dv = Math::dot(vB + Math::cross(wB, cs.rB)
					- vA - Math::cross(wA, cs.rA), ccs.normal);

			if (dv < -1.f) {
				cs.bias += -(ccs.restitution) * dv;
			}
		}

		velocities[ccs.indexA].v = vA;
		velocities[ccs.indexA].w = wA;
		velocities[ccs.indexB].v = vB;
		velocities[ccs.indexB].w = wB;
	}
}

void Physics::ContactSolver::solve() {
	ArrayList<Physics::VelocityState>& velocities
			= physicsEngine->getVelocityStates();
	ArrayList<Physics::ContactConstraintState>& contacts
			= physicsEngine->getContactStates();

	for (auto& ccs : contacts) {
		Vector3f& vA = velocities[ccs.indexA].v;
		Vector3f& wA = velocities[ccs.indexA].w;
		Vector3f& vB = velocities[ccs.indexB].v;
		Vector3f& wB = velocities[ccs.indexB].w;

		for (uint32 j = 0; j < ccs.numContacts; ++j) {
			Physics::ContactState& cs = ccs.contacts[j];

			// relative velocity at contact
			Vector3f dv = vB + Math::cross(wB, cs.rB)
					- vA - Math::cross(wA, cs.rA);

			// friction
			if (frictionEnabled) {
				for (uint32 i = 0; i < 2; ++i) {
					float lambda = -Math::dot(dv, ccs.tangents[i])
							* cs.tangentMass[i];

					// calculate frictional impulse
					float maxLambda = ccs.friction * cs.normalImpulse;

					// clamp frictional impulse
					float oldPT = cs.tangentImpulse[i];
					cs.tangentImpulse[i] = Math::clamp(-maxLambda, maxLambda,
							oldPT + lambda);
					lambda = cs.tangentImpulse[i] - oldPT;

					// apply friction impulse
					Vector3f impulse = ccs.tangents[i] * lambda;
					vA -= impulse * ccs.mA;
					wA -= ccs.iA * Math::cross(cs.rA, impulse);

					vB += impulse * ccs.mB;
					wB += ccs.iB * Math::cross(cs.rB, impulse);
				}
			}

			// normal
			{
				dv = vB + Math::cross(wB, cs.rB)
						- vA - Math::cross(wA, cs.rA);

				// normal impulse
				float vn = Math::dot(dv, ccs.normal);

				// factor in positional bias to calculate impulse scalar j
				float lambda = cs.normalMass * (-vn + cs.bias);

				// Clamp impulse
				float tempPN = cs.normalImpulse;
				cs.normalImpulse = Math::max(tempPN + lambda, 0.f);
				lambda = cs.normalImpulse - tempPN;

				DEBUG_LOG_TEMP("%.2f", tempPN);

				// Apply impulse
				Vector3f impulse = ccs.normal * lambda;
				vA -= impulse * ccs.mA;
				wA -= ccs.iA * Math::cross(cs.rA, impulse);

				vB += impulse * ccs.mB;
				wB += ccs.iB * Math::cross(cs.rB, impulse);
			}
		}

		velocities[ccs.indexA].v = vA;
		velocities[ccs.indexA].w = wA;
		velocities[ccs.indexB].v = vB;
		velocities[ccs.indexB].w = wB;
	}
}

void Physics::ContactSolver::flush() {
	ArrayList<Physics::ContactConstraint>& contactConstraints
			= physicsEngine->getContacts();
	ArrayList<Physics::ContactConstraintState>& contacts
			= physicsEngine->getContactStates();

	for (uint32 i = 0; i < contacts.size(); ++i) {
		Physics::ContactConstraintState& ccs = contacts[i];
		Physics::Manifold& manifold = contactConstraints[i].manifold;

		for (uint32 j = 0; j < ccs.numContacts; ++j) {
			Physics::Contact& oc = manifold.contacts[j];
			Physics::ContactState& cs = ccs.contacts[j];
			
			oc.normalImpulse = cs.normalImpulse;
			oc.tangentImpulse[0] = cs.tangentImpulse[0];
			oc.tangentImpulse[1] = cs.tangentImpulse[1];
		}
	}
}


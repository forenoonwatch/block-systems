#include "ship.hpp"

#include "util-components.hpp"
#include "renderable-mesh.hpp"

#include "game-render-context.hpp"

#include "physics.hpp"
#include "body.hpp"

#include "ocean.hpp"

#include <engine/game/game.hpp>

#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/material.hpp>

#include <engine/math/aabb.hpp>

#include <engine/core/util.hpp>

#include <cstdlib>
#include <fstream>

static Matrix3f calcBlockTensor(const Vector3f& localPos, float mass);

bool Ship::load(const String& fileName) {
	std::ifstream file(fileName);

	// FORMAT: ID, x, y, z, rx, ry, rz, rw
	// n = 8

	if (file.is_open()) {
		String line;
		ArrayList<String> tokens;

		while (file.good()) {
			std::getline(file, line);

			tokens.clear();
			Util::split(tokens, line, ',');

			if (tokens.size() < 8) {
				continue;
			}

			if (tokens[0].length() == 0 || tokens[0][0] == '#') {
				continue;
			}

			addBlock(std::atoi(tokens[0].c_str()),
					Vector3i(std::atoi(tokens[1].c_str()),
							std::atoi(tokens[2].c_str()),
							std::atoi(tokens[3].c_str())),
					Quaternion(std::atof(tokens[7].c_str()),
							std::atof(tokens[4].c_str()),
							std::atof(tokens[5].c_str()),
							std::atof(tokens[6].c_str())));
		}
	}
	else {
		return false;
	}

	return true;
}

void Ship::addBlock(uint32 type,
		const Vector3i& position, const Quaternion& rotation) {
	Block block;
	block.type = type;
	block.position = position;
	block.rotation = rotation;

	block.renderIndex = offsets[block.type].size();

	blocks[position] = block;
	blockTree.add(blocks[position]);
	
	offsets[block.type].push_back(Math::translate(Matrix4f(1.f),
			Vector3f(position)) * Math::quatToMat4(rotation));
	offsetIndices[block.type].push_back(&blocks[position]);

	changedBuffers.insert(block.type);

	const float blockMass = BlockInfo::getInfo(block.type).mass;

	totalMass += blockMass;
	inertiaSum += calcBlockTensor(Vector3f(block.position), blockMass);
	localCenterSum += Vector3f(block.position) * blockMass;
	massChanged = true;
}

void Ship::removeBlock(Block& block) {
	Block* back = offsetIndices[block.type].back();

	back->renderIndex = block.renderIndex;
	
	offsets[block.type][block.renderIndex] = offsets[block.type].back();
	offsetIndices[block.type][block.renderIndex] = back;

	offsets[block.type].pop_back();
	offsetIndices[block.type].pop_back();

	blocks.erase(block.position);
	blockTree.remove(block);

	changedBuffers.insert(block.type);

	const float blockMass = BlockInfo::getInfo(block.type).mass;

	totalMass -= blockMass;
	inertiaSum -= calcBlockTensor(Vector3f(block.position), blockMass);
	localCenterSum -= Vector3f(block.position) * blockMass;
	massChanged = true;
}

void rayShipIntersection(const Matrix4f& shipTransform, const Ship& ship,
		const Vector3f& origin, const Vector3f& direction, Block*& block,
		Vector3f* hitPosition, Vector3f* hitNormal) {
	const Matrix4f itf = Math::inverse(shipTransform);

	const Vector3f tfOrigin(itf * Vector4f(origin, 1.f));
	const Vector3f tfDirection(itf * Vector4f(direction, 0.f));

	Vector3i blockPosition;

	block = nullptr;

	const Vector3i directions[] = {Vector3i(-1, 0, 0), Vector3i(1, 0, 0),
			Vector3i(0, -1, 0), Vector3i(0, 1, 0), Vector3i(0, 0, -1),
			Vector3i(0, 0, 1)};

	if (ship.blockTree.intersectsRay(tfOrigin, tfDirection,
			&blockPosition, hitPosition)) {
		block = &const_cast<Ship&>(ship).blocks[blockPosition];
		
		float maxDot = 0.f;
		uint32 maxI = (uint32)-1;

		for (uint32 i = 0; i < ARRAY_SIZE_IN_ELEMENTS(directions); ++i) {
			const float d = Math::dot(*hitPosition - Vector3f(blockPosition),
					Vector3f(directions[i]));

			if (d > maxDot) {
				maxDot = d;
				maxI = i;
			}
		}

		//*hitPosition = Vector3f(shipTransform * Vector4f(*hitPosition, 1.f));
		//*hitNormal = Vector3f(shipTransform * Vector4f(directions[maxI], 0.f));
		*hitNormal = Vector3f(directions[maxI]);
	}	
}

void ShipBuoyancySystem::operator()(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, Ship,
			Physics::BodyHandle>().each([&](TransformComponent& tf, Ship& ship,
			Physics::BodyHandle& handle) {
		//body.worldCenter = tf.transform.transform(body.localCenter, 1.f);

		/*const Vector3f pWorld(0.f, 0.f, 0.f);
		const Vector3f nWorld(0.f, 1.f, 0.f);

		const Matrix4f tfi = tf.transform.inverse();
		const Vector3f p(tfi * Vector4f(pWorld, 1.f));
		const Vector3f n(tfi * Vector4f(nWorld, 0.f));

		const Vector3f vLocal = tf.transform.inverseTransform(body.velocity, 0.f);
		const Vector3f avLocal = tf.transform.inverseTransform(body.angularVelocity, 0.f);

		Vector3f netForce(0.f, 0.f, 0.f);
		Vector3f netTorque(0.f, 0.f, 0.f);
		
		for (auto& pair : ship.blocks) {
			const Vector3f pp(pair.first);
			const AABB box(pp - Vector3f(0.5f, 0.5f, 0.5f),
					pp + Vector3f(0.5f, 0.5f, 0.5f));

			float r, dPPP;

			if (box.belowPlane(p, n, r, dPPP)) {
				const float mass = BlockInfo::getInfo(pair.second.type).mass;
				const float totalV =
						BlockInfo::getInfo(pair.second.type).volume;
				
				Vector3f cob;
				float submergedV;

				if (dPPP > -r) {
					const IndexedModel& model =
							*BlockInfo::getInfo(pair.second.type).model;

					submergedV = model.calcSubmergedVolume(p - pp, n, cob);

					if (submergedV != 0.f) {
						cob /= submergedV;
						cob += pp;
					}
				}
				else {
					cob = pp;
					submergedV = totalV;
				}
				
				//cob = tf.transform.transform(cob, 1.f);
				float partialMass = mass * submergedV / totalV;

				//Vector3f rc = cob - body.worldCenter;
				Vector3f rc = cob - body.localCenter;

				//Vector3f buoyantForce = (-Physics::GRAVITY 
				//			* submergedV * Ocean::DENSITY);
				Vector3f buoyantForce = -n * Physics::GRAVITY.y
						* submergedV * Ocean::DENSITY;

				//Vector3f vc = body.velocity
				//		+ Math::cross(body.angularVelocity, rc);
				Vector3f vc = vLocal
						+ Math::cross(avLocal, rc);
				Vector3f dragForce = (partialMass * Ocean::LINEAR_DRAG)
						* (Ocean::VELOCITY - vc);

				Vector3f totalForce = buoyantForce + dragForce;

				//body.force += totalForce;
				//body.torque += Math::cross(rc, totalForce);
				netForce += totalForce;
				netTorque += Math::cross(rc, totalForce);

				// drag torque
				//body.torque += (-partialMass * Ocean::ANGULAR_DRAG)
				//		* body.angularVelocity;
				netTorque += (-partialMass * Ocean::ANGULAR_DRAG)
						* avLocal;
			}
		}

		netForce = tf.transform.transform(netForce, 0.f);
		netTorque = tf.transform.transform(netTorque, 0.f);

		body.force += netForce;
		body.torque += netTorque;*/

		ship.blockTree.applyBuoyantForce(*handle.body, tf.transform,
				Ocean::POSITION, Ocean::NORMAL);
	});
}

void ShipUpdateMassSystem::operator()(Game& game, float deltaTime) {
	game.getECS().view<Ship, Physics::BodyHandle>().each([&](Ship& ship,
			Physics::BodyHandle& handle) {
		if (ship.massChanged) {
			ship.massChanged = false;

			Physics::Body& body = *handle.body;

			body.mass = ship.totalMass;

			if (body.mass > 0.f) {
				body.invMass = 1.f / body.mass;
				body.localCenter = ship.localCenterSum * body.invMass;
				
				const Matrix3f inertia = ship.inertiaSum
						- (Matrix3f(Math::dot(body.localCenter,
							body.localCenter))
						- Math::outerProduct(body.localCenter,
							body.localCenter)) * body.mass;

				body.invInertiaLocal = Math::inverse(inertia);
			}
			else {
				body.invMass = 0.f;
				body.localCenter = Vector3f(0.f, 0.f, 0.f);
				body.invInertiaLocal = Matrix3f(0.f);
			}
		}
	});
}

void ShipUpdateVAOSystem::operator()(Game& game, float deltaTime) {
	game.getECS().view<Ship>().each([&](Ship& ship) {
		if (ship.changedBuffers.empty()) {
			return;
		}

		for (auto& type : ship.changedBuffers) {
			ship.blockArrays[type]->updateBuffer(4,
					&ship.offsets[type][0], ship.offsets[type].size()
					* sizeof(Matrix4f));
		}

		ship.changedBuffers.clear();
	});
}

void ShipRenderSystem::operator()(Game& game, float deltaTime) {
	Shader& shader = game.getAssetManager().getShader("ship-shader");
	GameRenderContext* grc = (GameRenderContext*)game.getRenderContext();
	Material* currentMaterial = nullptr;

	game.getECS().view<TransformComponent, Ship>().each([&](
			TransformComponent& transform, Ship& ship) {
		uint32 numTransforms;

		shader.setMatrix4f("baseTransform", transform.transform.toMatrix());

		for (auto& pair : ship.offsets) {
			numTransforms = pair.second.size();

			if (numTransforms == 0) {
				continue;
			}

			Material* material = BlockInfo::getInfo(pair.first).material;

			if (material != currentMaterial) {
				currentMaterial = material;

				shader.setSampler("diffuse", *material->diffuse,
						grc->getLinearMipmapSampler(), 0);
				shader.setSampler("normalMap", *material->normalMap,
						grc->getLinearMipmapSampler(), 1);
				shader.setSampler("materialMap", *material->materialMap,
						grc->getLinearMipmapSampler(), 2);
			}

			grc->draw(grc->getTarget(), shader,
					*ship.blockArrays[pair.first], GL_TRIANGLES,
					numTransforms);
		}
	});
}

inline static Matrix3f calcBlockTensor(const Vector3f& localPos, float mass) {
	return Matrix3f(mass / 6.f) + (Matrix3f(Math::dot(localPos, localPos))
			- Math::outerProduct(localPos, localPos)) * mass;
}


#include "ship.hpp"

#include "util-components.hpp"
#include "renderable-mesh.hpp"

#include "game-render-context.hpp"

#include <engine/game/game.hpp>

#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/material.hpp>

inline static void calcBlockTensor(const Vector3f& localPos, float mass,
		Matrix3f& inertia) {
	inertia = Matrix3f(mass / 6.f);
	inertia += (Matrix3f(Math::dot(localPos, localPos))
			- Math::outerProduct(localPos, localPos)) * mass;
}

void calcMassData(const Ship& ship, float& mass, float& invMass,
		Vector3f& localCenter, Matrix3f& inertia) {
	mass = 0.f;
	invMass = 0.f;
	localCenter = Vector3f(0.f, 0.f, 0.f);
	inertia = Matrix3f(0.f);
	
	Matrix3f localInertia;

	for (auto& pair : ship.blocks) {
		const float blockMass = ship.blockInfo[pair.second.type].mass;
		calcBlockTensor(Vector3f(pair.first), blockMass, localInertia);

		mass += blockMass;
		inertia += localInertia;
		localCenter += Vector3f(pair.first) * blockMass;
	}

	if (mass > 0.f) {
		invMass = 1.f / mass;
		localCenter *= invMass;
		inertia -= (Matrix3f(Math::dot(localCenter, localCenter))
				- Math::outerProduct(localCenter, localCenter)) * mass;
	}
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

	if (ship.hitTree.intersectsRay(tfOrigin, tfDirection,
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

void shipRenderSystem(Game& game, float deltaTime) {
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

			Material* material = ship.blockInfo[pair.first].material;

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
					*ship.blockInfo[pair.first].vertexArray, GL_TRIANGLES,
					numTransforms);
		}
	});
}


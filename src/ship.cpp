#include "ship.hpp"

#include "util-components.hpp"
#include "camera.hpp"

#include "game-render-context.hpp"

#include <engine/game/game.hpp>
#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/material.hpp>

#include <cfloat>

void ShipPickBlockSystem::operator()(Game& game, float deltaTime) { 
	const CameraComponent& cc = game.getECS().get<CameraComponent>(cameraInfo);
	const Vector4f tfOrigin(cc.position, 1.f);
	const Vector4f tfDirection(cc.rayDirection, 0.f);

	Vector3f intersectPos;
	Vector3f intersectNormal;

	game.getECS().view<TransformComponent, Ship>().each([&](
			TransformComponent& transform, Ship& ship) {
		const Block* block = nullptr;
		float minDist = FLT_MAX;

		for (ArrayList<Block>::const_iterator it = ship.blocks.cbegin(),
				end = ship.blocks.cend(); it != end; ++it) {
			const Matrix4f tf = transform.transform * it->offset;
			const Matrix4f tfi = Math::inverse(tf);

			if (BlockInfo::getInfo(it->type).model->intersectsRay(
					Vector3f(tfi * tfOrigin), Vector3f(tfi * tfDirection),
					&intersectPos, &intersectNormal)) {
				const float dist = Math::length(cc.position
						- Vector3f(tf * Vector4f(intersectPos, 1.f)));

				if (dist < minDist) {
					minDist = dist;
					block = &(*it);
				}
			}
		}

		if (block != nullptr) {
			DEBUG_LOG_TEMP2("Intersecting");
		}
	});
}

void shipRenderSystem(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, Ship>().each([&](TransformComponent& transform,
			Ship& ship) {
		for (ArrayList<Block>::const_iterator it = ship.blocks.cbegin(),
				end = ship.blocks.cend(); it != end; ++it) {
			((GameRenderContext*)game.getRenderContext())->renderMesh(
					*((VertexArray*)BlockInfo::getInfo(it->type).vertexArray),
					*((Material*)BlockInfo::getInfo(it->type).material),
					transform.transform * it->offset);
		}
	});
}


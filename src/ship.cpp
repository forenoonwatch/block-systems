#include "ship.hpp"

#include "util-components.hpp"
#include "camera.hpp"

#include "game-render-context.hpp"

#include <engine/core/application.hpp>

#include <engine/game/game.hpp>

#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/material.hpp>

#include <engine/math/math.hpp>

#include <cfloat>

void ShipPickBlockSystem::operator()(Game& game, float deltaTime) { 
	if (Application::getMousePressed(Input::MOUSE_BUTTON_LEFT)) {
		const CameraComponent& cc = game.getECS().get<CameraComponent>(cameraInfo);
		const Vector4f tfOrigin(cc.position, 1.f);
		const Vector4f tfDirection(cc.rayDirection, 0.f);
		const IndexedModel& cube = game.getAssetManager().getModel("cube");

		Vector3f intersectPos;
		Vector3f intersectNormal;

		game.getECS().view<TransformComponent, Ship, ShipBuildInfo>().each([&](
				TransformComponent& transform, Ship& ship, ShipBuildInfo& sbi) {
			uint32 index = (uint32)-1;
			float minDist = FLT_MAX;
			Vector3f mousePos;
			Vector3f hitNormal;

			for (uint32 i = 0; i < ship.blocks.size(); ++i) {
				const Matrix4f tf = transform.transform * ship.blocks[i].offset;
				const Matrix4f tfi = Math::inverse(tf);

				if (cube.intersectsRay(Vector3f(tfi * tfOrigin), Vector3f(tfi * tfDirection),
						&intersectPos, &intersectNormal)) {
					intersectPos = Vector3f(tf * Vector4f(intersectPos, 1.f));
					const float dist = Math::length(cc.position - intersectPos);

					if (dist < minDist) {
						minDist = dist;
						index = i;
						mousePos = intersectPos;
						hitNormal = Vector3f(ship.blocks[i].offset * Vector4f(intersectNormal, 0.f));
					}
				}
			}

			if (index != (uint32)-1) {
				if (Application::isKeyDown(Input::KEY_LEFT_SHIFT)) {
					if (ship.blocks.size() > 1) {
						ship.blocks[index] = ship.blocks.back();
						ship.blocks.pop_back();
					}
				}
				else {
					Vector3f pos(ship.blocks[index].offset[3]);
					pos += hitNormal * BlockInfo::OFFSET_SCALE;

					Block block;
					block.type = sbi.objectType;
					block.offset = Math::translate(Matrix4f(1.f), pos)
							* sbi.transform;
					ship.blocks.push_back(block);
				}
			}
		});
	}
}

void updateShipBuildInfo(Game& game, float deltaTime) {
	game.getECS().view<ShipBuildInfo>().each([&](ShipBuildInfo& sbi) {
		if (Application::getKeyPressed(Input::KEY_R)) {
			sbi.transform = Math::rotate(sbi.transform, Math::toRadians(90.f),
					Vector3f(0.f, 1.f, 0.f));
		}

		if (Application::getKeyPressed(Input::KEY_T)) {
			sbi.transform = Math::rotate(sbi.transform, Math::toRadians(90.f),
					Vector3f(1.f, 0.f, 0.f));
		}

		if (Application::getKeyPressed(Input::KEY_F)) {
			sbi.objectType = (enum BlockInfo::BlockType)(((int32)sbi.objectType + 1)
					% BlockInfo::NUM_TYPES);
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


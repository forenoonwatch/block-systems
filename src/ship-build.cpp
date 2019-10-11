#include "ship-build.hpp"

#include "ship.hpp"

#include "util-components.hpp"
#include "renderable-mesh.hpp"
#include "camera.hpp"

#include "game-render-context.hpp"

#include <engine/core/application.hpp>

#include <engine/game/game.hpp>

#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/material.hpp>

#include <engine/math/math.hpp>

#include <cfloat>

static void rayShipIntersection(const IndexedModel& cube, const Matrix4f& shipTransform,
		const Ship& ship, const Vector3f& origin, const Vector3f& direction, uint32* index,
		Vector3f* hitPosition, Vector3f* hitNormal);

void ShipPickBlockSystem::operator()(Game& game, float deltaTime) { 
	if (Application::getMousePressed(Input::MOUSE_BUTTON_LEFT)) {
		const CameraComponent& cc = game.getECS().get<CameraComponent>(cameraInfo);
		const IndexedModel& cube = game.getAssetManager().getModel("cube");

		uint32 index;
		Vector3f mousePos;
		Vector3f hitNormal;

		game.getECS().view<TransformComponent, Ship, ShipBuildInfo>().each([&](
				TransformComponent& transform, Ship& ship, ShipBuildInfo& sbi) {
			rayShipIntersection(cube, transform.transform, ship, cc.position,
					cc.rayDirection, &index, &mousePos, &hitNormal);

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

UpdateBuildToolTip::UpdateBuildToolTip(Game& game, ECS::Entity cameraInfo)
		: toolTip(game.getECS().create())
		, cameraInfo(cameraInfo) {
	game.getECS().assign<TransformComponent>(toolTip, Matrix4f(1.f));
	game.getECS().assign<RenderableMesh>(toolTip,
			const_cast<VertexArray*>(BlockInfo::getInfo(BlockInfo::TYPE_BASIC_CUBE).vertexArray),
			const_cast<Material*>(BlockInfo::getInfo(BlockInfo::TYPE_BASIC_CUBE).material),
			false);
}

void UpdateBuildToolTip::operator()(Game& game, float deltaTime) {
	const CameraComponent& cc = game.getECS().get<CameraComponent>(cameraInfo);
	const IndexedModel& cube = game.getAssetManager().getModel("cube");

	uint32 index;
	Vector3f mousePos;
	Vector3f hitNormal;

	TransformComponent& tf = game.getECS().get<TransformComponent>(toolTip);
	RenderableMesh& rm = game.getECS().get<RenderableMesh>(toolTip);

	game.getECS().view<ShipBuildInfo, TransformComponent, Ship>().each([&](ShipBuildInfo& sbi,
			TransformComponent& transform, Ship& ship) {
		rayShipIntersection(cube, transform.transform, ship, cc.position,
					cc.rayDirection, &index, &mousePos, &hitNormal);

		rm.render = index != (uint32)-1;

		if (rm.render) {
			Vector3f pos(ship.blocks[index].offset[3]);
			pos += hitNormal * BlockInfo::OFFSET_SCALE;
			tf.transform = transform.transform
					* Math::translate(Matrix4f(1.f), pos) * sbi.transform;
		}

		rm.vertexArray = const_cast<VertexArray*>(BlockInfo::getInfo(sbi.objectType).vertexArray);
		rm.material = const_cast<Material*>(BlockInfo::getInfo(sbi.objectType).material);
	});
}

void updateShipBuildInfo(Game& game, float deltaTime) {
	game.getECS().view<ShipBuildInfo>().each([&](ShipBuildInfo& sbi) {
		if (Application::getKeyPressed(Input::KEY_R)) {
			sbi.transform = Math::rotate(Matrix4f(1.f), Math::toRadians(90.f),
					Vector3f(0.f, 1.f, 0.f)) * sbi.transform;
		}

		if (Application::getKeyPressed(Input::KEY_T)) {
			sbi.transform = Math::rotate(Matrix4f(1.f), Math::toRadians(90.f),
					Vector3f(1.f, 0.f, 0.f)) * sbi.transform;
		}

		if (Application::getKeyPressed(Input::KEY_F)) {
			sbi.objectType = (enum BlockInfo::BlockType)(((int32)sbi.objectType + 1)
					% BlockInfo::NUM_TYPES);
		}
	});
}

static void rayShipIntersection(const IndexedModel& cube, const Matrix4f& shipTransform,
		const Ship& ship, const Vector3f& origin, const Vector3f& direction, uint32* index,
		Vector3f* hitPosition, Vector3f* hitNormal) {
	const Vector4f tfOrigin(origin, 1.f);
	const Vector4f tfDirection(direction, 0.f);

	Vector3f intersectPos, intersectNormal;
	float minDist = FLT_MAX;
	
	*index = (uint32)-1;

	for (uint32 i = 0; i < ship.blocks.size(); ++i) {
		const Matrix4f tf = shipTransform * ship.blocks[i].offset;
		const Matrix4f tfi = Math::inverse(tf);

		if (cube.intersectsRay(Vector3f(tfi * tfOrigin), Vector3f(tfi * tfDirection),
				&intersectPos, &intersectNormal)) {
			intersectPos = Vector3f(tf * Vector4f(intersectPos, 1.f));
			const float dist = Math::length(origin - intersectPos);

			if (dist < minDist) {
				minDist = dist;

				*index = i;
				*hitPosition = intersectPos;
				*hitNormal = Vector3f(ship.blocks[i].offset * Vector4f(intersectNormal, 0.f));
			}
		}
	}
}


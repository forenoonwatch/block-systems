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

void ShipBuildSystem::operator()(Game& game, float deltaTime) { 
	if (Application::getMousePressed(Input::MOUSE_BUTTON_LEFT)) {
	//if (Application::isMouseDown(Input::MOUSE_BUTTON_LEFT)) {
		const CameraComponent& cc =
			game.getECS().get<CameraComponent>(cameraInfo);

		Block* block;
		Vector3f mousePos;
		Vector3f hitNormal;

		game.getECS().view<TransformComponent, Ship, ShipBuildInfo>().each([&](
				TransformComponent& transform, Ship& ship,
				ShipBuildInfo& sbi) {
			rayShipIntersection(transform.transform.toMatrix(), ship,
					cc.position, cc.rayDirection, block, &mousePos,
					&hitNormal);

			if (block != nullptr) {
				if (Application::isKeyDown(Input::KEY_LEFT_SHIFT)) {
					ship.removeBlock(*block);
				}
				else {
					ship.addBlock(sbi.objectType,
							block->position + Vector3i(hitNormal),
							sbi.rotation);
				}
			}
		});
	}
}

UpdateBuildToolTip::UpdateBuildToolTip(Game& game, ECS::Entity cameraInfo)
		: toolTip(game.getECS().create())
		, cameraInfo(cameraInfo) {
	game.getECS().assign<TransformComponent>(toolTip, Transform());
	game.getECS().assign<RenderableMesh>(toolTip, nullptr, nullptr, false);
}

void UpdateBuildToolTip::operator()(Game& game, float deltaTime) {
	const CameraComponent& cc = game.getECS().get<CameraComponent>(cameraInfo);

	Block* block;
	Vector3f mousePos;
	Vector3f hitNormal;

	TransformComponent& tf = game.getECS().get<TransformComponent>(toolTip);
	RenderableMesh& rm = game.getECS().get<RenderableMesh>(toolTip);

	game.getECS().view<ShipBuildInfo, TransformComponent, Ship>().each([&](
			ShipBuildInfo& sbi, TransformComponent& transform, Ship& ship) {
		rayShipIntersection(transform.transform.toMatrix(), ship, cc.position,
					cc.rayDirection, block, &mousePos, &hitNormal);

		rm.render = block != nullptr;

		if (rm.render) {
			Vector3f pos = Vector3f(block->position) + hitNormal;

			Matrix4f mat = transform.transform.toMatrix()
					* (Math::translate(Matrix4f(1.f), pos)
					* Math::quatToMat4(sbi.rotation));
			
			tf.transform.setPosition(Vector3f(mat[3]));
			tf.transform.setRotation(Math::mat4ToQuat(mat));
		}

		rm.vertexArray = sbi.blockArrays[sbi.objectType].get();
		rm.material = BlockInfo::getInfo(sbi.objectType).material;
	});
}

void updateShipBuildInfo(Game& game, float deltaTime) {
	game.getECS().view<ShipBuildInfo>().each([&](ShipBuildInfo& sbi) {
		if (Application::getKeyPressed(Input::KEY_R)) {
			sbi.rotation = Math::rotate(sbi.rotation, Math::toRadians(90.f),
					Vector3f(1.f, 0.f, 0.f));
		}

		if (Application::getKeyPressed(Input::KEY_T)) {
			sbi.rotation = Math::rotate(sbi.rotation, Math::toRadians(90.f),
					Vector3f(0.f, 1.f, 0.f));
		}

		if (Application::getKeyPressed(Input::KEY_F)) {
			sbi.objectType = (enum BlockInfo::BlockType)(
					((int32)sbi.objectType + 1) % BlockInfo::NUM_TYPES);
		}
	});
}


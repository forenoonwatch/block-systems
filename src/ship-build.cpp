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
#include <engine/math/aabb.hpp>

#include <cfloat>

static void rayShipIntersection(const Matrix4f& shipTransform, const Ship& ship,
		const Vector3f& origin, const Vector3f& direction, Block*& block,
		Vector3f* hitPosition, Vector3f* hitNormal);

//static void addBlockToShip(Ship& ship);
static void removeBlockFromShip(Ship& ship, Block* block);

void ShipBuildSystem::operator()(Game& game, float deltaTime) { 
	if (Application::getMousePressed(Input::MOUSE_BUTTON_LEFT)) {
		const CameraComponent& cc = game.getECS().get<CameraComponent>(cameraInfo);

		Block* block;
		Vector3f mousePos;
		Vector3f hitNormal;

		game.getECS().view<TransformComponent, Ship, ShipBuildInfo>().each([&](
				TransformComponent& transform, Ship& ship, ShipBuildInfo& sbi) {
			rayShipIntersection(transform.transform, ship, cc.position,
					cc.rayDirection, block, &mousePos, &hitNormal);

			if (block != nullptr) {
				if (Application::isKeyDown(Input::KEY_LEFT_SHIFT)) {
					removeBlockFromShip(ship, block);
				}
				else {
					/*Vector3f pos(ship.blocks[index].offset[3]);
					pos += hitNormal * BlockInfo::OFFSET_SCALE;

					Block block;
					block.type = sbi.objectType;
					block.offset = Math::translate(Matrix4f(1.f), pos)
							* sbi.transform;
					ship.blocks.push_back(block);*/
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
	/*const CameraComponent& cc = game.getECS().get<CameraComponent>(cameraInfo);
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
	});*/
}

void updateShipBuildInfo(Game& game, float deltaTime) {
	/*game.getECS().view<ShipBuildInfo>().each([&](ShipBuildInfo& sbi) {
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
	});*/
}

static void rayShipIntersection(const Matrix4f& shipTransform, const Ship& ship,
		const Vector3f& origin, const Vector3f& direction, Block*& block,
		Vector3f* hitPosition, Vector3f* hitNormal) {
	const Vector4f tfOrigin(origin, 1.f);
	const Vector4f tfDirection(direction, 0.f);

	Vector3f intersectPos, intersectNormal;
	float minDist = FLT_MAX;
	
	block = nullptr;

	const AABB aabb(Vector3f(-1.f, -1.f, -1.f), Vector3f(1.f, 1.f, 1.f));
	float p1, p2;

	for (auto& pair : ship.blocks) {
		const Matrix4f& offset = const_cast<Ship&>(ship)
				.offsets[pair.second.type][pair.second.renderIndex];
		const Matrix4f tf = shipTransform * offset;
		const Matrix4f tfi = Math::inverse(tf);

		if (aabb.intersectRay(Vector3f(tfi * tfOrigin), Vector3f(tfi * tfDirection),
				p1, p2)) {
			intersectPos = Vector3f(tfi * tfOrigin) + Vector3f(tfi * tfDirection) * p1;
			intersectPos = Vector3f(tf * Vector4f(intersectPos, 1.f));
			// TODO: calculate intersect normal
			const float dist = Math::length(origin - intersectPos);

			if (dist < minDist) {
				minDist = dist;

				block = const_cast<Block*>(&pair.second);
				*hitPosition = intersectPos;
				*hitNormal = Vector3f(offset * Vector4f(intersectNormal, 0.f));
			}
		}
	}
}

inline static void removeBlockFromShip(Ship& ship, Block* block) {
	Block* back = ship.offsetIndices[block->type].back();

	back->renderIndex = block->renderIndex;
	
	ship.offsets[block->type][block->renderIndex] = ship.offsets[block->type].back();
	ship.offsetIndices[block->type][block->renderIndex] = back;

	ship.offsets[block->type].pop_back();
	ship.offsetIndices[block->type].pop_back();

	const_cast<VertexArray*>(BlockInfo::getInfo(block->type).vertexArray)->updateBuffer(4,
				&ship.offsets[block->type][0], ship.offsets[block->type].size()
				* sizeof(Matrix4f));
}


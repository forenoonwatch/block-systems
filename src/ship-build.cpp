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

static void rayShipIntersection(const Matrix4f& shipTransform,
		const Ship& ship, const Vector3f& origin, const Vector3f& direction,
		Block*& block, Vector3f* hitPosition, Vector3f* hitNormal);

static void addBlockToShip(Ship& ship, enum BlockInfo::BlockType type,
		const Vector3i& position, const Vector3i& rotation);
static void removeBlockFromShip(Ship& ship, Block* block);

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
			rayShipIntersection(transform.transform, ship, cc.position,
					cc.rayDirection, block, &mousePos, &hitNormal);

			if (block != nullptr) {
				if (Application::isKeyDown(Input::KEY_LEFT_SHIFT)) {
					removeBlockFromShip(ship, block);
				}
				else {
					addBlockToShip(ship, sbi.objectType,
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
	game.getECS().assign<TransformComponent>(toolTip, Matrix4f(1.f));
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
		rayShipIntersection(transform.transform, ship, cc.position,
					cc.rayDirection, block, &mousePos, &hitNormal);

		rm.render = block != nullptr;

		if (rm.render) {
			Vector3f pos = Vector3f(block->position) + hitNormal;

			Matrix4f rot = Math::rotate(Matrix4f(1.f),
					Math::toRadians(90.f * sbi.rotation.x),
					Vector3f(1.f, 0.f, 0.f));
			rot = Math::rotate(rot, Math::toRadians(90.f
					* sbi.rotation.y), Vector3f(0.f, 1.f, 0.f));

			tf.transform = transform.transform
					* (Math::translate(Matrix4f(1.f), pos) * rot);
		}

		rm.vertexArray = sbi.blockInfo[sbi.objectType].vertexArray.get();
		rm.material = sbi.blockInfo[sbi.objectType].material;
	});
}

void updateShipBuildInfo(Game& game, float deltaTime) {
	game.getECS().view<ShipBuildInfo>().each([&](ShipBuildInfo& sbi) {
		if (Application::getKeyPressed(Input::KEY_R)) {
			sbi.rotation += Vector3i(0, 1, 0);
		}

		if (Application::getKeyPressed(Input::KEY_T)) {
			sbi.rotation += Vector3i(1, 0, 0);
		}

		if (Application::getKeyPressed(Input::KEY_F)) {
			sbi.objectType = (enum BlockInfo::BlockType)(
					((int32)sbi.objectType + 1) % BlockInfo::NUM_TYPES);
		}
	});
}

/*static void rayShipIntersection(const Matrix4f& shipTransform,
		const Ship& ship, const Vector3f& origin, const Vector3f& direction,
		Block*& block, Vector3f* hitPosition, Vector3f* hitNormal) {
	const Vector4f tfOrigin(origin, 1.f);
	const Vector4f tfDirection(direction, 0.f);

	Vector3f intersectPos, intersectNormal;
	float minDist = FLT_MAX;
	
	block = nullptr;

	float p1, p2;

	const Vector3f start(Math::inverse(shipTransform) * tfOrigin);
	const Vector3f dir(Math::inverse(shipTransform) * tfDirection);

	for (auto& pair : ship.blocks) {
		const Matrix4f& offset = const_cast<Ship&>(ship)
				.offsets[pair.second.type][pair.second.renderIndex];
		const Vector3f p(pair.first);
		const AABB aabb(p - Vector3f(0.5f, 0.5f, 0.5f),
				p + Vector3f(0.5f, 0.5f, 0.5f));

		if (aabb.intersectRay(start, dir, p1, p2)) {
			intersectPos = Vector3f(shipTransform
					* Vector4f(start + dir * p1, 1.f));
			intersectNormal = Vector3f(0.f, 0.f, 1.f);
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
}*/

static void rayShipIntersection(const Matrix4f& shipTransform,
		const Ship& ship, const Vector3f& origin, const Vector3f& direction,
		Block*& block, Vector3f* hitPosition, Vector3f* hitNormal) {
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

inline static void addBlockToShip(Ship& ship, enum BlockInfo::BlockType type,
		const Vector3i& position, const Vector3i& rotation) {
	Block block;
	block.type = type;
	block.position = position;
	block.rotation = rotation;
	block.renderIndex = ship.offsets[block.type].size();

	ship.blocks[position] = block;
	ship.hitTree.addObject(position);

	Matrix4f rot = Math::rotate(Matrix4f(1.f),
			Math::toRadians(90.f * rotation.x),
			Vector3f(1.f, 0.f, 0.f));
	rot = Math::rotate(rot, Math::toRadians(90.f
			* rotation.y), Vector3f(0.f, 1.f, 0.f));
	
	ship.offsets[block.type].push_back(Math::translate(Matrix4f(1.f),
			Vector3f(position)) * rot);
	ship.offsetIndices[block.type].push_back(&ship.blocks[position]);

	ship.blockInfo[block.type].vertexArray->updateBuffer(4,
			&ship.offsets[block.type][0], ship.offsets[block.type].size()
			* sizeof(Matrix4f));
}

inline static void removeBlockFromShip(Ship& ship, Block* block) {
	Block* back = ship.offsetIndices[block->type].back();

	back->renderIndex = block->renderIndex;
	
	ship.offsets[block->type][block->renderIndex] =
			ship.offsets[block->type].back();
	ship.offsetIndices[block->type][block->renderIndex] = back;

	ship.offsets[block->type].pop_back();
	ship.offsetIndices[block->type].pop_back();

	ship.blocks.erase(block->position);
	ship.hitTree.removeObject(block->position);

	ship.blockInfo[block->type].vertexArray->updateBuffer(4,
			&ship.offsets[block->type][0], ship.offsets[block->type].size()
			* sizeof(Matrix4f));
}


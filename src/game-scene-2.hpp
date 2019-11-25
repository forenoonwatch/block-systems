#pragma once

#include <engine/game/scene.hpp>

namespace Physics {
	class PhysicsEngine;
	class SphereCollider;
	class ConvexCollider;
};

class GameScene2 : public Scene {
	public:
		GameScene2();

		virtual void load(Game& game) override;
		virtual void unload(Game& game) override;

		virtual ~GameScene2();
	private:
		Physics::PhysicsEngine* physicsEngine;
		
		Physics::SphereCollider* sphereCollider;
		Physics::SphereCollider* sphereCollider2;
		Physics::ConvexCollider* convexCollider;
};


#pragma once

#include <engine/ecs/ecs-system.hpp>

class RenderMesh : public ECS::System {
	public:
		virtual void operator()(Game&, float) override;
};

class RenderSkybox : public ECS::System {
	public:
		virtual void operator()(Game&, float) override;
};

class ToggleFullscreenSystem : public ECS::System {
	public:
		virtual void operator()(Game&, float) override;
};


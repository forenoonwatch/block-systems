#pragma once

#include <engine/scene/scene.hpp>

class TempScene final : public Scene {
	public:
		virtual void load() override;

		virtual void update(float deltaTime) override;
		virtual void render() override;

		virtual void unload() override;
	private:
};

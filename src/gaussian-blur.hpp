#pragma once

#include <engine/rendering/shader.hpp>

class GaussianBlur {
	public:
		GaussianBlur(RenderContext& context, Shader& blurShader,
				Texture& blurTarget);

		void update();

		inline Texture& getBuffer0() { return blurTarget; }
		inline Texture& getBuffer1() { return buffer; }
	private:
		RenderContext* context;

		Shader& blurShader;
		Texture& blurTarget;

		Texture buffer;

		bool horizontal;
};

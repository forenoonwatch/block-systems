#pragma once

#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/texture.hpp>

struct RenderableMesh {
	VertexArray* vertexArray;
	Texture* texture;
};


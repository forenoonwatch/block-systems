#pragma once

#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/material.hpp>

struct RenderableMesh {
	VertexArray* vertexArray;
	Material* material;
	bool render;
};


#include "ocean.hpp"

#include "ocean-projector.hpp"

#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/uniform-buffer.hpp>

#include <engine/ecs/ecs.hpp>

#define OCEAN_BUFFER_SIZE 4 * sizeof(Vector4f) + 3 * sizeof(float)

void initOcean(RenderContext& context, Ocean& ocean, uint32 gridLength) {
	ocean.gridLength = gridLength;

	IndexedModel grid;

	const float fGridLength = (float)gridLength;
	const float ifGridLength = 1.f / (fGridLength - 1.f);

	grid.allocateElement(2); // vec2 position
	//grid.allocateElement(16); // mat4 transform

	//grid.setInstancedElementStartIndex(1);

	for (float y = 0; y < fGridLength; ++y) {
		for (float x = 0; x < fGridLength; ++x) {
			grid.addElement2f(0, x / (fGridLength - 1.f),
					y / (fGridLength - 1.f));
		}
	}

	for (int32 y = 1; y < gridLength; ++y) {
		for (int32 x = 1; x < gridLength; ++x) {
			int32 i0 = y * gridLength + x;
			int32 i1 = (y - 1) * gridLength + x;
			int32 i2 = y * gridLength + x - 1;
			int32 i3 = (y - 1) * gridLength + x - 1;

			grid.addIndices3i(i0, i1, i2);
			grid.addIndices3i(i1, i3, i2);
		}
	}

	ocean.gridArray = Memory::make_shared<VertexArray>(context, grid,
			GL_STATIC_DRAW);
	ocean.oceanDataBuffer = Memory::make_shared<UniformBuffer>(context,
			OCEAN_BUFFER_SIZE, GL_STREAM_DRAW, 2);
}

void updateOceanBuffer(float deltaTime) { 
	ECS::Registry::getInstance().view<Ocean, OceanProjector>().each([&](
			Ocean& ocean, OceanProjector& op) {
		ocean.oceanFFT->update(deltaTime);
		ocean.oceanDataBuffer->update(op.corners, 4 * sizeof(Vector4f));
	});
}


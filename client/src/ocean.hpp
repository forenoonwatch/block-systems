#pragma once

#include "ocean-fft.hpp"

#include <engine/core/memory.hpp>

#include <engine/math/vector.hpp>

class VertexArray;
class UniformBuffer;
class RenderContext;

struct Ocean {
	static constexpr const float LINEAR_DRAG = 5.f;
	static constexpr const float ANGULAR_DRAG = 0.5f;
	static constexpr const float DENSITY = 1.f;
	static constexpr const Vector3f VELOCITY = Vector3f(0.f, 0.f, 0.f);
	
	static constexpr const Vector3f POSITION = Vector3f(0.f, 0.f, 0.f);
	static constexpr const Vector3f NORMAL = Vector3f(0.f, 1.f, 0.f);
	
	static constexpr const float MAX_AMPLITUDE = 4.f;

	Memory::SharedPointer<OceanFFT> oceanFFT;
	Memory::SharedPointer<VertexArray> gridArray;
	Memory::SharedPointer<UniformBuffer> oceanDataBuffer;
	uint32 gridLength;
};

void initOcean(RenderContext& context, Ocean& ocean, uint32 gridLength);

void updateOceanBuffer(float deltaTime);


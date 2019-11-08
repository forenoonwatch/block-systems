#include "block.hpp"

#include <engine/rendering/indexed-model.hpp>
#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/material.hpp>

#include <engine/rendering/render-context.hpp>

ArrayList<BlockInfo> BlockInfo::blockInfo;

void BlockInfo::registerType(uint32 type, uint32 flags,
		IndexedModel* model, Material* material, float mass, float volume) {
	blockInfo.push_back(BlockInfo(type, flags, model, material, mass, volume));
}

void BlockInfo::initVertexArrays(RenderContext& renderContext,
		ArrayList<Memory::SharedPointer<VertexArray>>& vaos) {
	for (auto& info : blockInfo) {
		vaos.emplace_back(new VertexArray(renderContext, *info.model,
				GL_STATIC_DRAW));
	}
}


#include "block.hpp"

ArrayList<BlockInfo> BlockInfo::blockInfo;

void BlockInfo::registerType(enum BlockType type, uint32 flags, IndexedModel& model,
		VertexArray& vertexArray, Material& material) {
	blockInfo.push_back(BlockInfo(type, flags, &model, &vertexArray, &material));
}


#include "block.hpp"

ArrayList<BlockInfo> BlockInfo::blockInfo;

void BlockInfo::registerType(enum BlockType type, IndexedModel& model,
		VertexArray& vertexArray, Material& material) {
	blockInfo.push_back(BlockInfo(type, &model, &vertexArray, &material));
}


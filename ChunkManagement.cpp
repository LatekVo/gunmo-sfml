//
// Created by armado on 9/8/22.
//

#include "EngineEssentials.h"


void Chunk::swapObjectToActive(std::shared_ptr<GameObject> &obj, Environment &env) {
	env.data_activeObjects.push_back(obj);
	data_passiveObjects.remove(obj);
}

void Chunk::swapObjectToPassive(std::shared_ptr<GameObject> &obj, Environment &env) {
	data_passiveObjects.push_back(obj);
	env.data_activeObjects.remove(obj);
}

e_biome ChunkManager::_perlinToBiome(float x, float y) {
	auto polar = chunkPerlin.getPolarization3d(x, y);
	return (e_biome)(polar.first + polar.second + polar.second);
}

Chunk ChunkManager::_generate(int x, int y) {
	Chunk newChunk;
	int chunkSize = newChunk.tileMap.size();
	float globalCoords[] {(float)(x * chunkSize), (float)(y * chunkSize)};

	for (unsigned int x_i = 0; x_i < chunkSize; x_i++) {
		for (unsigned int y_i = 0; y_i < chunkSize; y_i++) {
			Tile newTile;

			//newTile.biomeVertex = {}; // i need 2 persistent 0->1 values here

			// noise for:   chunk position  +  tile offset  +  tile offset offset
			newTile.biomeName = _perlinToBiome(globalCoords[0] + (float)x_i + newTile.biomeVertex[0], globalCoords[1] + (float)y_i + newTile.biomeVertex[1]);
			newChunk.tileMap[x][y] = newTile;
		}
	}

	return newChunk;
}
// looking for chunk to load sequence:  historyBuffer -> activeChunkBuffer -> savedOnFile -> generateNew
std::shared_ptr<Chunk> ChunkManager::_getChunk(int x, int y, Environment &ctx) {

	// check history buffer x and y
	// double nested if - to ensure right order of operation
	auto historyBufferCheck_x = historyChunkBuffer.find(x);
	if (historyBufferCheck_x != historyChunkBuffer.end()) {
		auto historyBufferCheck_y = historyBufferCheck_x->second.find(y);
		if (historyBufferCheck_y != historyBufferCheck_x->second.end()) {
			return historyBufferCheck_y->second;
		}
	}

	// active is a list, so use std::find();

	return std::make_shared<Chunk>(_generate(x, y));

}

ChunkManager::ChunkManager(Environment &ctx) {
	chunkPerlin = LayeredPerlin(2, ctx.s_mapSeed);
}

void ChunkManager::update() {

}

void ChunkManager::draw() {

}
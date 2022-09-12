//
// Created by armado on 9/12/22.
//

#ifndef GUNMO_CHUNKMANAGEMENT_H
#define GUNMO_CHUNKMANAGEMENT_H

#include "EngineEssentials.h"


// Tile is the smallest building unit, and the smallest biome detail unit, 1 tile has 1 randomly placed biome vertex
// portions of tile are re-generated every time it is loaded, the ones that cannot be altered by user

// For the dynamic chunk loading to work, all of the objects will have to be stored in the chunk data somehow.
// When objects turn active, they are stored in the active register, independently of chunks.
// When they turn dossile (have no job) they turn to the chunk storage.
// Active objects are always globally loaded, and always force a chunk they are in to load as well.

struct Tile {
	float biomeVertex[2] = {0.5f, 0.5f};
	unsigned int sameBiomeFaces = 0; // utility value, not neccesary
	e_biome biomeName = e_biome::TBD;
};

// chunk is a screen sized lump of Tiles, not necessary just makes Tiles easier to manage and load, and increases float precision (or it will at some point)
class Chunk {
public:
	std::array<std::array<Tile, 16>, 16> tileMap;
	// using data_x format for consistency with an Env object named the same way
	std::list<std::shared_ptr<GameObject>> data_passiveObjects;
	bool isActive = false;
	bool isModified = false;

	// these functions may be moved in future to the GameObject class
	void swapObjectToActive(std::shared_ptr<GameObject> &obj, Environment &env);
	void swapObjectToPassive(std::shared_ptr<GameObject> &obj, Environment &env);
};


// loading, generating, storing chunks
class ChunkManager {
private:
	LayeredPerlin chunkPerlin;
	std::weak_ptr<Environment> ctx;

	//storing all chunks as pointers to move them around instead of copying them over and over again.

	// 3x3 array for what player sees
	std::deque<std::deque<std::shared_ptr<Chunk>>> playerBuffer; // pretty much activeChunkBuffer but drawn on screen.
	// ram buffer to avoid generating or loading chunks from disc,
	// will have to find a way to manage this 2d map's memory usage.
	std::map<int, std::map<int, std::shared_ptr<Chunk>>> historyChunkBuffer;

	// why there are two different containers for one thing:
	// we need a simple, parsable container, with good remove() functionality, list offers just that, you can instantly remove by reference.
	// meanwhile, we still need a coord map to quickly quarry for chunks, especially important for _load() function
	std::list<std::shared_ptr<std::shared_ptr<Chunk>>> activeChunkBuffer;
	std::map<int, std::map<int, std::shared_ptr<std::weak_ptr<Chunk>>>> activeChunkBuffer_coordMap;


	e_biome _perlinToBiome(float x, float y);

	Chunk _generate(int x, int y);

	// looking for chunk to load sequence:  historyBuffer -> activeChunkBuffer -> savedOnFile -> generateNew
	std::shared_ptr<Chunk> _getChunk(int x, int y);

public:
	explicit ChunkManager(std::shared_ptr<Environment> &_ctx);

	void update();
	void draw();
};


#endif //GUNMO_CHUNKMANAGEMENT_H

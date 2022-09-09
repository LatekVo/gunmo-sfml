//
// Created by armado on 9/9/22.
//

#ifndef GUNMO_LIBDRAW_H
#define GUNMO_LIBDRAW_H

#include <memory>

#include "EngineEssentials.h"

class LibDraw {
public:
	std::shared_ptr<Environment> ctx_environment;
	std::shared_ptr<ChunkManager> ctx_chunkManager;

	void attach(Environment &ctx_environment, ChunkManager &ctx_chunkManager);

	LibDraw() = default;
	LibDraw(Environment &ctx_environment, ChunkManager &ctx_chunkManager);

};


#endif //GUNMO_LIBDRAW_H

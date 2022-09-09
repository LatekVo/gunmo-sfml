//
// Created by armado on 9/9/22.
//

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "LibDraw.h"

void LibDraw::attach(Environment &_ctx_environment, ChunkManager &_ctx_chunkManager) {
	ctx_environment = std::make_shared<Environment>(_ctx_environment);
	ctx_chunkManager = std::make_shared<ChunkManager>(_ctx_chunkManager);
}

LibDraw::LibDraw(Environment &_ctx_environment, ChunkManager &_ctx_chunkManager) {
	attach(_ctx_environment, _ctx_chunkManager);
}
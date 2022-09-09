//
// Created by armado on 9/9/22.
//

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "LibDraw.h"

void LibDraw::attach(std::weak_ptr<sf::RenderWindow> &_ctx_window, std::weak_ptr<Environment> &_ctx_environment, std::weak_ptr<ChunkManager> &_ctx_chunkManager) {
	// base initializers (tried them in header) don't seem to work. : ctx_window(_ctx_window), ctx_environment(_ctx_environment), ctx_chunkManager(_ctx_chunkManager)
	ctx_window = _ctx_window;
	ctx_environment =_ctx_environment;
	ctx_chunkManager = _ctx_chunkManager;
}

LibDraw::LibDraw(std::weak_ptr<sf::RenderWindow> &_ctx_window, std::weak_ptr<Environment> &_ctx_environment, std::weak_ptr<ChunkManager> &_ctx_chunkManager) {
	attach(_ctx_window, _ctx_environment, _ctx_chunkManager);
}
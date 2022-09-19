//
// Created by armado on 9/9/22.
//

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "LibDraw.h"

void LibDraw::attach(std::shared_ptr<sf::RenderWindow> &_ctx_window, std::shared_ptr<Environment> &_ctx_environment, std::shared_ptr<ChunkManager> &_ctx_chunkManager) {
	// base initializers (tried them in header) don't seem to work. : ctx_window(_ctx_window), ctx_environment(_ctx_environment), ctx_chunkManager(_ctx_chunkManager)
	ctx_window = _ctx_window;
	ctx_environment =_ctx_environment;
	ctx_chunkManager = _ctx_chunkManager;
}

LibDraw::LibDraw(std::shared_ptr<sf::RenderWindow> &_ctx_window, std::shared_ptr<Environment> &_ctx_environment, std::shared_ptr<ChunkManager> &_ctx_chunkManager) {
	attach(_ctx_window, _ctx_environment, _ctx_chunkManager);
}

void LibDraw::update() {

}
void LibDraw::draw() {
	// draw biome

	// draw GOs

}

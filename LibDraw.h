//
// Created by armado on 9/9/22.
//

#ifndef GUNMO_LIBDRAW_H
#define GUNMO_LIBDRAW_H

#include <memory>

#include "EngineEssentials.h"

class LibDraw {
public:
	std::weak_ptr<sf::RenderWindow> ctx_window;
	std::weak_ptr<Environment> ctx_environment;
	std::weak_ptr<ChunkManager> ctx_chunkManager;

	void attach(std::weak_ptr<sf::RenderWindow> &_ctx_window, std::weak_ptr<Environment> &_ctx_environment, std::weak_ptr<ChunkManager> &_ctx_chunkManager);

	LibDraw() = default;
	LibDraw(std::weak_ptr<sf::RenderWindow> &_ctx_window, std::weak_ptr<Environment> &_ctx_environment, std::weak_ptr<ChunkManager> &_ctx_chunkManager);

};


#endif //GUNMO_LIBDRAW_H

//
// Created by armado on 9/9/22.
//

#ifndef GUNMO_LIBDRAW_H
#define GUNMO_LIBDRAW_H

#include <memory>

#include "EngineEssentials.h"
#include "ChunkManagement.h"

// update rate should be an-par with that of the Environment, though drawing rate can be different (lower)
class LibDraw {
	// insertion sort
	void _sort();
public:
	std::weak_ptr<sf::RenderWindow> ctx_window;
	std::weak_ptr<Environment> ctx_environment;
	std::weak_ptr<ChunkManager> ctx_chunkManager;

	void attach(std::shared_ptr<sf::RenderWindow> &_ctx_window, std::shared_ptr<Environment> &_ctx_environment, std::shared_ptr<ChunkManager> &_ctx_chunkManager);

	LibDraw() = default;
	LibDraw(std::shared_ptr<sf::RenderWindow> &_ctx_window, std::shared_ptr<Environment> &_ctx_environment, std::shared_ptr<ChunkManager> &_ctx_chunkManager);

	// this should be done RIGHT AFTER Environment update
	void update();

	// this can be done whenever it's needed, and on a separate thread (not yet implemented)
	void draw();

};


#endif //GUNMO_LIBDRAW_H

//
// Created by armado on 9/8/22.
//

#include "EngineEssentials.h"

void Environment::addActiveObject(const std::shared_ptr<GameObject>& ptr_newGameObject) {
	data_activeObjects.push_back(ptr_newGameObject);
}
void Environment::removeActiveObject(const std::shared_ptr<GameObject>& ptr_newGameObject) {
	data_activeObjects.remove(ptr_newGameObject);
}

void Environment::newFrameAdjustment() {
	// new frame's delta time counter
	g_currentFrameAdjustment = g_delta.getElapsedTime().asSeconds();
	g_delta.restart();

	// all additional context-dependant variables
	v_mouseWheelTicks = 0.f;
}
float Environment::getFrameAdjustment() const {
	return g_timeScale * g_currentFrameAdjustment * 1000;
}

void Environment::updateGameState() {
	/* List of actions:
		call update on every active object
		update camera's position if needed (pan)
	*/
	for (auto &each_object : data_activeObjects) {

		// Should GOs really update themselves? Or should Env take on this responsibility?
		each_object->update(*this);
	}
}
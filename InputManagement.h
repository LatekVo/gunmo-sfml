//
// Created by armado on 9/17/22.
//

#ifndef GUNMO_INPUTMANAGEMENT_H
#define GUNMO_INPUTMANAGEMENT_H

#include "EngineEssentials.h"
#include "Constants.h"

class Input {
private:

	void _keyboard();
	void _mouse();

public:

	enum class InputType {
		KEYBOARD = 1,
		MOUSE = 2,
	};

	// even 128 should be enough
	bool keyboard[256] = {false};
	float mouse[2] = {0., 0.};

	void update(unsigned char inputTypeMask);

};


#endif //GUNMO_INPUTMANAGEMENT_H

//
// Created by armado on 9/5/22.
//

#ifndef GUNMO_LAYEREDPERLIN_H
#define GUNMO_LAYEREDPERLIN_H

#include "PerlinNoise.h"

class LayeredPerlin {
	unsigned int l_seed = 0;
	unsigned int l_count = 1;

public:
	PerlinNoise noise;

	// no need for scaling value, 1/x works fine 90% of the time, 1/2^x is unnecesarly complex,
	// and anything larger than 1/2 will result in spiky and usually unneeded noisemaps

	LayeredPerlin(unsigned int layers = 1, unsigned int seed = 0);

	float getNoise(float x, float y);

	// noise is 3d, we can measure displacement's direction by subtracting noise at one level from noise at level above.
	// if this will be incredibly inefficient, i'll have to modify the pn algo at core to return the very same information.

	// NOTE: not mentioned anywhere, but the first value is a layered noise and the other one is not.
	std::pair<bool, bool> getPolarization3d(float x, float y);
};


#endif //GUNMO_LAYEREDPERLIN_H

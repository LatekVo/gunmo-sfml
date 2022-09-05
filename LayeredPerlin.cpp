//
// Created by armado on 9/5/22.
//

#include <stdexcept>
#include "LayeredPerlin.h"

LayeredPerlin::LayeredPerlin(unsigned int layers, unsigned int seed) : l_seed(seed), l_count(layers) {
	if (layers < 1) throw std::invalid_argument("LayeredPerlin: template argument 'L' - number of layers must be positive");
		noise.setSeed(l_seed);

}

float LayeredPerlin::getNoise(float x, float y) {
	float o = 0;

	for (float i = 1; i <= l_count; i += 1.f) { // NOLINT(cert-flp30-c,cppcoreguidelines-narrowing-conversions)
		o += (float)noise.noise(x / i, y / i, i) / i;
	}

	return o;
}

// noise is 3d, we can measure displacement's direction by subtracting noise at one level from noise at level above.
// if this will be incredibly inefficient, i'll have to modify the pn algo at core to return the very same information.

// NOTE: not mentioned anywhere, but the first returned value is a layered noise and the other one is the polarization bool.
std::pair<bool, bool> LayeredPerlin::getPolarization3d(float x, float y) {
	float base2dNoise = 0;
	float diff = 0;

	// we only want to look at the biggest layer's displacemnt, including smaller layers will create tiny anomalies
	base2dNoise = (float)noise.noise(x, y, 0);
	diff = (float)noise.noise(x, y, 0.1f); // TODO: this value (0.1f) will be broken with big enough x and y, change do adaptive value

	// from layer 2 proceed as usual
	for (float i = 2; i <= l_count; i += 1.f) { // NOLINT(cert-flp30-c,cppcoreguidelines-narrowing-conversions)
		base2dNoise += (float)noise.noise(x / i, y / i, i) / i;
	}

	// case 0 is practically impossible even when the math plays out because of float error
	return {base2dNoise > 0, diff > base2dNoise};
}
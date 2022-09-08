//
// Created by armado on 9/7/22.
//

#ifndef GUNMO_CONSTANTS_H
#define GUNMO_CONSTANTS_H


const float PI = 3.14159;

// dictates logic attached to moving, should probably rather be controlled by anon functions and not some hardcoded switch statements
enum class e_targetingType {
	NONE = 0, // no movement even in case of speed > 0
	CUSTOM,
	SIMPLE, // simple head-on

	// PROJECTILE SHOOTING ALGO
	LEADING_LINEAR, // leading, assuming straight-line movement of the target
	LEADING_QUADRATIC, // includes acceleration changes of the target

	// Movements below are prefabs/templates and will probably be replaced by "CUSTOM" type.
	// STAYING NEAR
	MOVE_LINGER, // random direction in a vicinity
	MOVE_ORBIT, // orbit in a vicinity
	MOVE_HIT_AND_RUN, // speed past the object (distance is set)
};

// dictates rotation logic
enum class e_rotationType {
	NONE = 0, // no ratation even in case of speed > 0
	CUSTOM, // set this to use a custom functor function for rotating

	SIMPLE_TARGET, // instantly rotate to target
	SIMPLE_VELOCITY, // instant deducted from the movement direction

	SMOOTH_TARGET,
	SMOOTH_VELOCITY,

};

enum class e_biome {
	FOREST,
	PLAINS,
	SAVANNA,
	DESERT,

	COUNT,
	TBD,
};

#endif //GUNMO_CONSTANTS_H

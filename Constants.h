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

// every supported key. keys themselves don't matter,
// only thing that matters is KEY - VALUE relation to remain unchanged between all versions of this library
enum class e_key {
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	Y,
	Z,

	N_ONE,
	N_TWO,
	N_THREE,
	N_FOUR,
	N_FIVE,
	N_SIX,
	N_SEVEN,
	N_EIGHT,
	N_NINE,
	N_ZERO,

	Alt,
	Alt_L,
	Alt_R,

	Shift,
	Shift_L,
	Shift_R,

	S_Esc,
	S_Brackets_L,
	S_Brackets_R,
	S_Curly_L,
	S_Curly_R,
	S_Slash,
	S_BackSlash,
	S_LessThan,
	S_GreaterThan,
	S_Dot,
	S_Comma
};

#endif //GUNMO_CONSTANTS_H

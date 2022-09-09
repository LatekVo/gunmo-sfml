//
// Created by armado on 9/5/22.
//

#ifndef GUNMO_ENGINEESSENTIALS_H
#define GUNMO_ENGINEESSENTIALS_H

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Config.hpp>

#include <array>
#include <vector>
#include <list>
#include <deque>
#include <memory>

#include "LayeredPerlin.h"
#include "Constants.h"

class Environment; // environment will actually also control all the watch-dogged data and objects that need to be constantly updated
class GameObject;

// todo: to be moved to another class file
namespace Math {
	template<typename T>
	class Vec {
	public:
		T x = 0;
		T y = 0;


	};
}

// movement and rotation, may be replaced with unit vectors to target


// GameObject is universal, not only for 'real' objects, but for things like markings or bullets as well.

// Tile is the smallest building unit, and the smallest biome detail unit, 1 tile has 1 randomly placed biome vertex
// portions of tile are re-generated every time it is loaded, the ones that cannot be altered by user

// For the dynamic chunk loading to work, all of the objects will have to be stored in the chunk data somehow.
// When objects turn active, they are stored in the active register, independently of chunks.
// When they turn dossile (have no job) they turn to the chunk storage.
// Active objects are always globally loaded, and always force a chunk they are in to load as well.

struct Tile {
	float biomeVertex[2] = {0.5f, 0.5f};
	unsigned int sameBiomeFaces = 0; // utility value, not neccesary
	e_biome biomeName = e_biome::TBD;
};

// chunk is a screen sized lump of Tiles, not necessary just makes Tiles easier to manage and load, and increases float precision (or it will at some point)
class Chunk {
public:
	std::array<std::array<Tile, 16>, 16> tileMap;
	// using data_x format for consistency with an Env object named the same way
	std::list<std::shared_ptr<GameObject>> data_passiveObjects;
	bool isActive = false;
	bool isModified = false;

	// these functions may be moved in future to the GameObject class
	void swapObjectToActive(std::shared_ptr<GameObject> &obj, Environment &env);
	void swapObjectToPassive(std::shared_ptr<GameObject> &obj, Environment &env);
};

// loading, generating, storing chunks
class ChunkManager;

// save file specific detail. some variables not affiliated to save file.
class Environment {
public:
	// game info & properties
	float g_timeScale = 1.f;
	sf::Clock g_delta;
	float g_currentFrameAdjustment = 0;

	// settings
	std::string s_saveFilename = "newSaveFile";
	unsigned int s_mapSeed = 0xDEADBEEF;
	float s_mouseWheelSensitivity = 0.1f;

	// must-store, context-dependant variables
	float v_mouseWheelTicks = 0;

	// meta stuff, variables exceeding what the player's game object can do
	// these pointers are only a reference, main object should be treated regardless of these pointers (thus weak_ptr)
	std::weak_ptr<GameObject> m_objectFocus;
	std::weak_ptr<GameObject> m_objectMain;

	// main storage ---
	// objects constantly updated
	std::list<std::shared_ptr<GameObject>> data_activeObjects;
	// objects waiting for update
	std::list<std::shared_ptr<GameObject>> data_staticObjects;
	// add chunks later as well

	void addActiveObject(const std::shared_ptr<GameObject>& ptr_newGameObject);
	void removeActiveObject(const std::shared_ptr<GameObject>& ptr_newGameObject);

	void updateGameState(Environment &ctx); // passing self, this is dumb, there has to be a better way

	std::map<std::string, GameObject> data_GamePresets; // holds copyable objects,

	void newFrameAdjustment();

	float getFrameAdjustment() const;

};

// use anonymous function objects for special behaviour, like exploding after set amount of time or something
class GameObject {
private:

	// linking, for example a turret mounted on a vehicle
	// a child list could be a better solution, though it could have some problems with drawing
	std::weak_ptr<GameObject> parentObject;
	bool isChild = false; // mainly for checking whether the parent is dead, or was there in the first place

public:

	// currently, defaults to "rotating, not moving", should not be a problem if no target is set
	float movSpeed_max = 0;
	float movSpeed_accel = 0;
	float rotSpeed_max = 1; // in degrees, DONT USE RADS
	float rotSpeed_accel = 0;

	sf::Vector2f position {0,0};
	sf::Vector2f position_d {0, 0};
	float rotation = 0; // deg, still less conversion will have to be done than when using rad up-front
	float rotation_d = 0;
	e_rotationType rotatingMode = e_rotationType::NONE;

	float detectionRange = 0.; // gets aggro
	float focusRange = 0.; // hunts down
	float attackRange = 0.; // maximum hit range
	float hitBox = 0.; //

	std::weak_ptr<GameObject> primaryTarget;
	std::weak_ptr<GameObject> currentTarget; // internal actual target, eg: things in the way of target

	e_targetingType targetingMode = e_targetingType::NONE;

	void setTarget(std::shared_ptr<GameObject> &target, e_targetingType mode = e_targetingType::SIMPLE);

	void setParent(std::shared_ptr<GameObject> &arg_newParent);

	/*

	void setTexture(const std::string& texturePath);

	void draw(sf::RenderWindow& rx);

	void initView(sf::RenderWindow& rx);

	void centerView(sf::RenderWindow& rx);

	*/

	void updatePlayerInput(Environment &ctx);


	// only sets the object for targeting, may be used internally, functionality expanded in the main update function

	void linkObject(std::shared_ptr<GameObject> &parent);

	void remove(Environment &ctx, bool animation = true);

	void update(Environment &ctx);

	// starting with _ are routine, internal commands
	// those functions will have to be executed multiple times to complete a certain task.
	void _rotateToTarget(Environment &ctx);

	void _approachTarget(Environment &ctx);

	// a full routine of approaching and attacking if possible
	// i think this should be an anonymous function that is not included in the GameObject. It's too specific for such a general class.
	void _attackTarget(Environment &ctx) const;

	static int _getNewTarget(Environment &ctx);

	GameObject() = default;
};

class ChunkManager {
private:
	LayeredPerlin chunkPerlin;

	//storing all chunks as pointers to move them around instead of copying them over and over again.

	// 3x3 array for what player sees
	std::deque<std::deque<std::shared_ptr<Chunk>>> playerBuffer; // pretty much activeChunkBuffer but drawn on screen.
	// ram buffer to avoid generating or loading chunks from disc,
	// will have to find a way to manage this 2d map's memory usage.
	std::map<int, std::map<int, std::shared_ptr<Chunk>>> historyChunkBuffer;

	// why there are two different containers for one thing:
	// we need a simple, parsable container, with good remove() functionality, list offers just that, you can instantly remove by reference.
	// meanwhile, we still need a coord map to quickly quarry for chunks, especially important for _load() function
	std::list<std::shared_ptr<std::shared_ptr<Chunk>>> activeChunkBuffer;
	std::map<int, std::map<int, std::shared_ptr<std::weak_ptr<Chunk>>>> activeChunkBuffer_coordMap;


	e_biome _perlinToBiome(float x, float y);

	Chunk _generate(int x, int y);

	// looking for chunk to load sequence:  historyBuffer -> activeChunkBuffer -> savedOnFile -> generateNew
	std::shared_ptr<Chunk> _getChunk(int x, int y, Environment &ctx);

public:
	explicit ChunkManager(Environment &ctx);

	void update();
	void draw();
};

#endif //GUNMO_ENGINEESSENTIALS_H

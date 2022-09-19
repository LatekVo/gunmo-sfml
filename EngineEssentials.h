//
// Created by armado on 9/5/22.
//

#ifndef GUNMO_ENGINEESSENTIALS_H
#define GUNMO_ENGINEESSENTIALS_H

#include <array>
#include <vector>
#include <list>
#include <deque>
#include <memory>
#include <map>
#include <chrono>

#include "LayeredPerlin.h"
#include "Constants.h"

class Environment; // environment will actually also control all the watch-dogged data and objects that need to be constantly updated
class GameObject;

// todo: to be moved to another class file, manageable enough to be .hpp
namespace Math {
	template<typename T>
	class Vec {

		void _add(Vec<T> &val) {
			x += val.x;
			y += val.y;
		}
		void _sub(Vec<T> &val) {
			x -= val.x;
			y -= val.y;
		}
		void _mult(T val) {
			x *= val;
			y *= val;
		}
		void _div(T val) {
			x /= val;
			y /= val;
		}

	public:
		T x = 0;
		T y = 0;

		void operator +=(Vec<T> &rhs) {
			_add(rhs);
		}

		void operator -=(Vec<T> &rhs) {
			_sub(rhs);
		}

		void operator *=(Vec<T> &rhs) {
			_mult(rhs);
		}

		void operator /=(Vec<T> &rhs) {
			_div(rhs);
		}
	};
}

namespace Time {
	class Clock {
	public:
		void start();
		void stop();
		void restart();
	};
}

// GameObject is universal, not only for 'real' objects, but for things like markings or bullets as well.

// save file specific detail. some variables not affiliated to save file.
class Environment {
public:
	// game info & properties
	float g_timeScale = 1.f;
	Time::Clock g_delta;
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

	void updateGameState();

	std::map<std::string, GameObject> data_GamePresets; // holds copyable objects,

	void newFrameAdjustment();

	float getFrameAdjustment() const;

};

// use anonymous function objects for special behaviour, like exploding after set amount of time or something
// * use anonymous functions for everything custom, the built-in protocols are very bare-bones
// GO should still contain graphics related variables, just nothing related to SFML.
class GameObject {
private:

	// linking, for example a turret mounted on a vehicle
	// a child list could be a better solution, though it could have some problems with drawing
	std::weak_ptr<GameObject> parentObject;
	bool isChild = false; // mainly for checking whether the parent is dead, or was there in the first place

public:

	// drawing info, todo: to be moved to a json/xml file

	struct drawInfo {
		float draw_zLevel = 0.; // higher = on top
		std::string texturePath;

		void setTexture(const std::string& _texturePath);

	};



	// currently, defaults to "rotating, not moving", should not be a problem if no target is set
	float movSpeed_max = 0;
	float movSpeed_accel = 0;
	float rotSpeed_max = 1; // in degrees, DONT USE RADS
	float rotSpeed_accel = 0;

	Math::Vec<float> position {0,0};
	Math::Vec<float> position_d {0, 0};
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

	void draw(sf::RenderWindow& rx);

	void initView(sf::RenderWindow& rx);

	void centerView(sf::RenderWindow& rx);

	*/

	void updatePlayerInput(Environment &ctx, bool keyboardData[256]);

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


#endif //GUNMO_ENGINEESSENTIALS_H

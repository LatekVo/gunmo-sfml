//
// Created by armado on 9/5/22.
//

#include <memory>
#include <iostream>

#include "EngineEssentials.h"

GamePreset::GamePreset() = default;

GamePreset::GamePreset(const std::string &filePath) {

}

void GameObject::setTarget(std::shared_ptr<GameObject> &target, e_targetingType mode) {
	primaryTarget = target;
	targetingMode = mode;
}

void GameObject::setParent(std::shared_ptr<GameObject> &arg_newParent) {
	parentObject = arg_newParent; // shared to weak conversion, check later whether this alone works
	isChild = true;
}

void GameObject::setTexture(const std::string& texturePath) {
	if (texture.loadFromFile("rsc/" + texturePath) == 0) {
		std::cout << "replacing texture with a placeholder" << std::endl;
		texture.loadFromFile("rsc/placeholder.bmp");
	}
	sprite.setTexture(texture);
}

void GameObject::draw(sf::RenderWindow& rx) {
	sprite.setPosition(position);
	sprite.setRotation(rotation);
	rx.draw(sprite);
}

void GameObject::updatePlayerInput(Environment &ctx) {

	float mov = movSpeed_max * ctx.getFrameAdjustment();
	float xMov = 0;
	float yMov = 0;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
		yMov -= mov;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
		yMov += mov;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
		xMov -= mov;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
		xMov += mov;

	if (xMov != 0 && yMov != 0) {
		// 707 = sin 45deg = cos 45deg
		xMov *= .707;
		yMov *= .707;
	}

	position.x += xMov;
	position.y += yMov;
	sprite.setPosition(position);
}

void GameObject::initView(sf::RenderWindow& rx) {
	objectView.reset(sf::FloatRect(0, 0, (float) rx.getSize().x, (float) rx.getSize().y));
}

void GameObject::centerView(sf::RenderWindow& rx) {
	objectView.setCenter(position);
	objectView.move((float)sprite.getTextureRect().width / 2,
					(float)sprite.getTextureRect().height / 2);
	rx.setView(objectView);
}

// only sets the object for targeting, may be used internally, functionality expanded in the main update function

void GameObject::linkObject(std::shared_ptr<GameObject> &parent) {
	parentObject.reset();
	parentObject = parent;
}

void GameObject::remove(Environment &ctx, bool animation) {
	// ctx . data{} . remove()
}

// starting with _ are routine, internal commands
// those functions will have to be executed multiple times to complete a certain task.
void GameObject::_rotateToTarget(Environment &ctx) {
	auto lockedTarget = primaryTarget.lock();
}


void GameObject::_approachTarget(Environment &ctx) {

	auto lockedTarget = primaryTarget.lock();

	// this is temporary, change to switch statement as there are more options

	switch (targetingMode) {
		case e_targetingType::NONE:
			return;
		case e_targetingType::SIMPLE: {
			// TODO: refactor to navmesh navigation
			float mov = movSpeed_max * ctx.getFrameAdjustment();

			// don't recalculate anything, use an already existing difference
			float xMov = lockedTarget->position.x - position.x;
			float yMov = lockedTarget->position.y - position.y;

			// distance check, extend this into aggro, search and wondering system
			if (xMov < focusRange && xMov > -focusRange && yMov < focusRange && yMov > -focusRange) {
				float xAb = std::abs(xMov);
				float yAb = std::abs(yMov);
				float zAb = xAb + yAb;

				if (xMov != 0)
					xMov /= zAb;
				if (yMov != 0)
					yMov /= zAb;

				xMov *= mov;
				yMov *= mov;

				position.x += xMov;
				position.y += yMov;
			}
		}
			break;
		case e_targetingType::LEADING_LINEAR: {

		}
			break;
		case e_targetingType::LEADING_QUADRATIC: {

		}
			break;
		case e_targetingType::MOVE_LINGER: {

		}
			break;
		case e_targetingType::MOVE_ORBIT: {

		}
			break;
		case e_targetingType::MOVE_HIT_AND_RUN: {

		}
			break;
	}



}

// a full routine of approaching and attacking if possible
// i think this should be an anonymous function that is not included in the GameObject. It's too specific for such a general class.
void GameObject::_attackTarget(Environment &ctx) const {

	auto lockedTarget = primaryTarget.lock();

	float xDist = lockedTarget->position.x - position.x;
	float yDist = lockedTarget->position.y - position.y;

	if (xDist < attackRange && xDist > -attackRange && yDist < attackRange && yDist > -attackRange) {
		auto ptr_newBullet = std::make_shared<GameObject>();
		ptr_newBullet->setTarget(lockedTarget);
		ptr_newBullet->movSpeed_max = 0.7;
	}
}

int GameObject::_getNewTarget(Environment &ctx) {
	return 0;
}

// static basic object init
GameObject::GameObject(const std::string& texturePath) {
	setTexture(texturePath);
}
GameObject::GameObject() {
	setTexture("placeholder.bmp");
}

std::shared_ptr<GameObject> GamePreset::generate(Environment &env) {
	auto newObject = std::make_shared<GameObject>(texturePath);

	for(auto &eachChild : childObjects) {
		auto childObject = eachChild.generate(env);
		childObject->setParent(newObject);
	}

	return newObject;
}

void GameObject::update(Environment &ctx) {

	// temporary past value's storage
	rotation_d = rotation;
	position_d = position;

	if (primaryTarget.expired()) {
		// no target, try to acquire new one
		if(!_getNewTarget(ctx)) {
			// no targets, can't acquire new target
			// remove(ctx);
		}
	}

	//TODO: very important: before applying this logic, we need to make sure parent updates first
	if(!parentObject.expired()) {
		position += parentObject.lock()->position_d;
		rotation += parentObject.lock()->rotation_d;
	}

	if(rotSpeed_max > 0.) _rotateToTarget(ctx);
	if(movSpeed_max > 0.) _approachTarget(ctx);
	if(attackRange > 0.) _attackTarget(ctx);

	rotation_d = rotation - rotation_d;
	position_d = position - position_d;
}

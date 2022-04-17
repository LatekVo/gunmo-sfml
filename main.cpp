#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Config.hpp>

#include <list>
#include <iostream>
#include <memory>
#include <valarray>

class Environment; // environment will actually also control all the watch-dogged data and objects that need to be constantly updated
class GamePreset;
class GameObject;

// movement and rotation, may be replaced with unit vectors to target
enum class e_targetingType {
    NONE = 0,
    // HEAD ON
    SIMPLE,
    // SHOOTING ALGO
    LEADING_LINEAR, // leading, assuming straight-line movement
    LEADING_QUADRATIC, // includes acceleration changes,
    // STAYING NEAR
    MOVE_LINGER, // random direction in a vicinity
    MOVE_ORBIT, // orbit in a vicinity
    MOVE_HIT_AND_RUN, // speed past the object (distance is set)
};

enum class e_rotationType {
    NONE = 0,
    SMOOTH = 1,
    SNAP_GRID, // may change this to SNAP_VALUE for more flexibility, equiv to SNAP_8
};

const float PI = 3.14159;

// For starters, player will have like 2 minute truce before enemies start coming,
// maybe each discovered spawner is stronger and has a strength cap,
// growing for a couple of minutes before reaching said cap? Then closing after an hour?

// GameObject are unversal, not only for 'real' objects, but can be used for things like markings as well.
// You could set up a system where the enemy moves to a certain spot, using game objects, while shooting a player,
// since every part on an Enemy that's moving, is a separate, mounted game object.

class Environment {
public:
    // game info & properties
    float g_timeScale = 1.f;
    sf::Clock g_delta;
    float g_currentFrameAdjustment = 0;

    // settings
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

    void addActiveObject(const std::shared_ptr<GameObject>& ptr_newGameObject) {
        data_activeObjects.push_back(ptr_newGameObject);
    }
    void removeActiveObject() {

    }
    void updateGameState(Environment &ctx); // passing self, this is dumb, there has to be a better way

    std::map<std::string, GamePreset> data_GamePresets; // holds copyable objects,

    void newFrameAdjustment() {
        // new frame's delta time counter
        g_currentFrameAdjustment = g_delta.getElapsedTime().asSeconds();
        g_delta.restart();

        // all additional context-dependant variables
        v_mouseWheelTicks = 0.f;
    }
    float getFrameAdjustment() const {
        return g_timeScale * g_currentFrameAdjustment * 1000;
    }

};

class GamePreset {
public:

    std::string texturePath;

    float movSpeed = 0;
    float rotSpeed = 1;

    sf::Vector2f position {0,0};
    float rotation = 0;
    e_rotationType rotatingMode = e_rotationType::NONE;

    float detectionRange = 0.;
    float focusRange = 0.;
    float attackRange = 0.;

    std::vector<GamePreset> childObjects;

    std::shared_ptr<GameObject> generate(Environment &env);

    bool load(std::string filePath) {

        return 0;
    }
};

class GameObject {
private:
    //bin file will be in the bin/ directory anyway, so I can use this format for everything without changing the cmake file
    sf::Texture texture;
    sf::Sprite sprite;

    // linking, for example a turret mounted on a vehicle
    // a child list could be a better solution, though it could have some problems with drawing
    std::weak_ptr<GameObject> parentObject;
    bool isChild = false; // mainly for checking whether the parent is dead, or was there in the first place

public:
    sf::View objectView; // will be used for panning, following

    // currently, defaults to "rotating, not moving", should not be a problem if no target is set
    float movSpeed = 0;
    float rotSpeed = 1; // in degrees, DONT USE RADS

    sf::Vector2f position {0,0};
    float rotation = 0; // deg, still less conversion will have to be done than when using rad up-front
    e_rotationType rotatingMode = e_rotationType::NONE;

    float detectionRange = 0.; // gets aggro
    float focusRange = 0.; // hunts down
    float attackRange = 0.; // maximum hit range
    float hitBox = 0.; //

    std::weak_ptr<GameObject> primaryTarget;
    std::weak_ptr<GameObject> currentTarget; // internal actual target, eg: things in the way of target

    e_targetingType targetingMode = e_targetingType::NONE;

    void setTarget(std::shared_ptr<GameObject> &target, e_targetingType mode = e_targetingType::SIMPLE) {
        primaryTarget = target;
        targetingMode = mode;
    }

    void setParent(std::shared_ptr<GameObject> &arg_newParent) {
        parentObject = arg_newParent; // shared to weak conversion, check later whether this alone works
        isChild = true;
    }

    void setTexture(const std::string& texturePath) {
        if (texture.loadFromFile("rsc/" + texturePath) == 0) {
            std::cout << "replacing texture with a placeholder" << std::endl;
            texture.loadFromFile("rsc/placeholder.bmp");
        }
        sprite.setTexture(texture);
    }

    void draw(sf::RenderWindow& rx) {
        sprite.setPosition(position);
        sprite.setRotation(rotation);
        rx.draw(sprite);
    }

    void updatePlayerInput(Environment &ctx) {

        float mov = movSpeed * ctx.getFrameAdjustment();
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

    void initView(sf::RenderWindow& rx) {
        objectView.reset(sf::FloatRect(0, 0, (float) rx.getSize().x, (float) rx.getSize().y));
    }

    void panView(sf::RenderWindow& rx) {

    }

    void centerView(sf::RenderWindow& rx) {
        objectView.setCenter(position);
        objectView.move((float)sprite.getTextureRect().width / 2,
                        (float)sprite.getTextureRect().height / 2);
        rx.setView(objectView);
    }

    // only sets the object for targeting, may be used internally, functionality expanded in the main update function

    void linkObject(std::shared_ptr<GameObject> &parent) {
        parentObject.reset();
        parentObject = parent;
    }

    void remove(Environment &ctx, bool animation = true) {
        // ctx . data{} . remove()
    }

    void update(Environment &ctx);
    // starting with _ are routine, internal commands
    // those functions will have to be executed multiple times to complete a certain task.
    void _rotateToTarget(Environment &ctx) {

    }
    void _approachTarget(Environment &ctx) {

        auto lockedTarget = primaryTarget.lock();

        // this is temporary, change to switch statement as there are more options

        if (targetingMode == e_targetingType::LEADING_LINEAR) {
            // generate two circles, each representing the velocity, draw a line from one intersection of circles to the other
            // that line's intersection with the target's direction vector should be the hitting point, if the point is behind the target, the bullet is too slow
            // not sure if this will work, though it works on paper

        } else {
            // targetingMode == e_targetingType::SIMPLE

            // TODO: refactor to navmesh navigation
            float mov = movSpeed * ctx.getFrameAdjustment();

            // rewrite, don't recalculate anything, use the already existing difference
            float xMov = lockedTarget->position.x - position.x;
            float yMov = lockedTarget->position.y - position.y;

            // distance check, elaborate this into aggro, search and wondering system
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

    }

    // a full routine of approaching and attacking if possible
    void _attackTarget(Environment &ctx) {

        auto lockedTarget = primaryTarget.lock();

        float xDist = lockedTarget->position.x - position.x;
        float yDist = lockedTarget->position.y - position.y;

        if (xDist < attackRange && xDist > -attackRange && yDist < attackRange && yDist > -attackRange) {
            auto ptr_newBullet = std::make_shared<GameObject>("enemy.bmp");
            ptr_newBullet->setTarget(lockedTarget);
            ptr_newBullet->movSpeed = 0.7;
        }
    }

    static int _getNewTarget(Environment &ctx) {
        return 0;
    }

    // static basic object init
    explicit GameObject(const std::string& texturePath) {
        setTexture(texturePath);
    }
    GameObject() {
        setTexture("placeholder.bmp");
    }
};

std::shared_ptr<GameObject> GamePreset::generate(Environment &env) {
    auto newObject = std::make_shared<GameObject>(texturePath);

    for(auto &eachChild : childObjects) {
        auto childObject = eachChild.generate(env);
        childObject->setParent(newObject);

    }

    return newObject;
}

void GameObject::update(Environment &env) {

    if (primaryTarget.expired()) {
        // no target, try to acquire new one
        if(!_getNewTarget(env)) {
            // no targets, can't acquire new target
            remove(env);
        }
    }

    if(rotSpeed > 0.) _rotateToTarget(env);
    if(movSpeed > 0.) _approachTarget(env);
    if(attackRange > 0.) _attackTarget(env);
}

void Environment::updateGameState(Environment &ctx) {
    /* List of actions:
        call update on every active object
        update camera's position if needed (pan)
    */
    for (auto &each_object : data_activeObjects) {
        each_object->update(ctx);

    }
}

int main() {

    sf::RenderWindow window(sf::VideoMode(), "gunmo", sf::Style::Fullscreen);
    window.setFramerateLimit(144);

    // objects have their own view, game will try being cinematic
    //sf::View mainView(sf::Vector2f(0., 0.), sf::Vector2f((float) window.getSize().x, (float) window.getSize().y));
    //sf::View mainView = window.getDefaultView();

    std::shared_ptr<GameObject> player = std::make_shared<GameObject>("player.bmp");
    player->initView(window);
    player->movSpeed = 1;

    // a temporary reference point
    GameObject ref("placeholder.bmp");
    ref.initView(window);

    Environment env;

    // preloading setup
    GamePreset preset_enemyPlaceholder;

    env.data_GamePresets["enemy_placeholder"] = preset_enemyPlaceholder;

    sf::Clock enemyClock;
    int enemyDelay = 1000;

    while(window.isOpen()) {

        env.newFrameAdjustment();

        sf::Event event {};
        while(window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                        window.close();
                    }
                    break;
                case sf::Event::MouseWheelScrolled:
                        env.v_mouseWheelTicks = event.mouseWheelScroll.delta;
                    break;

                case sf::Event::LostFocus:
                    goto jmp_draw;
                    break;
                case sf::Event::JoystickButtonPressed:
                case sf::Event::JoystickButtonReleased:
                case sf::Event::JoystickMoved:
                    std::cout << "NOTICE: Joystick movement has not been properly implemented yet\n";
                    break;
                case sf::Event::JoystickConnected:
                    std::cout << "HARDWARE: Joystick connected\n";
                    std::cout << "NOTICE: Joystick has not been properly implemented yet\n";
                    break;
                case sf::Event::JoystickDisconnected:
                    std::cout << "HARDWARE: Joystick disconnected\n";
                    break;
            }
        }

        // LOGIC, VIEW, DRAW

        player->updatePlayerInput(env);
        if (env.v_mouseWheelTicks != 0) {
            float zoomFactor = 1 - (env.v_mouseWheelTicks * env.s_mouseWheelSensitivity);

            player->objectView.zoom(zoomFactor);

        }

        env.updateGameState(env);
        // spawn enemy
        if (enemyClock.getElapsedTime().asMilliseconds() > enemyDelay) {
            enemyClock.restart();

            auto ptr_newEnemy = std::make_shared<GameObject>("enemy.bmp");
            ptr_newEnemy->setTarget(player);
            ptr_newEnemy->movSpeed = 0.7;
            ptr_newEnemy->focusRange = 1000.;

            auto ptr_newTurret = std::make_shared<GameObject>("enemy.bmp");
            ptr_newTurret->setParent(ptr_newEnemy);

            env.addActiveObject(ptr_newEnemy);
        }

        // --

        player->centerView(window);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G))
            ref.centerView(window);

        // --
        jmp_draw:

        // menu logic and rest of drawing should end up here

        ref.draw(window);

        player->draw(window);

        for (auto &each_object : env.data_activeObjects) {
            each_object->draw(window);
        }

        for (auto &each_object : env.data_staticObjects) {
            each_object->draw(window);
        }

        window.setView(window.getDefaultView());
        window.display();
        window.clear(sf::Color::White);

    }
}
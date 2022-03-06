#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Config.hpp>

#include <list>
#include <iostream>
#include <memory>

class Environment; // environment will actually also control all the watch-dogged data and objects that need to be constantly updated
class GameObject;

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
    void remActiveObject() {

    }
    void updateGameState(Environment &ctx); // passing self, this is dumb, there has to be a better way

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

class GameObject {
// possibly parenting class, thus everything public
public:
    //bin file will be in the bin/ directory anyway, so I can use this format for everything without changing the cmake file
    sf::Texture texture;
    sf::Sprite sprite;

    sf::View objectView;

    // linking, for example a turret mounted on a vehicle
    std::shared_ptr<GameObject> parentObject;

    // currently defaults to "rotating, not moving"
    float movSpeed = 0;
    float rotSpeed = 1; // in degrees, DONT USE RADS

    sf::Vector2f position {0,0};
    float rotation = 0; // deg, still less conversion will have to be done than when using rad up-front
    e_rotationType rotatingMode = e_rotationType::NONE;

    std::weak_ptr<GameObject> primaryTarget;
    std::weak_ptr<GameObject> currentTarget; // if the path to the target is obstructed, this is first priority target

    e_targetingType targetingMode = e_targetingType::NONE;

    void setTarget(std::shared_ptr<GameObject> &target, e_targetingType mode = e_targetingType::SIMPLE) {
        primaryTarget = target;
        targetingMode = mode;
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

    void deleteSelf() {

    }

    void update(Environment &ctx);
    // starting with _ are routine, internal commands
    // those functions will have to be executed multiple times to complete a certain task.
    void _rotateToTarget(Environment &ctx) {

    }
    void _approachTarget(Environment &ctx) {

    }

    static int _getNewTarget() {
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

void GameObject::update(Environment &ctx) {

    if (primaryTarget.expired()) {
        // no target, try to acquire new one
        if(!_getNewTarget()) {
            // no targets, can't acquire new target
            deleteSelf();
        }
    }

    _rotateToTarget(ctx);
    _approachTarget(ctx);

    // refactor to navmesh navigation
    float mov = movSpeed * ctx.getFrameAdjustment();

    // rewrite, don't recalculate anything, use the already existing difference

    float xMov = primaryTarget.lock()->position.x - position.x;
    float yMov = primaryTarget.lock()->position.y - position.y;

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
    sprite.setPosition(position);
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

            auto ptr_newTurret = std::make_shared<GameObject>("enemy.bmp");

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
#include <iostream>
#include <memory>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Config.hpp>
#include <list>

class Environment; // environment will actually also control all the watch-dogged data and objects that need to be constantly updated
class GameObject;

enum e_targetingType {
    NONE = 0,
    SIMPLE, // face-on
    LEADING_LINEAR, // leading, assuming straight-line movement
    LEADING_QUADRATIC, // includes acceleration changes,
};

const float PI = 3.14159;

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

    void addActiveObject(GameObject *ptr_newGameObject) {
        data_activeObjects.push_back(std::shared_ptr<GameObject>(ptr_newGameObject));
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

    float rotation = 0; // deg, still less conversion will have to be done than when using rad up-front
    sf::Vector2f position {0,0};

    std::shared_ptr<GameObject> targetObject;
    e_targetingType targetingMode = e_targetingType::NONE;

    void setTarget(std::shared_ptr<GameObject> &target, e_targetingType mode = e_targetingType::SIMPLE) {
        targetObject = target;
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

    void update(Environment &ctx);

    // static basic object init
    explicit GameObject(const std::string& texturePath) {
        setTexture(texturePath);
    }
    GameObject() {
        setTexture("placeholder.bmp");
    }
};

void GameObject::update(Environment &ctx) {
    float mov = movSpeed * ctx.getFrameAdjustment();

    float xMov = 0;
    float yMov = 0;

    float x_dist = std::abs(targetObject->position.x) - std::abs(position.x);
    float y_dist = std::abs(targetObject->position.y) - std::abs(position.y);

    if (x_dist > movSpeed /* minimum distance */) {
        if (position.x > targetObject->position.x)
            xMov -= mov;
        else
            xMov += mov;
    }

    if (y_dist > movSpeed /* minimum distance */) {
        if (position.y > targetObject->position.y)
            yMov -= mov;
        else
            yMov += mov;
    }

    if (xMov != 0 && yMov != 0) {
        // 707 = sin 45deg = cos 45deg
        xMov *= .707;
        yMov *= .707;
    }

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
                    // PAUSE IF POSSIBLE
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

        }

        // --

        player->centerView(window);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G))
            ref.centerView(window);

        // --

        ref.draw(window);

        player->draw(window);

        for (auto &each_object : env.data_activeObjects) {
            each_object->draw(window);

        }

        window.setView(window.getDefaultView());
        window.display();
        window.clear(sf::Color::White);
    }
}
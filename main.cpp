#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Config.hpp>

#include "PerlinNoise.h"

#include <array>
#include <list>
#include <iostream>
#include <memory>
#include <valarray>
#include <stdexcept>
#include <deque>

class Environment; // environment will actually also control all the watch-dogged data and objects that need to be constantly updated
class GamePreset;
class GameObject;

class LayeredPerlin {
    unsigned int l_seed = 0;
    unsigned int l_count = 1;
public:
    PerlinNoise noise;

    // no need for scaling value, 1/x works fine 90% of the time, 1/2^x is unnecesarly complex,
    // and anything larger than 1/2 will result in spiky and usually unneeded noisemaps

    LayeredPerlin(unsigned int layers = 1, unsigned int seed = 0) : l_seed(seed), l_count(layers) {

        if (layers < 1) throw std::invalid_argument("LayeredPerlin: template argument 'L' - number of layers must be positive");
        noise.setSeed(l_seed);

    }

    float getNoise(float x, float y) {
        float o = 0;

        for (float i = 1; i <= l_count; i += 1.f) { // NOLINT(cert-flp30-c,cppcoreguidelines-narrowing-conversions)
            o += (float)noise.noise(x / i, y / i, i) / i;
        }

        return o;
    }

    // noise is 3d, we can measure displacement's direction by subtracting noise at one level from noise at level above.
    // if this will be incredibly inefficient, i'll have to modify the pn algo at core to return the very same information.
    // NOTE: not mentioned anywhere, but the first value is a layered noise and the other one is not.
    std::pair<bool, bool> getPolarization3d(float x, float y) {
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
};

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
    NONE = 0, // instant in case of rotation speed > 0
    SMOOTH,
    SNAP_GRID, // may change this to SNAP_VALUE for more flexibility, equiv to SNAP_8
};

const float PI = 3.14159;

// GameObject is universal, not only for 'real' objects, but for things like markings or bullets as well.

// Tile is the smallest building unit, and the smallest biome detail unit, 1 tile has 1 randomly placed biome vertex
// portions of tile are re-generated every time it is loaded, the ones that cannot be altered by user

// For the dynamic chunk loading to work, all of the objects will have to be stored in the chunk data somehow.
// When objects turn active, they are stored in the active register, independently of chunks.
// When they turn dossile (have no job) they turn to the chunk storage.
// Active objects are always globally loaded, and always force a chunk they are in to load as well.

enum class e_biome {
    FOREST,
    PLAINS,
    SAVANNA,
    DESERT,

    COUNT,
    TBD,
};

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

    void addActiveObject(const std::shared_ptr<GameObject>& ptr_newGameObject) {
        data_activeObjects.push_back(ptr_newGameObject);
    }
    void removeActiveObject(const std::shared_ptr<GameObject>& ptr_newGameObject) {
        data_activeObjects.remove(ptr_newGameObject);
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

    static bool load(const std::string &filePath) {

        return 0;
    }
};

// use anonymous function objects for special behaviour, like exploding after set amount of time or something
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
            float mov = movSpeed_max * ctx.getFrameAdjustment();

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
    void _attackTarget(Environment &ctx) const {

        auto lockedTarget = primaryTarget.lock();

        float xDist = lockedTarget->position.x - position.x;
        float yDist = lockedTarget->position.y - position.y;

        if (xDist < attackRange && xDist > -attackRange && yDist < attackRange && yDist > -attackRange) {
            auto ptr_newBullet = std::make_shared<GameObject>();
            ptr_newBullet->setTarget(lockedTarget);
            ptr_newBullet->movSpeed_max = 0.7;
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

void Chunk::swapObjectToActive(std::shared_ptr<GameObject> &obj, Environment &env) {
    env.data_activeObjects.push_back(obj);
    data_passiveObjects.remove(obj);
}

void Chunk::swapObjectToPassive(std::shared_ptr<GameObject> &obj, Environment &env) {
    data_passiveObjects.push_back(obj);
    env.data_activeObjects.remove(obj);
}

class ChunkManager {
private:
    LayeredPerlin chunkPerlin;

    // a simple 3x3 buffer and maybe some extensions to 5x5 realised inside the chunk logic
    std::deque<std::deque<Chunk>> playerBuffer;

    e_biome _perlinToBiome(float x, float y) {
        auto polar = chunkPerlin.getPolarization3d(x, y);
        return (e_biome)(polar.first + polar.second + polar.second);
    }

    Chunk _generate(int x, int y) {
        Chunk newChunk;
        int chunkSize = newChunk.tileMap.size();
        float globalCoords[] {(float)(x * chunkSize), (float)(y * chunkSize)};

        for (unsigned int x_i = 0; x_i < chunkSize; x_i++) {
            for (unsigned int y_i = 0; y_i < chunkSize; y_i++) {
                Tile newTile;

                //newTile.biomeVertex = {}; // i need 2 persistent 0->1 values here

                // noise for:   chunk position  +  tile offset  +  tile offset offset
                newTile.biomeName = _perlinToBiome(globalCoords[0] + (float)x_i + newTile.biomeVertex[0], globalCoords[1] + (float)y_i + newTile.biomeVertex[1]);
                newChunk.tileMap[x][y] = newTile;
            }
        }

        return newChunk;
    }
    // looking for chunk to load sequence: activeChunkBuffer -> chunkBuffer -> savedOnFile -> generateNew
    void _load() {

    }
public:
    explicit ChunkManager(Environment &ctx) {
        chunkPerlin = LayeredPerlin(2, ctx.s_mapSeed);
    }
    void update() {

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
    player->movSpeed_max = 1;

    // a temporary reference point
    GameObject centerReferenceBlock("placeholder.bmp");
    centerReferenceBlock.initView(window);

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
            ptr_newEnemy->movSpeed_max = 0.7;
            ptr_newEnemy->focusRange = 1000.;

            auto ptr_newTurret = std::make_shared<GameObject>("turret.png");
            ptr_newTurret->setTarget(player);
            ptr_newTurret->setParent(ptr_newEnemy);
            ptr_newEnemy->rotSpeed_max = 0.7;

            env.addActiveObject(ptr_newEnemy);
            env.addActiveObject(ptr_newTurret);
        }

        // --

        player->centerView(window);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G))
            centerReferenceBlock.centerView(window);

        // --
        jmp_draw:

        // menu logic and rest of drawing should end up here

        centerReferenceBlock.draw(window);

        player->draw(window);

        for (auto &each_object : env.data_activeObjects) {
            each_object->draw(window);
        }

        for (auto &each_object : env.data_staticObjects) {
            each_object->draw(window);
        }


        // DEBUG OVERLAY
        /*
        sf::Text text;
        text.setString("Hello world");
        text.setCharacterSize(24); // in pixels, not points!
        text.setFillColor(sf::Color::Green);
        window.draw(text);
        */

        window.setView(window.getDefaultView());
        window.display();
        window.clear(sf::Color::White);

    }
}
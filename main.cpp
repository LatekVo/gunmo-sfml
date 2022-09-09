#include <iostream>
#include <memory>

#include "EngineEssentials.h"
#include "LibDraw.h"

// CURRENT HIERARCHY PLANS
/*
 * 	Environment: Runtime tasks, runtime object management, probing, logic, drawing.
 * 	ChunkManager: Long-term storage, chunk generation, position storage. Moves all active objects into Environment's jurisdiction.
 * 	GameObject: Basic building block of any intractable object. More customization will likely be achieved by implementing LUA or by dynamically loading (is that even manageable???) foreign functors.
 *  GamePreset: A format used for storing GameObject presets. There are some limitations to storing GameObject in a binary format directly, and GamePreset helps to avoid them.
 *  			Additionally, it acts as a means of storage for already loaded GO presets, they can be then used to spawn multiple identical GOs
 *
 *  LibDraw: An independent class/lib, reads other classes but isn't read by any. It's an optional graphical interface.
 */

int main() {

	// Base initialization
    auto window = std::make_shared<sf::RenderWindow>(sf::VideoMode(), "t", sf::Style::Fullscreen);
    window->setFramerateLimit(144);

    std::shared_ptr<GameObject> player = std::make_shared<GameObject>("player.bmp");
    player->movSpeed_max = 1;

    auto env = std::make_shared<Environment>();

    // preloading setup
    GameObject preset_enemyPlaceholder;

    env->data_GamePresets["enemy_placeholder"] = preset_enemyPlaceholder;

    sf::Clock enemyClock;
    int enemyDelay = 1000;

    while(window->isOpen()) {

        env->newFrameAdjustment();

        sf::Event event {};
        while(window->pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window->close();
                    break;
                case sf::Event::KeyPressed:
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                        window->close();
                    }
                    break;
                case sf::Event::MouseWheelScrolled:
                        env->v_mouseWheelTicks = event.mouseWheelScroll.delta;
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

		// todo: reverse this function, player(env) -> env(player)
        player->updatePlayerInput(*env);

		// todo: this function shouldn't have to feed itself
        env->updateGameState(*env);

        // debug: spawn enemy
		// instead, create a spawner item prefab, that spawns/shoots the placeholder_enemy prefab (spawning is already supported i think)

        // --

        jmp_draw:

        // menu logic and rest of drawing should end up here

		/*
        for (auto &each_object : env.data_activeObjects) {
            each_object->draw(window);
        }

        for (auto &each_object : env.data_staticObjects) {
            each_object->draw(window);
        }
		*/

        // DEBUG OVERLAY
        /*
        sf::Text text;
        text.setString("Hello world");
        text.setCharacterSize(24); // in pixels, not points!
        text.setFillColor(sf::Color::Green);
        window.draw(text);
        */

        window->setView(window->getDefaultView());
        window->display();
        window->clear(sf::Color::White);

    }
}
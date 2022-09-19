#include <iostream>
#include <memory>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>


#include "EngineEssentials.h"
#include "ChunkManagement.h"
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
 *
 *  Any libraries that need to use external low level libraries are to be kept optional and replaceable.
 *
 */

int main() {

	// Base initialization
    auto window = std::make_shared<sf::RenderWindow>(sf::VideoMode(), "t", sf::Style::Fullscreen);
    window->setFramerateLimit(144);

    std::shared_ptr<GameObject> player = std::make_shared<GameObject>();
    player->movSpeed_max = 1;

    auto env = std::make_shared<Environment>();
	auto chunkMan = std::make_shared<ChunkManager>(env);

	// preloading setup
    GameObject preset_enemyPlaceholder;

    env->data_GamePresets["enemy_placeholder"] = preset_enemyPlaceholder;

	LibDraw drawAgent;
	drawAgent.attach(window, env, chunkMan);

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

		// after some thinking, having input gathering in GOs is better than in env, simply put, there is much more flexibility and multiplayer implementation ease when using this system.
		player->updatePlayerInput(*env);

		jmp_draw:

		env->updateGameState();

        window->setView(window->getDefaultView());
        window->display();
        window->clear(sf::Color::White);

    }
}
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Config.hpp>

#include "EngineEssentials.h"

#include <array>
#include <list>
#include <iostream>
#include <memory>
#include <valarray>
#include <stdexcept>
#include <deque>
#include <map>

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
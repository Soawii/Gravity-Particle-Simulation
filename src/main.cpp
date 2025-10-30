#include "Body.h"
#include "Screen.h"
#include "BodySimulation.h"
#include "MouseInputHandler.h"
#include "Buttons.h"
#include "Menu.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

int main()
{
	srand(time(NULL));
	Screen::window.create(sf::VideoMode(Screen::WIDTH, Screen::HEIGHT), "BarnesHut", sf::Style::Close | sf::Style::Titlebar | sf::Style::Resize);
	Screen::window.setFramerateLimit(Constants::FPS);
	std::vector<Body> bodies;
	BodySimulation sim(bodies, 0.6f, 10);
	MouseInputHandler mouseHandler(Screen::window, sim);

	Menu menu(sim);

	sf::Clock clock;

	while (Screen::window.isOpen())
	{
		Constants::CURRENT_FPS = 1.0f / clock.restart().asSeconds();

		sf::Event event;
		while (Screen::window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				Screen::window.close();
			}
			else if (event.type == sf::Event::Resized)
			{
				Screen::screenChanged();
			}
			if (!menu.handler.processEvent(event))
				mouseHandler.handleEvent(event);
		}
		Screen::window.clear(Screen::BACKGROUD_COLOR);
		mouseHandler.frameUpdate();
		menu.handler.update(sf::Mouse::getPosition(Screen::window));

		sim.update(Constants::dt);

		sim.draw(Screen::window);

		Screen::window.draw(menu.handler);

		Screen::window.display();
	}
}
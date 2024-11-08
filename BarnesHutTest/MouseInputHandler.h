#pragma once

#include <SFML/Graphics.hpp>
#include "BodySimulation.h"
#include "Screen.h"
#include "Spawner.h"
#include <vector>

class MouseInputHandler
{
public:
	sf::RenderWindow& window;
	BodySimulation& sim;
	Spawner spawner;
	float scrollSpeed = 0.1f;

	bool leftButtonPressed = false, prevLeftButtonPressed = false;
	float circleRadius = std::min(Screen::WIDTH, Screen::HEIGHT) / 40.0f;
	float circleMass = 4e10;
	float wallRadius = std::min(Screen::WIDTH, Screen::HEIGHT) / 400.0f;
	float wallMass = 2e11;

	bool rightButtonPressed = false;
	sf::Vector2f rightButtonPressedPos = { 0, 0 };
	float projectile_radius = std::min(Screen::WIDTH, Screen::HEIGHT) / 25.0f;
	float projectile_mass = 5e10;

	sf::Vector2i prev_pos;

	MouseInputHandler(sf::RenderWindow& window, BodySimulation& sim) :
		window(window), sim(sim), spawner(sim.bodies) {}

	void update_radius()
	{
		circleRadius = std::min(Screen::WIDTH, Screen::HEIGHT) / 40.0f;
		projectile_radius = std::min(Screen::WIDTH, Screen::HEIGHT) / 25.0f;
	}

	void handleEvent(sf::Event& event)
	{
		update_radius();
		if (event.type == sf::Event::MouseWheelScrolled)
		{
			float delta = event.mouseWheelScroll.delta;
			int sign = delta < 0 ? -1 : 1;
			Screen::zoomIn(sign * scrollSpeed, sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y));
			update_bodies();
		}
		else if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				leftButtonPressed = true;
			}
			else if (event.mouseButton.button == sf::Mouse::Right)
			{
				rightButtonPressed = true;
				rightButtonPressedPos.x = event.mouseButton.x;
				rightButtonPressedPos.y = event.mouseButton.y;
			}
			else if (event.mouseButton.button == sf::Mouse::Middle)
			{
				std::vector<Body>& bodies = sim.bodies;
				sf::Vector2f center = Screen::window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
				if (Constants::mode == "CIRCLE")
				{
					float radius = Screen::convertPixelsToMeters(sf::Vector2f(circleRadius, circleRadius)).x;
					spawner.spawnCircle(center, radius, radius, 5, 4, projectile_mass);
				}
				else if (Constants::mode == "WALL")
				{
					sf::Vector2f topLeft = center - sf::Vector2f(Screen::X / 4.0f, Screen::Y / 4.0f);
					float radius = Screen::convertPixelsToMeters(sf::Vector2f(wallRadius, wallRadius)).x;
					spawner.spawnWall(topLeft, 30, 30, radius, Screen::X / 2.0f / 30.0f, wallMass);
				}
			}
		}
		else if (event.type == sf::Event::MouseButtonReleased)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				leftButtonPressed = false;
			}
			else if (event.mouseButton.button == sf::Mouse::Right)
			{
				float radius = Screen::convertPixelsToMeters(sf::Vector2f(projectile_radius, projectile_radius)).x;
				sf::Vector2f mouse_pressed_pos = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
				spawner.spawnBody(Screen::window.mapPixelToCoords(sf::Vector2i(mouse_pressed_pos.x, mouse_pressed_pos.y)), 
					radius, projectile_mass, Screen::convertPixelsToMeters(rightButtonPressedPos - mouse_pressed_pos) / 10.0f);
				rightButtonPressed = false;
			}
		}
	}

	void update_bodies()
	{
		for (Body& body : sim.bodies)
		{
			body.update_drawables();
		}
	}

	void frameUpdate()
	{
		update_radius();
		sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);

		if (leftButtonPressed)
		{
			if (prevLeftButtonPressed)
			{
				Screen::move(mouse_pos - prev_pos);
			}
			prevLeftButtonPressed = true;
		}
		else
		{
			prevLeftButtonPressed = false;
		}

		if (rightButtonPressed)
		{
			float radius = Screen::convertPixelsToMeters(sf::Vector2f(projectile_radius, projectile_radius)).x;
			sf::CircleShape circle(radius, 60);
			sf::Color circle_color(sf::Color::Cyan);
			circle.setFillColor(circle_color);
			circle.setOrigin(radius, radius);
			circle.setPosition(Screen::window.mapPixelToCoords(mouse_pos));

			sf::Color line_color(sf::Color::Red);
			line_color.a = 50;

			sf::Vertex line[] = { sf::Vertex(Screen::window.mapPixelToCoords(mouse_pos), line_color),
									sf::Vertex(Screen::window.mapPixelToCoords(sf::Vector2i(rightButtonPressedPos.x, rightButtonPressedPos.y)), line_color) };

			window.draw(line, 2, sf::Lines);
			window.draw(circle);
		}

		prev_pos = mouse_pos;
	}
};
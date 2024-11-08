#pragma once
#include "Screen.h"
#include <SFML/Graphics.hpp>
#include <iostream>

class alignas(64) Body
{
public:
	sf::Vector2f center, prev_center;
	float mass, radius;

	sf::CircleShape* circle = new sf::CircleShape();
	sf::Vertex* point = new sf::Vertex();

	sf::Vector2f acceleration;
	bool fixed, isVertex = true;
	bool enabled = true;

	Body(sf::Vector2f center, float mass, float radius, sf::Vector2f velocity, bool fixed) :
		center(center), prev_center(center - velocity), mass(mass), radius(radius), acceleration(0, 0), fixed(fixed)
	{
		point->position = center;
		point->color = Screen::BODY_COLOR;
		circle->setFillColor(Screen::BODY_COLOR);
		circle->setRadius(radius);
		circle->setOrigin(radius, radius);
		circle->setPosition(center);
		update_drawables();
	}

	void setAcceleration(sf::Vector2f _acceleration)
	{
		acceleration = _acceleration;
	}

	void checkForNan()
	{
		if (isinf(center.x) || isnan(center.x) || isinf(center.y) || isnan(center.y))
		{
			center.x = 0;
			center.y = 0;
			mass = 0;
			radius = 0.01;
			enabled = false;
		}
	}

	void update(float dt)
	{
		if (!enabled || fixed)
			return;
		sf::Vector2f velocity = center - prev_center;
		prev_center = center;
		center += velocity + acceleration * dt * dt;
		checkForNan();
	}

	void draw(sf::RenderWindow& window)
	{
		if (!isInWindow())
			return;
		if (isVertex)
		{
			point->position = center;
			window.draw(point, 1, sf::Points);
		}
		else
		{
			circle->setPosition(center);
			window.draw(*circle);
		}
	}

	void update_drawables()
	{
		if (!enabled)
			return;
		float pixel_radius = Screen::convertMetersToPixels(sf::Vector2f(radius, radius)).x;
		if (pixel_radius > 1)
			isVertex = false;
		else
			isVertex = true;
	}

	bool isInWindow()
	{
		if (!enabled)
			return false;
		int Xn = std::max(Screen::TOP_LEFT.x, std::min(center.x, Screen::BOTTOM_RIGHT.x));
		int Yn = std::max(Screen::TOP_LEFT.y, std::min(center.y, Screen::BOTTOM_RIGHT.y));
		int Dx = Xn - center.x;
		int Dy = Yn - center.y;
		return (Dx * Dx + Dy * Dy) <= radius * radius;
	}

	void checkBounds()
	{
		if (center.x >= Screen::X)
			center.x = Screen::X - 1;
		if (center.x <= 0)
			center.x = 1;
		if (center.y >= Screen::Y)
			center.y = Screen::Y - 1;
		if (center.y <= 0)
			center.y = 1;
	}

	void handleCollision(Body& other)
	{
		if (!enabled || !other.enabled)
			return;
		if (fixed && other.fixed)
			return;
		float dx = other.center.x - center.x, dy = other.center.y - center.y;
		float min_distance = radius + other.radius;
		float distance = std::sqrt(dx * dx + dy * dy);
		if (distance > min_distance)
			return;
		float distance_to_add = (min_distance - distance);

		float total_part = std::max(0.1f, abs(dx) + abs(dy));
		float dx_part = dx / total_part;
		float dy_part = dy / total_part;
		sf::Vector2f vec_distance_to_add(distance_to_add * dx_part, distance_to_add * dy_part);

		float mass_ratio = mass / (mass + other.mass);
		if (fixed)
			mass_ratio = 1;
		else
		{
			center.x -= vec_distance_to_add.x * (1 - mass_ratio);
			center.y -= vec_distance_to_add.y * (1 - mass_ratio);
			checkForNan();
		}
		if (!other.fixed)
		{
			other.center.x += vec_distance_to_add.x * mass_ratio;
			other.center.y += vec_distance_to_add.y * mass_ratio;
			other.checkForNan();
		}
	}
};
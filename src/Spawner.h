#pragma once
#include "Body.h"
#include <vector>

class Spawner
{
public:
	std::vector<Body>& bodies;
	Spawner(std::vector<Body>& bodies) : bodies(bodies) {}

	void spawnBody(sf::Vector2f center, float radius, float mass_coeff, sf::Vector2f velocity)
	{
		float mass = 4 * Constants::PI / 3 * radius * radius * radius * mass_coeff;
		if (isnan(mass) || isinf(mass))
			return;
		bodies.push_back(Body(center, mass, radius, velocity, false));
	}

	void spawnCircle(sf::Vector2f center, float radius, float space_between_bodies, int layers, float bodies_per_layer, float mass_coeff)
	{
		float mass = 4 * Constants::PI / 3 * radius * radius * radius * mass_coeff;
		if (isnan(mass) || isinf(mass))
			return;
		bodies.push_back(Body(center, mass * 5, radius, sf::Vector2f(0, 0), false));
		for (int i = 1; i < layers; i++)
		{
			float r = i * (2 * radius) + i * space_between_bodies + radius;
			for (int j = 0; j < bodies_per_layer; j++)
			{
				float angle = 2 * Constants::PI / bodies_per_layer * j;
				bodies.push_back(Body(sf::Vector2f(center.x + r * cos(angle), center.y + r * sin(angle)), mass, radius, sf::Vector2f(0, 0), false));
			}
			bodies_per_layer *= 1.5;
		}
	}

	void spawnWall(sf::Vector2f top_left, int N, int M, float radius, float space_between_bodies, float mass_coeff)
	{
		float mass = 4 * Constants::PI / 3 * radius * radius * radius * mass_coeff;
		if (isnan(mass) || isinf(mass))
			return;

		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < M; j++)
			{
				bodies.push_back(Body(sf::Vector2f(top_left.x + j * space_between_bodies + radius * (j + 1),
													top_left.y + i * space_between_bodies +radius * (i + 1)),
					mass, radius, sf::Vector2f(0, 0), false));
			}
		}
	}
};
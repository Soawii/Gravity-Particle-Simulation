#pragma once

#include "Body.h"
#include "Screen.h"
#include "BarnesHut.h"
#include <SFML/Graphics.hpp>
#include <vector>

class BodySimulation
{
public:
	std::vector<Body>& bodies;
	BarnesHut bh;
	CollisionHandler collision_handler;
	bool showQuadTree = false;
	int collisionPrecision = 2;

	BodySimulation(std::vector<Body>& bodies, float threshold, int maxLeafSize) 
		: bodies(bodies), bh(bodies, threshold, maxLeafSize), collision_handler(bodies, bh.head) {}

	void update(float dt)
	{
		sf::Clock clock;

		bh.createTree();
		float tree_creation = clock.restart().asMilliseconds();

		for (int i = 0; i < collisionPrecision; i++)
			collision_handler.handleCollisions();

		bh.applyGravity();

		for (Body& body : bodies)
		{
			body.update(dt);
		}
		for (int i = 0; i < bodies.size(); i++)
		{
			if (!bodies[i].enabled)
			{
				bodies.erase(bodies.begin() + i);
				i--;
			}
		}
	}

	void draw(sf::RenderWindow& window)
	{
		for (Body& body : bodies)
		{
			body.draw(window);
		}
		if (showQuadTree)
			bh.head.draw(window);
	}
};
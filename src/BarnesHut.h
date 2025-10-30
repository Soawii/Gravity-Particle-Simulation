#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include "Body.h"
#include "Screen.h"
#include "QuadTree.h"
#include "CollisionHandler.h"

class BarnesHut
{
public:
	std::vector<Body>& bodies;
	float threshold;
	float eps = 0.0001;
	QuadTree head;
	int maxLeafSize;

	BarnesHut(std::vector<Body>& bodies, float threshold, int maxLeafSize) 
		: bodies(bodies), threshold(threshold), maxLeafSize(maxLeafSize), head(bodies, maxLeafSize){}

	void createTree()
	{
		sf::Vector2f center(Screen::X / 2, Screen::Y / 2);
		head.build();
	}

	void applyGravity(int num_threads) const
	{
		const int batch_size = bodies.size() / num_threads;
		if (batch_size > 0) {
			std::vector<std::thread> threads;
			for (int i = 0; i < num_threads; i++)
			{
				const int start = i * batch_size, end = (i + 1) * batch_size;
				threads.emplace_back([this, start, end]() {
					for (int i = start; i < end; i++) {
						getAcceleration(i);
					}
				});
			}
			for (int i = 0; i < num_threads; i++) {
				threads[i].join();
			}
		}
		for (int i = 0; i < int(bodies.size()) % num_threads; i++)
		{
			getAcceleration(int(bodies.size()) - 1 - i);
		}

		for (int i = 0; i < bodies.size(); i++)
		{
			if (isnan(bodies[i].acceleration.x) || isnan(bodies[i].acceleration.y) || isinf(bodies[i].acceleration.x) || isinf(bodies[i].acceleration.y))
			{
				bodies.erase(bodies.begin() + i);
				i--;
			}
		}
	}

	void getAcceleration(size_t body) const
	{
		if (bodies[body].fixed || !bodies[body].enabled)
			return;
		getAccelerationHelper(body);
		bodies[body].acceleration *= Constants::G;
	}

	void getAccelerationHelper(size_t index) const
	{
		Body& body = bodies[index];
		int node_index = 0;

		while (true)
		{
			const Node& node = head.nodes[node_index];

			sf::Vector2f delta(node.center_mass.x - body.center.x, node.center_mass.y - body.center.y);
			float d = sqrt(delta.x * delta.x + delta.y * delta.y);


			if (node.isEmpty() || (d < eps))
			{
				if (node.next == 0)
					break;
				node_index = node.next;
				continue;
			}

			if ((node.isLeaf() || (node.bottom_right.x - node.top_left.x) / d < threshold) && !(index >= node.start && index < node.end))
			{
				body.acceleration += node.mass / d / d / d * delta;

				if (node.next == 0)
					break;

				node_index = node.next;
			}
			else if (!node.isLeaf())
			{
				node_index = node.children;
			}
			else
			{
				if (node.next == 0)
					break;
				node_index = node.next;
			}
		}
	}
};
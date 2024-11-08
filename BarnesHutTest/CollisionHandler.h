#pragma once
#include "BodySimulation.h"
#include "Body.h"
#include <vector>
#include <omp.h>

class CollisionHandler 
{
public:
	std::vector<Body>& bodies;
	QuadTree& tree;

	CollisionHandler(std::vector<Body>& bodies, QuadTree& tree) : bodies(bodies), tree(tree) {}

	void handleCollisions() const
	{
		// Assumes the quad tree has already been updated to the current frame

		/*for (int i = 0; i < bodies.size(); i++)
		{
			handleCollisionForBody(i, tree.nodes[0]);
			if (!bodies[i].enabled)
			{
				bodies.erase(bodies.begin() + i);
				i--;
			}
		}*/

		int N = bodies.size();

		#pragma omp parallel num_threads(4)
		{
			int i = omp_get_thread_num();
			int start = (i == 0) ? 0 : tree.nodes[0].splits[i - 1];
			int end = (i == 3) ? N : tree.nodes[0].splits[i];
			for (int j = start; j < end; j++)
				handleCollisionForBody(j, tree.nodes[1 + i]);
		}

		float max_radius = tree.nodes[0].maxRadius;
		std::vector<bool> on_center(bodies.size(), false);
		sf::Vector2f center = tree.nodes[0].top_left + (tree.nodes[0].bottom_right - tree.nodes[0].top_left) / 2.0f;
		for (int i = 0; i < N; i++)
		{
			if (std::min(abs(bodies[i].center.x - center.x), abs(bodies[i].center.y - center.y)) < bodies[i].radius + max_radius)
			{
				on_center[i] = true;
			}
		}

		for (int i = 0; i < bodies.size(); i++)
		{
			if (on_center[i])
				handleCollisionForBody(i, tree.nodes[0]);
			if (!bodies[i].enabled)
			{
				bodies.erase(bodies.begin() + i);
				i--;
			}
		}
	}

	void handleCollisionForBody(int index, const Node& node) const
	{
		Body& body = bodies[index];
		if (node.isEmpty() || node.distanceFromBody(bodies[index]) > (body.radius + node.maxRadius))
			return;
		if (node.isLeaf())
		{
			if (node.start <= index && index < node.end)
			{
				for (int i = node.start; i < index; i++)
					body.handleCollision(bodies[i]);
				for (int i = index + 1; i < node.end; i++)
					body.handleCollision(bodies[i]);
			}
			else
			{
				for (int i = node.start; i < node.end; i++)
					body.handleCollision(bodies[i]);
			}
			return;
		}

		for (int i = 0; i < 4; i++)
		{
			handleCollisionForBody(index, tree.nodes[node.children + i]);
		}
	}
};
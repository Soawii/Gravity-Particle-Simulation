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

		const int N = bodies.size();

		#pragma omp parallel for
		for (int i = 0; i < N; i++)
		{
			bodies[i].new_center = bodies[i].center;
			handleCollisionForBody(i, tree.nodes[0]);
		}


		for (int i = 0; i < bodies.size(); i++) {
			bodies[i].center = bodies[i].new_center;
			bodies[i].checkForNan();

			if (!bodies[i].enabled) {
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
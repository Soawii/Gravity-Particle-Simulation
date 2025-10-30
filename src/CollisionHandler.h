#pragma once
#include "BodySimulation.h"
#include "Body.h"
#include <vector>
#include <thread>

class CollisionHandler 
{
public:
	std::vector<Body>& bodies;
	QuadTree& tree;

	CollisionHandler(std::vector<Body>& bodies, QuadTree& tree) : bodies(bodies), tree(tree) {}

	void handleCollisions(int num_threads) const
	{
		// Assumes the quad tree has already been updated to the current frame

		std::vector<int> leafs;
		for (int i = 0; i < tree.nodes.size(); i++) {
			if (tree.nodes[i].isLeaf() && !tree.nodes[i].isEmpty()) {
				leafs.push_back(i);
			}
		}

		std::vector<int> edge_bodies;
		for (int i = 0; i < leafs.size(); i++) {
			for (int j = tree.nodes[leafs[i]].start; j < tree.nodes[leafs[i]].end; j++) {
				float x_d = std::min(bodies[j].center.x - tree.nodes[leafs[i]].top_left.x, tree.nodes[leafs[i]].bottom_right.x - bodies[j].center.x);
				float y_d = std::min(bodies[j].center.y - tree.nodes[leafs[i]].top_left.y, tree.nodes[leafs[i]].bottom_right.y - bodies[j].center.y);
				if (x_d < bodies[j].radius || y_d < bodies[j].radius) {
					edge_bodies.push_back(j);
				}
			}
		}

		const int batch_size = leafs.size() / num_threads;
		if (batch_size > 0) {
			std::vector<std::thread> threads;
			for (int i = 0; i < num_threads; i++)
			{
				const int start = i * batch_size, end = (i + 1) * batch_size;
				threads.emplace_back([this, &leafs, start, end]() {
					for (int i = start; i < end; i++) {
						handleCollisionInLeaf(tree.nodes[leafs[i]]);
					}
				});
			}
			for (int i = 0; i < num_threads; i++) {
				threads[i].join();
			}
		}
		for (int i = 0; i < int(leafs.size()) % num_threads; i++)
		{
			handleCollisionInLeaf(tree.nodes[leafs[int(leafs.size()) - 1 - i]]);
		}

		for (int i = 0; i < edge_bodies.size(); i++) {
			handleCollisionForBody(edge_bodies[i], tree.nodes[0]);
		}
	}

	void handleCollisionInLeaf(const Node& node) const {
		for (int i = node.start; i < node.end - 1; i++) {
			for (int j = i + 1; j < node.end; j++) {
				bodies[i].handleCollision(bodies[j]);
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
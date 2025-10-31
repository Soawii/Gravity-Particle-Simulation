#pragma once
#include "Body.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>


class alignas(64) Node
{
public:
	sf::Vector2f top_left, bottom_right;
	int next, depth;
	float maxRadius = 0;
	int children = 0;
	int splits[3] = {0, 0, 0};

	sf::Vector2f center_mass{ 0, 0 };

	float mass = 0;
	int start, end;

	Node(sf::Vector2f top_left, sf::Vector2f bottom_right, int next, int start, int end, int depth) 
		: top_left(top_left), bottom_right(bottom_right), next(next), start(start), end(end), depth(depth)
	{
		sf::Vector2f center = sf::Vector2f(top_left.x + (bottom_right.x - top_left.x) / 2, top_left.y + (bottom_right.y - top_left.y) / 2);
	}

	inline bool isEmpty() const
	{
		return mass == 0;
	}

	inline bool isLeaf() const
	{
		return children == 0;
	}

	inline int getLeafSize() const
	{
		return end - start;
	}

	float distanceFromBody(const Body& body) const
	{
		float Xn = std::max(top_left.x, std::min(body.center.x, bottom_right.x));
		float Yn = std::max(top_left.y, std::min(body.center.y, bottom_right.y));
		float Dx = Xn - body.center.x;
		float Dy = Yn - body.center.y;
		return sqrt(Dx * Dx + Dy * Dy);
	}
};

class alignas(64) QuadTree
{
public:
	std::vector<Node> nodes;
	int maxLeafSize;
	std::vector<Body>& bodies;

	QuadTree(std::vector<Body>& bodies, int maxLeafSize) 
		: bodies(bodies), maxLeafSize(maxLeafSize)
	{
	}

	void createChildren(size_t index)
	{
		if (nodes[index].depth > 200)
			return;
		const int start = nodes[index].start, end = nodes[index].end;
		const sf::Vector2f length = (nodes[index].bottom_right - nodes[index].top_left) / 2.0f;
		const sf::Vector2f center = nodes[index].top_left + length;

		int splits[] = { start, 0, 0, 0, end };

		splits[2] = std::partition(bodies.begin() + start, bodies.begin() + end, [&center](const Body& b) {
			return b.center.y < center.y;
			}) - bodies.begin();

		splits[1] = std::partition(bodies.begin() + start, bodies.begin() + splits[2], [&center](const Body& b) {
			return b.center.x < center.x;
			}) - bodies.begin();

		splits[3] = std::partition(bodies.begin() + splits[2], bodies.begin() + end, [&center](const Body& b) {
			return b.center.x < center.x;
			}) - bodies.begin();

		nodes[index].splits[0] = splits[1];
		nodes[index].splits[1] = splits[2];
		nodes[index].splits[2] = splits[3];

		nodes[index].children = nodes.size();

		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				nodes.emplace_back(sf::Vector2f(nodes[index].top_left.x + j * length.x, nodes[index].top_left.y + i * length.y),
					sf::Vector2f(nodes[index].bottom_right.x - (1 - j) * length.x, nodes[index].bottom_right.y - (1 - i) * length.y),
					(((i == 1) && (j == 1)) ? nodes[index].next : nodes.size() + 1),
					splits[2 * i + j], splits[2 * i + j + 1], nodes[index].depth + 1);
			}
		}
	}

	void build()
	{
		nodes.clear();
		nodes.reserve(bodies.size() / 4);

		sf::Vector2f top_left(INT_MAX, INT_MAX), bottom_right(INT_MIN, INT_MIN);
		for (const Body& body : bodies)
		{
			if (!body.enabled)
				continue;
			if (body.center.x < top_left.x)
				top_left.x = body.center.x;
			if (body.center.y < top_left.y)
				top_left.y = body.center.y;
			if (body.center.x > bottom_right.x)
				bottom_right.x = body.center.x;
			if (body.center.y > bottom_right.y)
				bottom_right.y = body.center.y;
		}
		bottom_right.x += 0.1f;
		bottom_right.y += 0.1f;

		float x_length = bottom_right.x - top_left.x;
		float y_length = bottom_right.y - top_left.y;

		if (x_length < y_length)
		{
			bottom_right.x += (y_length - x_length) / 2;
			top_left.x -= (y_length - x_length) / 2;
			x_length = y_length;
		}
		if (y_length < x_length)
		{
			bottom_right.y += (x_length - y_length) / 2;
			top_left.y -= (x_length - y_length) / 2;
			y_length = x_length;
		}

		nodes.emplace_back(top_left, bottom_right, 0, 0, bodies.size(), 0);

		for (int i = 0; i < nodes.size(); i++)
		{
			//std::cout << i << " " << nodes[i].getLeafSize() << std::endl;
			if (nodes[i].getLeafSize() > maxLeafSize)
			{
				createChildren(i);
			}
			else
			{
				sf::Vector2f mass_sum{ 0, 0 };
				for (int j = nodes[i].start; j < nodes[i].end; j++)
				{
					mass_sum += bodies[j].center * bodies[j].mass;
					nodes[i].mass += bodies[j].mass;
					nodes[i].maxRadius = std::max(nodes[i].maxRadius, bodies[j].radius);
				}
				if (nodes[i].mass != 0)
					nodes[i].center_mass = mass_sum / nodes[i].mass;
			}
		}
		calculateCenterMass();
	}

	void calculateCenterMass()
	{
		for (int i = nodes.size() - 1; i >= 0; i--)
		{
			if (nodes[i].isLeaf())
				continue;
			int c = nodes[i].children;

			while (c != nodes[i].next)
			{
				nodes[i].center_mass += nodes[c].mass * nodes[c].center_mass;
				nodes[i].mass += nodes[c].mass;
				c = nodes[c].next;
				nodes[i].maxRadius = std::max(nodes[i].maxRadius, nodes[c].maxRadius);
			}

			nodes[i].center_mass /= nodes[i].mass;
		}
	}

	void draw(sf::RenderWindow& window, size_t index = 0) const
	{
		const Node& node = nodes[index];

		sf::Vector2f center = sf::Vector2f(node.top_left.x + (node.bottom_right.x - node.top_left.x) / 2, node.top_left.y + (node.bottom_right.y - node.top_left.y) / 2);

		sf::Vertex box[] = { sf::Vertex(node.top_left, sf::Color::Green),
			sf::Vertex(sf::Vector2f(node.bottom_right.x, node.top_left.y), sf::Color::Green),
			sf::Vertex(node.bottom_right, sf::Color::Green),
			sf::Vertex(sf::Vector2f(node.top_left.x, node.bottom_right.y), sf::Color::Green),
			sf::Vertex(node.top_left, sf::Color::Green) };
	
		window.draw(box, 5, sf::LineStrip);

		if (node.isLeaf())
			return;

		int c = node.children;
		for (int i = 0; i < 4; i++)
		{
			draw(window, c);
			c = nodes[c].next;
		}
	}
};
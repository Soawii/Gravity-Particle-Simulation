#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>

namespace Screen
{
	int WIDTH = 1200, HEIGHT = 800;
	float X = WIDTH, Y = HEIGHT;
	sf::Vector2f TOP_LEFT(0, 0), BOTTOM_RIGHT(X, Y);

	const sf::Color BACKGROUD_COLOR = sf::Color::Black;
	const sf::Color BODY_COLOR = sf::Color::White;
	sf::RenderWindow window;

	sf::Vector2f convertMetersToPixels(sf::Vector2f xy)
	{
		return sf::Vector2f(xy.x / X * WIDTH, xy.y / Y * HEIGHT);
	}

	sf::Vector2f convertPixelsToMeters(sf::Vector2f wh)
	{
		return sf::Vector2f(wh.x / WIDTH * X, wh.y / HEIGHT * Y);
	}
	
	void updateParameters()
	{
		WIDTH = window.getSize().x;
		HEIGHT = window.getSize().y;
		X = window.getView().getSize().x;
		Y = window.getView().getSize().y;
		TOP_LEFT = sf::Vector2f(window.getView().getCenter().x - X / 2.0f, window.getView().getCenter().y - Y / 2.0f);
		BOTTOM_RIGHT = sf::Vector2f(window.getView().getCenter().x + X / 2.0f, window.getView().getCenter().y + Y / 2.0f);
	}

	void screenChanged()
	{
		sf::Vector2f size(window.getSize().x, window.getSize().y);
		float dw = 1.0f * size.x / WIDTH, dh = 1.0f * size.y / HEIGHT;
		sf::View view = window.getView();
		sf::Vector2f c = view.getCenter();
		sf::Vector2f view_size = view.getSize();
		sf::Vector2f top_left(c.x - view_size.x / 2.0f, c.y - view_size.y / 2.0f);
		
		window.setView(sf::View(sf::FloatRect(top_left.x - top_left.x * (1 - dw) / 2.0f,
			top_left.y - top_left.y * (1 - dh) / 2.0f,
			view_size.x * dw, view_size.y * dh)));
		updateParameters();
	}

	void zoomIn(float coeff, sf::Vector2i mouse_pos)
	{
		sf::View view = window.getView();
		sf::Vector2f new_top_left = view.getCenter() - view.getSize() / 2.0f, new_bottom_right = view.getCenter() + view.getSize() / 2.0f;
		new_top_left.x += (1.0f * mouse_pos.x / WIDTH) * coeff * Screen::X;
		new_top_left.y += (1.0f * mouse_pos.y / HEIGHT) * coeff * Screen::Y;

		new_bottom_right.x -= (1 - 1.0f * mouse_pos.x / WIDTH) * coeff * Screen::X;
		new_bottom_right.y -= (1 - 1.0f * mouse_pos.y / HEIGHT) * coeff * Screen::Y;
		window.setView(sf::View(sf::FloatRect(new_top_left, new_bottom_right - new_top_left)));
		updateParameters();
	}

	void move(sf::Vector2i pixel_delta)
	{
		sf::View view = window.getView();
		view.move(convertPixelsToMeters(sf::Vector2f(-pixel_delta.x, -pixel_delta.y)));
		window.setView(view);
		updateParameters();
	}

	sf::Vector2f getScale()
	{
		return sf::Vector2f(X, Y);
	}
}

namespace Constants
{
	const float G = 6.67e-11f;
	const int FPS = 60;
	const float dt = 0.005f;
	const float PI = 3.1415;

	float CURRENT_FPS = 0.0f;
	std::string mode = "CIRCLE";
}
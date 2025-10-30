#pragma once
#include "Buttons.h"
#include "BodySimulation.h"
#include "Body.h"

class Menu
{
public:
	Buttons::ButtonHandler handler;
	BodySimulation& sim;
	sf::Font font;

	Menu(BodySimulation& sim_) : sim(sim_)
	{
		using namespace Buttons;
		font.loadFromFile("../resources/font.ttf");
		font.setSmooth(true);
		const std::string prefix = "";
		const int FONT_SIZE = 20;
		const float SPACE = 5.0f;
		
		InteractableLabel* LABEL_fps = new InteractableLabel({ 0, 0 }, { 1000, 1000 }, sf::Text("FPS: 1234567890", font, FONT_SIZE), false, 1);
		LABEL_fps->setOnAction([LABEL_fps, this]()
			{
				std::string s = "FPS: " + std::to_string((int)round(Constants::CURRENT_FPS));
				if (LABEL_fps->getString() != s)
					LABEL_fps->setString(s);
			});
		LABEL_fps->fixPoint(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(20.0f, 20.0f));


		InteractableLabel* LABEL_scale = new InteractableLabel({ 0, 0 }, { 1000, 1000 }, sf::Text(prefix + "X SCALE : " + std::to_string(Screen::X) + "\n" + prefix + "Y SCALE : " + std::to_string(Screen::Y), font, FONT_SIZE), false, 1);
		LABEL_scale->setOnAction([LABEL_scale, this, prefix]()
			{
				std::string s = prefix + "X SCALE : " + std::to_string(Screen::X) + "\n" + prefix + "Y SCALE : " + std::to_string(Screen::Y);
				if (LABEL_scale->getString() != s)
					LABEL_scale->setString(s);
			});
		LABEL_scale->fixPoint(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(20.0f, LABEL_fps->getPosition().y + LABEL_fps->getSize().y + SPACE));


		InteractableLabel* LABEL_BodyAmount = new InteractableLabel({ 0, 0 }, { 1000, 1000 }, sf::Text(prefix + "N : 1234567890", font, FONT_SIZE), false, 1);
		LABEL_BodyAmount->setOnAction([LABEL_BodyAmount, this, prefix]()
			{
				std::string body_amount = prefix + "N: " + std::to_string(this->sim.bodies.size());
				if (LABEL_BodyAmount->getString() != body_amount)
					LABEL_BodyAmount->setString(body_amount);
			});
		LABEL_BodyAmount->fixPoint(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(20.0f, LABEL_scale->getPosition().y + LABEL_scale->getSize().y + SPACE));


		CheckBox* CHECKBOX_quadtree = new CheckBox(
			new RoundButtonShape(
				sf::Vector2f(20.0f, LABEL_BodyAmount->getPosition().y + LABEL_BodyAmount->getSize().y + SPACE),
				sf::Vector2f(30, 30), sf::Text(), false,
				{ sf::Color(100, 100, 100), sf::Color(140, 140, 140), sf::Color(180, 180, 180), sf::Color(220, 220, 220) }, 8.0f),
			sf::Text("QuadTree", font, FONT_SIZE), sf::Vector2f(1000, 1000), false, 5.0f, 1);
		CHECKBOX_quadtree->setOnAction([CHECKBOX_quadtree, this]() {
			this->sim.showQuadTree = CHECKBOX_quadtree->checkBox.isPressed();
			});


		int minSize = 1, maxSize = 30;
		Slider* SLIDER_maxleafsize = new Slider(
			new SliderShape(
				sf::Vector2f(20.0f, CHECKBOX_quadtree->checkBox.shape->getPosition().y + CHECKBOX_quadtree->checkBox.shape->getSize().y + FONT_SIZE + SPACE),
				sf::Vector2f(120, 30), sf::Text("Max. Leaf Size: " + std::to_string(sim.bh.maxLeafSize), font, FONT_SIZE), sf::Vector2f(120, FONT_SIZE - 5), true, 3.0f,
				{ sf::Color(100, 100, 100), sf::Color(140, 140, 140), sf::Color(180, 180, 180) },
				{ sf::Color(220, 220, 220), sf::Color(220, 220, 220), sf::Color(220, 220, 220) }),
			0.5f, 1);
			
		SLIDER_maxleafsize->setOnAction([SLIDER_maxleafsize, this, minSize, maxSize]() {
			int newLeafSize = minSize + (SLIDER_maxleafsize->point * (maxSize - minSize));
			if (newLeafSize != this->sim.bh.maxLeafSize)
			{
				this->sim.bh.maxLeafSize = newLeafSize;
				this->sim.bh.head.maxLeafSize = newLeafSize;
				SLIDER_maxleafsize->shape->label.setString("Max. Leaf Size: " + std::to_string(newLeafSize));
			}
			});


		float minThreshold = 0.1f, maxThreshold = 1.5f;
		Slider* SLIDER_threshold = new Slider(
			new SliderShape(
				sf::Vector2f(20.0f, SLIDER_maxleafsize->shape->getPosition().y + SLIDER_maxleafsize->shape->getSize().y + FONT_SIZE + SPACE),
				sf::Vector2f(120, 30), sf::Text("Threshold: " + std::to_string(sim.bh.threshold), font, FONT_SIZE), sf::Vector2f(120, FONT_SIZE - 5), true, 3.0f,
				{ sf::Color(100, 100, 100), sf::Color(140, 140, 140), sf::Color(180, 180, 180) },
				{ sf::Color(220, 220, 220), sf::Color(220, 220, 220), sf::Color(220, 220, 220) }),
			0.5f, 1);

		SLIDER_threshold->setOnAction([SLIDER_threshold, this, minThreshold, maxThreshold]() {
			float newThreshold = minThreshold + (SLIDER_threshold->point * (maxThreshold - minThreshold));
			if (newThreshold != this->sim.bh.threshold)
			{
				this->sim.bh.threshold = newThreshold;
				SLIDER_threshold->shape->label.setString("Threshold: " + std::to_string(newThreshold));
			}
			});


		int minCollision = 0, maxCollision = 10;
		Slider* SLIDER_collision = new Slider(
			new SliderShape(
				sf::Vector2f(20.0f, SLIDER_threshold->shape->getPosition().y + SLIDER_threshold->shape->getSize().y + FONT_SIZE + SPACE),
				sf::Vector2f(120, 30), sf::Text("Collision precision: " + std::to_string(sim.collisionPrecision), font, FONT_SIZE), sf::Vector2f(120, FONT_SIZE), true, 3.0f,
				{ sf::Color(100, 100, 100), sf::Color(140, 140, 140), sf::Color(180, 180, 180) },
				{ sf::Color(220, 220, 220), sf::Color(220, 220, 220), sf::Color(220, 220, 220) }),
			0.5f, 1);

		SLIDER_collision->setOnAction([SLIDER_collision, this, minCollision, maxCollision]() {
			int newCollision = minCollision + (SLIDER_collision->point * (maxCollision - minCollision));
			if (newCollision != this->sim.collisionPrecision)
			{
				this->sim.collisionPrecision = newCollision;
				SLIDER_collision->shape->label.setString("Collision precision: " + std::to_string(newCollision));
			}
			});


		SwitchableButtonGroup* spawnGroup = new SwitchableButtonGroup(1);

		sf::Text text = sf::Text("CIRCLE", font, 14);
		text.setOutlineThickness(2.0f);

		SwitchableButton* circleSpawnButton = new SwitchableButton(
			new RoundButtonShape(
				sf::Vector2f(20.0f, SLIDER_collision->shape->getPosition().y + SLIDER_collision->shape->getSize().y + SPACE + 10.0f),
				sf::Vector2f(55, 40), text, false,
				{ sf::Color(255, 165, 0), sf::Color(255, 195, 0), sf::Color(255, 220, 0), sf::Color(255, 255, 0) }, 10.0f),
			1);

		text = sf::Text("WALL", font, 14);
		text.setOutlineThickness(2.0f);

		SwitchableButton* wallSpawnButton = new SwitchableButton(
			new RoundButtonShape(
				sf::Vector2f(circleSpawnButton->shape->getPosition().x + circleSpawnButton->shape->getSize().x + 10.0f, SLIDER_collision->shape->getPosition().y + SLIDER_collision->shape->getSize().y + SPACE + 10.0f),
				sf::Vector2f(55, 40), text, false,
				{ sf::Color(255, 165, 0), sf::Color(255, 195, 0), sf::Color(255, 220, 0), sf::Color(255, 255, 0) }, 10.0f),
			1);

		circleSpawnButton->setOnAction([circleSpawnButton]() {if (circleSpawnButton->isPressed()) Constants::mode = "CIRCLE"; });
		wallSpawnButton->setOnAction([wallSpawnButton]() {if (wallSpawnButton->isPressed()) Constants::mode = "WALL"; });
		spawnGroup->addSwitchable(circleSpawnButton);
		spawnGroup->addSwitchable(wallSpawnButton);

		PrioritableLabel* LABEL_info = new PrioritableLabel({ 0, 0 }, { 1000, 1000 }, sf::Text("M1 - move\nM2 - spawn projectile\nM3 - spawn group", font, FONT_SIZE - 4), false, 1);
		LABEL_info->fixPoint(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(20.0f, circleSpawnButton->shape->getPosition().y + circleSpawnButton->shape->getSize().y + SPACE + 10.0f));


		handler.addItem(LABEL_fps);
		handler.addItem(LABEL_BodyAmount);
		handler.addItem(LABEL_scale);
		handler.addItem(CHECKBOX_quadtree);
		handler.addItem(SLIDER_maxleafsize);
		handler.addItem(SLIDER_threshold);
		handler.addItem(SLIDER_collision);
		handler.addItem(spawnGroup);
		handler.addItem(LABEL_info);
	}
};
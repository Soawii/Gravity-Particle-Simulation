#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <algorithm>
#include <exception>

namespace Buttons
{
	sf::Font DEFAULT_FONT;
	
	enum class ButtonState
	{
		NORMAL = 0,
		HOVER = 1,
		BEING_PRESSED = 2,
		PRESSED = 3
	};

	enum class FocusableState
	{
		NORMAL = 0,
		HOVER = 1,
		FOCUSED = 2
	};

	class Prioritable : virtual public sf::Drawable
	{
	protected:
		int priority;
	public:
		Prioritable(int priority_) : priority(priority_) {}
		int getPriority() const
		{
			return priority;
		}
	};


	class Interactable : public Prioritable
	{
	protected:
		bool interacted_this_frame = false;
		std::function<void()> onAction = []() {};
	public:
		Interactable(int priority_) : Prioritable(priority_) {}

		virtual void processEvent(sf::Event& event) = 0;
		virtual void update(sf::Vector2i mouse_pos) = 0;

		virtual void setOnAction(std::function<void()> onAction_)
		{
			onAction = onAction_;
		}

		void activateOnAction()
		{
			onAction();
		}

		bool wasInteractedThisFrame() const
		{
			return interacted_this_frame;
		}
	};


	class Pressable : public Interactable
	{
	protected:
		ButtonState state;
		bool is_pressed;
	public:
		Pressable(int priority_, bool is_pressed_) : Interactable(priority_), is_pressed(is_pressed_) {}

		virtual void press() = 0;

		bool isPressed() const
		{
			return is_pressed;
		}

		ButtonState getState() const
		{
			return state;
		}
	};

	class Focusable : public Interactable
	{
	protected:
		FocusableState state;
		bool is_focused;
	public:
		Focusable(int priority_, bool is_focused_) : Interactable(priority_), is_focused(is_focused_) {}

		virtual void focus()
		{
			is_focused = true;
			state = FocusableState::FOCUSED;
		}

		virtual void unfocus()
		{
			is_focused = false;
			state = FocusableState::NORMAL;
		}

		bool isFocused() const
		{
			return is_focused;
		}

		FocusableState getState() const
		{
			return state;
		}
	};


	class Label : virtual public sf::Drawable, public sf::Transformable
	{
	protected:
		sf::Vector2f topLeft;
		sf::Vector2f maxSize;
		sf::Text text;

		bool isPointFixed = false, fillSpace = false;
		sf::Vector2f fixedPoint, fixedPosition;

	public:
		Label(sf::Vector2f topLeft_, sf::Vector2f maxSize_, sf::Text text_, bool fillSpace_)
			: topLeft(topLeft_), maxSize(maxSize_), fillSpace(fillSpace_)
		{
			setText(text_);
		}

		void checkBounds()
		{
			if (fillSpace)
			{
				while (getSize().x < maxSize.x && getSize().y < maxSize.y)
					text.setCharacterSize(text.getCharacterSize() + 1);
			}
			while (getSize().x > maxSize.x || getSize().y > maxSize.y)
				text.setCharacterSize(text.getCharacterSize() - 1);

			if (isPointFixed)
			{
				setPositionAt(fixedPoint, fixedPosition);
			}
		}

		void setFillSpace(bool fillSpace_)
		{
			fillSpace = fillSpace_;
		}

		void setText(sf::Text text_)
		{
			text = text_;
			checkBounds();
		}

		void setString(std::string s)
		{
			text.setString(s);
			checkBounds();
		}

		std::string getString() const
		{
			return text.getString();
		}

		void setMaxSize(sf::Vector2f maxSize_)
		{
			maxSize.x = maxSize_.x;
			maxSize.y = maxSize_.y;
			checkBounds();
		}

		void fixPoint(sf::Vector2f point, sf::Vector2f position)
		{
			isPointFixed = true;
			fixedPoint = point;
			fixedPosition = position;
			checkBounds();
		}

		sf::Vector2f getSize() const
		{
			return sf::Vector2f(text.getLocalBounds().left + text.getLocalBounds().width, text.getLocalBounds().top + text.getLocalBounds().height);
		}

		sf::Vector2f getPositionAt(sf::Vector2f point) const
		{
			return sf::Vector2f(
				text.getLocalBounds().left + text.getLocalBounds().width * point.x,
				text.getLocalBounds().top + text.getLocalBounds().height * point.y);
		}

		void setPositionAt(sf::Vector2f point, sf::Vector2f position)
		{
			setOrigin(getPositionAt(point));
			setPosition(position);
		}

		void setCenter(sf::Vector2f center)
		{
			setPositionAt({ 0.5f, 0.5f }, center);
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			states.transform.combine(getTransform());
			target.draw(text, states);
		}
	};


	class PrioritableLabel : virtual public Label, virtual public Prioritable
	{
	public:
		PrioritableLabel(sf::Vector2f topLeft_, sf::Vector2f maxSize_, sf::Text text_, bool fillSpace, int priority_)
			: Label(topLeft_, maxSize_, text_, fillSpace), Prioritable(priority_) {}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			Label::draw(target, states);
		}
	};


	class InteractableLabel : virtual public Label, virtual public Interactable
	{
	public:
		InteractableLabel(sf::Vector2f topLeft_, sf::Vector2f maxSize_, sf::Text text_, bool fillSpace, int priority_)
			: Label(topLeft_, maxSize_, text_, fillSpace), Interactable(priority_) {}

		void processEvent(sf::Event& event) override {}

		void update(sf::Vector2i mouse_pos) override
		{
			activateOnAction();
		}

		void setOnAction(std::function<void()> onAction_) override
		{
			onAction = onAction_;
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			Label::draw(target, states);
		}
	};


	class Shape : public sf::Drawable, public sf::Transformable
	{
	protected:
		sf::Color activeColor = sf::Color::White;
		std::vector<sf::Color> activeColors;
		sf::Vector2f size;
	public:
		Shape(sf::Vector2f topLeft_, sf::Vector2f size_, std::vector<sf::Color> activeColors_) : activeColors(activeColors_), size(size_)
		{
			setPosition(topLeft_);
			activeColor = activeColors[0];
		}

		virtual bool isHovering(sf::Vector2f mouse_pos) const = 0;

		virtual void updateColors() = 0;

		virtual void setActiveColorToState(size_t state)
		{
			activeColor = activeColors[state];
		}

		void setActiveColorOfState(size_t state, sf::Color color)
		{
			activeColors[state] = color;
		}

		void setActiveColors(std::vector<sf::Color> colors)
		{
			if (activeColors.size() != colors.size())
			{
				std::cout << "active colors size != colors size @ [setActiveColors]\n";
				return;
			}
			activeColors = colors;
		}

		sf::Vector2f getSize() const
		{
			return size;
		}
	};

	class ButtonShape : public Shape
	{
	public:
		Label label;

		ButtonShape(sf::Vector2f topLeft_, sf::Vector2f size_, sf::Text text_, bool fillSpace, std::vector<sf::Color> colors)
			: Shape(topLeft_, size_, colors),
				label(sf::Vector2f(0, 0), size_, text_, fillSpace)
		{
			label.setCenter(size_ / 2.0f);
		}

		virtual void update(Pressable* pressable, sf::Vector2i mouse_pos)
		{
			sf::Vector2f mouse_pos_f(mouse_pos.x, mouse_pos.y);
			if (pressable->getState() == ButtonState::BEING_PRESSED)
			{
				if (isHovering(mouse_pos_f))
					setActiveColorToState((size_t)ButtonState::BEING_PRESSED);
				else
					setActiveColorToState(pressable->isPressed() ? (size_t)ButtonState::PRESSED : (size_t)ButtonState::NORMAL);
			}
			else
				setActiveColorToState((size_t)pressable->getState());
		}
	};


	class RectButtonShape : public ButtonShape
	{
	public:
		sf::RectangleShape shape;

		RectButtonShape(sf::Vector2f topLeft_, sf::Vector2f size_, sf::Text text_, bool fillSpace, std::vector<sf::Color> colors)
			: ButtonShape(topLeft_, size_, text_, fillSpace, colors)
		{
			shape.setSize(size_);
		}

		bool isHovering(sf::Vector2f point) const override
		{
			sf::Transform inv_trasform = getInverseTransform();
			sf::Vector2f point_as_local = inv_trasform.transformPoint(point);
			return shape.getLocalBounds().contains(point_as_local);
		}

		virtual void updateColors() override
		{
			shape.setFillColor(activeColor);
		}

		virtual void update(Pressable* pressable, sf::Vector2i mouse_pos)
		{
			ButtonShape::update(pressable, mouse_pos);
			updateColors();
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			states.transform.combine(getTransform());
			target.draw(shape, getTransform());
			target.draw(label, getTransform());
		}
	};


	class RoundButtonShape : public ButtonShape
	{
	public:
		sf::RectangleShape horizontalRect, verticalRect;
		sf::CircleShape circles[4];
		float radius;

		RoundButtonShape(sf::Vector2f topLeft_, sf::Vector2f size_, sf::Text text_, bool fillSpace, std::vector<sf::Color> colors, float radius_)
			: ButtonShape(topLeft_, size_, text_, fillSpace, colors), radius(radius_)
		{
			radius = std::min(radius, size.y / 2.0f);
			for (int i = 0; i < 4; i++)
			{
				circles[i].setRadius(radius);
				circles[i].setOrigin(radius, radius);
			}
			circles[0].setPosition(sf::Vector2f(radius, radius));
			circles[1].setPosition(sf::Vector2f(size.x - radius, radius));
			circles[2].setPosition(sf::Vector2f(radius, size.y - radius));
			circles[3].setPosition(sf::Vector2f(size.x - radius, size.y - radius));

			horizontalRect.setSize(sf::Vector2f(size.x, size.y - radius * 2));
			verticalRect.setSize(sf::Vector2f(size.x - radius * 2, size.y));
			horizontalRect.setPosition(sf::Vector2f(0, radius));
			verticalRect.setPosition(sf::Vector2f(radius, 0));
		}

		virtual void updateColors() override
		{
			horizontalRect.setFillColor(activeColor);
			verticalRect.setFillColor(activeColor);
			for (int i = 0; i < 4; i++)
				circles[i].setFillColor(activeColor);
		}

		virtual void update(Pressable* pressable, sf::Vector2i mouse_pos) override
		{
			ButtonShape::update(pressable, mouse_pos);
			updateColors();
		}

		bool isHovering(sf::Vector2f point) const override
		{
			sf::Transform inv_transform = getInverseTransform();
			sf::Vector2f point_as_local = inv_transform.transformPoint(point);
			if (horizontalRect.getLocalBounds().contains(point_as_local) || verticalRect.getLocalBounds().contains(point_as_local))
				return true;
			for (int i = 0; i < 4; i++)
			{
				if (sqrt((circles[i].getPosition().x - point_as_local.x) * (circles[i].getPosition().x - point_as_local.x) + (circles[i].getPosition().y - point_as_local.y) * (circles[i].getPosition().y - point_as_local.y)) <= radius)
					return true;
			}
			return false;
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			states.transform.combine(getTransform());

			target.draw(horizontalRect, states);
			target.draw(verticalRect, states);

			for (int i = 0; i < 4; i++)
				target.draw(circles[i], states);
			target.draw(label, states);
		}
	};


	class PressableButton : public Pressable
	{
	public:
		ButtonShape* shape;
		PressableButton(ButtonShape* shape_, int priority_) : Pressable(priority_, false), shape(shape_) {}

		virtual void processEvent(sf::Event& event) override
		{
			interacted_this_frame = false;
			if (event.type == sf::Event::MouseButtonPressed)
			{
				sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
				if (event.mouseButton.button == sf::Mouse::Left && shape->isHovering(mousePos))
				{
					state = ButtonState::BEING_PRESSED;
					interacted_this_frame = true;
				}
			}
			else if (event.type == sf::Event::MouseButtonReleased)
			{
				sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					if (state == ButtonState::BEING_PRESSED)
					{
						if (shape->isHovering(mousePos))
						{
							press();
							interacted_this_frame = true;
						}
						else
						{
							state = isPressed() ? ButtonState::PRESSED : ButtonState::NORMAL;
						}
					}
				}
			}
		}

		virtual void update(sf::Vector2i mouse_pos) override
		{
			sf::Vector2f mouse_pos_f(mouse_pos.x, mouse_pos.y);

			switch (state)
			{
			case ButtonState::NORMAL:
				if (shape->isHovering(mouse_pos_f))
					state = ButtonState::HOVER;
				break;
			case ButtonState::HOVER:
				if (!shape->isHovering(mouse_pos_f))
					state = ButtonState::NORMAL;
				break;
			case ButtonState::BEING_PRESSED:
				break;
			case ButtonState::PRESSED:
				break;
			}

			shape->update(this, mouse_pos);
		}

		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			target.draw(*shape, states);
		}

		~PressableButton()
		{
			delete shape;
		}
	};


	class ClickableButton : public PressableButton
	{
	public:
		ClickableButton(ButtonShape* button_, int priority_)
			: PressableButton(button_, priority_) {}

		void press() override
		{
			state = ButtonState::PRESSED;
			is_pressed = true;
			activateOnAction();
			is_pressed = false;
			state = ButtonState::HOVER;
		}
	};


	class SwitchableButton : public PressableButton
	{
	public:
		SwitchableButton(ButtonShape* button_, int priority_)
			: PressableButton(button_, priority_) {}

		void press() override
		{
			if (is_pressed)
				state = ButtonState::HOVER;
			else 
				state = ButtonState::PRESSED;
			is_pressed = !is_pressed;
			activateOnAction();
		}
	};


	class CheckBox : public Interactable
	{
	public:
		SwitchableButton checkBox;
		Label label;

		CheckBox(ButtonShape* checkBoxShape_, 
			sf::Text text_, sf::Vector2f labelSize_, bool fillSpace_, float spaceBetween_, int priority_)
			: 
			checkBox(checkBoxShape_, priority_),
			label(sf::Vector2f(0, 0), labelSize_, text_, fillSpace_),
			Interactable(priority_)
		{
			label.fixPoint(sf::Vector2f(0.0f, 0.5f), sf::Vector2f(checkBoxShape_->getPosition().x + checkBoxShape_->getSize().x + spaceBetween_, checkBoxShape_->getPosition().y + checkBoxShape_->getSize().y / 2.0f));
		}

		void setOnAction(std::function<void()> onAction_) override
		{
			checkBox.setOnAction(onAction_);
		}
		
		void processEvent(sf::Event& event) override
		{
			checkBox.processEvent(event);
			interacted_this_frame = checkBox.wasInteractedThisFrame();
		}

		void update(sf::Vector2i mouse_pos) override
		{
			checkBox.update(mouse_pos);
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			target.draw(checkBox, states);
			target.draw(label, states);
		}
	};


	class SliderShape : public Shape
	{
	public:
		sf::CircleShape sliderShape;
		RoundButtonShape lineShape;
		Label label;
		float radius;

		SliderShape(sf::Vector2f topLeft_, sf::Vector2f size_, 
			sf::Text text_, sf::Vector2f labelSize_, bool fillSpace, float spaceBetween_,
			std::vector<sf::Color> sliderColors_, std::vector<sf::Color> lineColors_)
			:
			Shape(topLeft_, size_, sliderColors_), 
			label(sf::Vector2f(0, 0), labelSize_, text_, fillSpace),
			radius(size_.y / 2.0f), lineShape(sf::Vector2f(0, 0), size_, sf::Text(), false, lineColors_, size_.y / 2.0f)
		{
			lineShape.setActiveColors(lineColors_);
			sliderShape.setFillColor(activeColor);

			label.fixPoint(sf::Vector2f(0.5f, 1.0f), sf::Vector2f(size_.x / 2.0f, -spaceBetween_));

			sliderShape.setRadius(radius);
			sliderShape.setOrigin(radius, 0);
			sliderShape.setPointCount(30);
			updateSlider(0.0f);
		}

		virtual bool isHovering(sf::Vector2f mouse_pos) const override
		{
			sf::Transform inv_transform = getInverseTransform();
			return lineShape.isHovering(inv_transform.transformPoint(mouse_pos));
		}

		virtual void update(Focusable* focusable)
		{
			setActiveColorToState((size_t)focusable->getState());
			lineShape.setActiveColorToState((size_t)focusable->getState());
		}

		void updateSlider(float current_point)
		{
			sliderShape.setPosition((size.x - radius * 2) * current_point + radius, 0);
		}

		void updateColors() override
		{
			lineShape.updateColors();
			sliderShape.setFillColor(activeColor);
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			states.transform.combine(getTransform());
			target.draw(lineShape, states);
			target.draw(sliderShape, states);
			target.draw(label, states);
		}
	};


	class Slider : public Focusable
	{
	public:
		SliderShape* shape;
		float point;

		Slider(SliderShape* shape_, float starting_point_, int priority_)
			: point(starting_point_), shape(shape_), Focusable(priority_, false)
		{
			state = FocusableState::NORMAL;
			shape->updateSlider(point);
		}

		void processEvent(sf::Event& event) override
		{
			if (event.type == sf::Event::MouseButtonPressed)
			{
				sf::Vector2f mouse_pos_f(event.mouseButton.x, event.mouseButton.y);
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					if (shape->isHovering(mouse_pos_f))
					{
						focus();
					}
				}
			}
			else if (event.type == sf::Event::MouseButtonReleased)
			{
				sf::Vector2f mouse_pos_f(event.mouseButton.x, event.mouseButton.y);
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					unfocus();
				}
			}
			interacted_this_frame = isFocused();
		}

		void update(sf::Vector2i mouse_pos) override
		{
			sf::Vector2f mouse_pos_f(mouse_pos.x, mouse_pos.y);

			bool is_hovering = shape->isHovering(mouse_pos_f);

			switch (state)
			{
			case FocusableState::NORMAL:
				if (is_hovering)
					state = FocusableState::HOVER;
				break;
			case FocusableState::HOVER:
				if (!is_hovering)
					state = FocusableState::NORMAL;
				break;
			case FocusableState::FOCUSED:
				break;
			}
			
			shape->update(this);
			shape->updateColors();
			if (isFocused())
			{
				float new_point = (mouse_pos_f.x - (shape->getPosition() - shape->getOrigin()).x - shape->sliderShape.getRadius()) / (shape->getSize().x - shape->sliderShape.getRadius() * 2);
				new_point = std::min(new_point, 1.0f);
				new_point = std::max(new_point, 0.0f);
				if (point != new_point)
				{
					point = new_point;
					shape->updateSlider(point);
					activateOnAction();
				}
			}
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			target.draw(*shape, states);
		}

		~Slider()
		{
			delete shape;
		}
	};


	class SwitchableButtonGroup : public Interactable
	{
	public:
		std::vector<SwitchableButton*> switchables;

		SwitchableButtonGroup(int priority_) : Interactable(priority_) {}

		void addSwitchable(SwitchableButton* button)
		{
			switchables.push_back(button);
		}

		virtual void processEvent(sf::Event& event)
		{
			interacted_this_frame = false;
			for (int i = 0; i < switchables.size(); i++)
			{
				if (switchables[i]->isPressed()) 
					continue;
				switchables[i]->processEvent(event);
				if (switchables[i]->isPressed())
				{
					for (int j = 0; j < switchables.size(); j++)
					{
						if (i == j)
							continue;
						if (switchables[j]->isPressed())
							switchables[j]->press();
					}
				}
				interacted_this_frame = interacted_this_frame || switchables[i]->wasInteractedThisFrame();
			}
		}

		virtual void update(sf::Vector2i mouse_pos)
		{
			for (int i = 0; i < switchables.size(); i++)
			{
				switchables[i]->update(mouse_pos);
			}
		}

		bool wasInteractedThisFrame() const
		{
			return interacted_this_frame;
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			for (int i = 0; i < switchables.size(); i++)
			{
				target.draw(*switchables[i], states);
			}
		}
	};

	
	class ButtonHandler : public sf::Drawable
	{
	public:
		std::vector<Prioritable*> Items;
		std::vector<Interactable*> Interactables;
		std::vector<Focusable*> Focusables;
		Focusable* focused_item = nullptr;

		ButtonHandler() {}

		void addItem(Focusable* focusable)
		{
			Items.push_back(focusable);
			Focusables.push_back(focusable);

			bool(*pred)(Prioritable*, Prioritable*) = [](Prioritable* l, Prioritable* r) { return l->getPriority() < r->getPriority(); };
			std::sort(Items.begin(), Items.end(), pred);
			std::sort(Focusables.begin(), Focusables.end(), pred);
		}

		void addItem(Interactable* interactable)
		{
			Items.push_back(interactable);
			Interactables.push_back(interactable);

			bool(*pred)(Prioritable*, Prioritable*) = [](Prioritable* l, Prioritable* r) { return l->getPriority() < r->getPriority(); };
			std::sort(Items.begin(), Items.end(), pred);
			std::sort(Interactables.begin(), Interactables.end(), pred);
		}

		void addItem(Prioritable* prioritable)
		{
			Items.push_back(prioritable);
			bool(*pred)(Prioritable*, Prioritable*) = [](Prioritable* l, Prioritable* r) { return l->getPriority() < r->getPriority(); };
			std::sort(Items.begin(), Items.end(), pred);
		}

		bool processEvent(sf::Event& event)
		{
			if (focused_item != nullptr)
			{
				focused_item->processEvent(event);
				if (!focused_item->wasInteractedThisFrame())
					focused_item = nullptr;
				return true;
			}

			int l = int(Interactables.size()) - 1, r = int(Focusables.size()) - 1;
			while (l >= 0 && r >= 0)
			{
				if (Interactables[l]->getPriority() <= Focusables[r]->getPriority())
				{
					Focusables[r]->processEvent(event);
					if (Focusables[r]->wasInteractedThisFrame())
					{
						focused_item = Focusables[r];
						return true;
					}
					r--;
				}
				else
				{
					Interactables[l]->processEvent(event);
					if (Interactables[l]->wasInteractedThisFrame())
					{
						return true;
					}
					l--;
				}
			}
			while (l >= 0)
			{
				Interactables[l]->processEvent(event);
				if (Interactables[l]->wasInteractedThisFrame())
				{
					return true;
				}
				l--;
			}
			while (r >= 0)
			{
				Focusables[r]->processEvent(event);
				if (Focusables[r]->wasInteractedThisFrame())
				{
					focused_item = Focusables[r];
					return true;
				}
				r--;
			}
			return false;
		}

		void update(sf::Vector2i mouse_pos)
		{
			for (int i = 0; i < Interactables.size(); i++)
				Interactables[i]->update(mouse_pos);
			for (int i = 0; i < Focusables.size(); i++)
				Focusables[i]->update(mouse_pos);
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			sf::View view = target.getView();
			sf::Vector2u targetSize(target.getSize());
			target.setView(sf::View(sf::FloatRect(0, 0, targetSize.x, targetSize.y)));

			for (int i = 0; i < Items.size(); i++)
			{
				target.draw(*Items[i]);
			}

			target.setView(view);
		}

		~ButtonHandler()
		{
			for (int i = 0; i < Items.size(); i++)
				delete Items[i];
		}
	};
}
#include "Snake.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <utility>
#include <algorithm>
#include <exception>
#include <cassert>
#include <iostream>

std::shared_ptr<spdlog::logger> const Snake::m_consoleLogger{ spdlog::stderr_color_mt("Snake") };

constexpr auto Snake::changeToPlayFieldAwarePosition(Snake::Point& point) -> void
{
	if (point.second /* y */ < m_gameFrameDimension.first.second /* top y value */)
	{
		point.second += m_gameFrameDimension.second.second + 1 /* bottom y value */;
	}
	else if (point.second /* y */ > m_gameFrameDimension.second.second /* bottom y value */)
	{
		point.second -= m_gameFrameDimension.second.second + 1 /* bottom y value */;
	}
	else if (point.first /* x */ < m_gameFrameDimension.first.first /* left x value */)
	{
		point.first += m_gameFrameDimension.second.first + 1 /* left x value */;
	}
	else if (point.first /* x */ > m_gameFrameDimension.second.first /* left x value */)
	{
		point.first -= m_gameFrameDimension.second.first + 1 /* right x value */;
	}
}

Snake::Snake(const std::pair<Point, Point>& gameFrameDimension,
	const Point& start, Direction direction, std::int32_t length, std::int32_t speedUpCountDown)
	: m_gameFrameDimension(gameFrameDimension), m_length(length), m_viewDirection(direction),
	  m_tailDirection(Snake::getOpposite(direction)), m_movementSpeed(3), m_speedUpCountDown(speedUpCountDown), m_level(1)
{
	// Initialization outside of the game frame
	if (start.first < gameFrameDimension.first.first || gameFrameDimension.second.first < start.first)
	{
		m_consoleLogger->error("The x value to spawn the snake {:d} is out of the game frame size ({:d},{:d})", start
			.first, gameFrameDimension.first.first, gameFrameDimension.second.first);
		throw std::runtime_error{ "x value out of game frame range" };
	}
	else if (start.second < gameFrameDimension.first.second || gameFrameDimension.second.second < start.second)
	{
		m_consoleLogger->error("The y value to spawn the snake {:d} is out of the game frame size ({:d},{:d})", start
			.second, gameFrameDimension.first.second, gameFrameDimension.second.second);
		throw std::runtime_error{ "x value out of game frame range" };
	}

	// Setup of body & head
	m_body = { start };

	Point bodyPoint{ start };
	for (int i = 1; i < length; ++i)
	{
		// Makes the snake "grow" in the opposite direction of the initial view direction
		// buffer because operator+= & operator- not overwritten
		Point buffer{ getDirectionsRelativeCoords(direction) };
		bodyPoint.first += -buffer.first;
		bodyPoint.second += -buffer.second;
		m_body.push_back(bodyPoint);
	}

	// Extreme case: The snake spawns on a snack & does not grow because the length updates happening in move()
	std::for_each(m_body.begin(), m_body.end(), [this](auto& bodyPoint)
	{
	  Snake::changeToPlayFieldAwarePosition(bodyPoint);
	});

	// Logging
	m_consoleLogger->info("Created from {:d},{:d} to {:d},{:d}:",
		m_body.front().first, m_body.front().second, m_body.back().first, m_body.back().second);

	m_consoleLogger->info(fmt::format("{}", m_body));
}

Snake::Snake(const std::pair<Point, Point>& gameFrameDimension, Direction direction, std::int32_t length)
	: Snake(gameFrameDimension, Point{
	(gameFrameDimension.first.first + gameFrameDimension.second.first) / 2,
	(gameFrameDimension.first.second + gameFrameDimension.second.second) / 2
}, direction, length)
{
}

auto Snake::move(const Point& snackPosition) -> bool
{
	static std::int32_t initSpeedUpCountDown{ m_speedUpCountDown };

	Point buffer{ getDirectionsRelativeCoords(m_viewDirection) };
	Point newHeadPosition{ m_body.front().first + buffer.first, m_body.front().second + buffer.second };

	// Snake is moving about a border
	changeToPlayFieldAwarePosition(newHeadPosition);

	m_body.push_front(newHeadPosition);

	bool ateSnack{ false };
	if (this->isOnSnack(snackPosition))
	{
		std::int32_t new_length = ++m_length;
		m_length = new_length;
		ateSnack = true;

		if (--m_speedUpCountDown == 0)
		{
			m_movementSpeed *= 1.25;
			m_speedUpCountDown = ++initSpeedUpCountDown;

			m_consoleLogger
				->info("It's time for a speedup! Snake now moves every {}s.", static_cast<double>(1) / m_movementSpeed);

			++m_level;
		}

		m_consoleLogger->info("Ate snack on position {}. Adjusting size to {}", m_body.front(), new_length);
	}
	else
	{
		m_body.pop_back();
	}
	m_tailDirection = getOpposite(m_viewDirection);
	return ateSnack;
}

auto Snake::turn(Direction direction) -> void
{
	//assert(direction != m_viewDirection);
	//assert(direction != static_cast<Direction>((directionAsInt + 2) % 4));
	if (m_viewDirection == direction || m_tailDirection == direction)
	{}
	else
	{
		m_viewDirection = direction;
		//m_consoleLogger->info("Turned snake {}", static_cast<int>(direction));
	}
}

auto Snake::isOnSnack(const Point& snackPosition) -> bool
{
	bool onSnack{ false };
	std::for_each(m_body.begin(), m_body.end(), [&](auto& bodyPoint)
	{
	  onSnack += (bodyPoint == snackPosition);
	});

	return onSnack;
}

auto Snake::getBody() -> std::vector<Point>
{
	return { m_body.begin(), m_body.end() };
}

auto Snake::getLength() const -> std::int32_t
{
	return m_length;
}
constexpr auto Snake::getDirectionsRelativeCoords(Direction direction) -> Point
{
	Point relativeCoordinates{};
	switch (direction)
	{
	case Direction::NORTH:
	{
		relativeCoordinates = Point{ 0, -1 };
		break;
	}
	case Direction::EAST:
	{
		relativeCoordinates = Point{ 1, 0 };
		break;
	}
	case Direction::SOUTH:
	{
		relativeCoordinates = Point{ 0, 1 };
		break;
	}
	case Direction::WEST:
	{
		relativeCoordinates = Point{ -1, 0 };
		break;
	}
	}

	return relativeCoordinates;
}

auto Snake::isEatingItself() -> bool
{
	Point& head{ m_body.front() };
	bool eatsItself{ false };
	std::for_each(m_body.cbegin() + 1, m_body.cend(), [&](auto& body_point)
	{
	  eatsItself += body_point == head;
	});

	return eatsItself;
}
auto Snake::turnLeft() -> void
{
	int viewDirectionAsInt{ static_cast<int>(m_viewDirection) };
	viewDirectionAsInt = (viewDirectionAsInt + 3) % 4;
	this->turn(static_cast<Direction>(viewDirectionAsInt));
}
auto Snake::turnRight() -> void
{
	int viewDirectionAsInt{ static_cast<int>(m_viewDirection) };
	viewDirectionAsInt = (viewDirectionAsInt + 1) % 4;
	this->turn(static_cast<Direction>(viewDirectionAsInt));
}
auto Snake::getSpeed() -> double
{
	return m_movementSpeed;
}
constexpr auto Snake::getOpposite(Direction direction) -> Direction
{
	int directionAsInt{ static_cast<int>(direction) };
	return static_cast<Direction>((directionAsInt + 2) % 4);
}

auto Snake::getLevel() -> std::int32_t
{
	return m_level;
}
Snake& Snake::operator=(Snake&& snake) noexcept
{
	// Vater unser im Himmel, geheiligt werde dein Name. Dein Reich komme, Dein Wille geschehe, ...
	// I'm so sorry but I'm too tired for the big 5 / researching how to move semantics with const members :/
	m_body = snake.m_body;
	m_viewDirection = snake.m_viewDirection;
	m_tailDirection = snake.m_tailDirection;
	m_movementSpeed = snake.m_movementSpeed;
	m_speedUpCountDown = snake.m_speedUpCountDown;
	m_length = snake.m_length;
	m_level = snake.m_level;
	m_gameFrameDimension = snake.m_gameFrameDimension; // Had to make this non-const for this operator TODO!

	return *this;
}

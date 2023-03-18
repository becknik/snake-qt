#include "Snake.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <cassert>

std::shared_ptr<spdlog::logger> const Snake::mConsoleLogger{ spdlog::stderr_color_mt("Snake") };

auto Snake::changeToPlayFieldAwarePosition(Snake::Point& point) -> void
{
	auto& [x, y] = point;
	auto& [xGameDimension, yGameDimension] = mGameFrameDimension;

	if (y < 0)
	{
		y += yGameDimension + 1;
	}
	else if (yGameDimension < y)
	{
		y -= yGameDimension + 1;
	}
	else if (x < 0)
	{
		x += xGameDimension + 1;
	}
	else if (xGameDimension < x)
	{
		x -= xGameDimension + 1;
	}
}

Snake::Snake(const Point& gameFrameDimension, const Point& start, Direction direction,
	const std::int32_t length, const std::int32_t speedUpCountDown)
	: mGameFrameDimension(gameFrameDimension), mCurrentMovementSpeed(3), mCurrentSpeedUpCountDownStart(speedUpCountDown), mLevel(1),
	  mViewDirection(direction), mTailDirection(Snake::getOpposite(direction))
{
	auto& [xStart, yStart] = start;
	auto& [xGameDimension, yGameDimension] = gameFrameDimension;

	// Initialization outside of the game frame
	if (xStart < 0 || xGameDimension < xStart)
	{
		mConsoleLogger->error("The x value to spawn the snake {:d} is out of the game frame size ({:d},{:d})",
			xStart, 0, xGameDimension);
		throw std::runtime_error{ "x value out of game frame range" };
	}
	else if (yStart < 0 || yGameDimension < yStart)
	{
		mConsoleLogger->error("The y value to spawn the snake {:d} is out of the game frame size ({:d},{:d})",
			yStart, 0, yGameDimension);
		throw std::runtime_error{ "x value out of game frame range" };
	}

	// Setup of body & head
	mBody = { start };

	Point nextBodyPoint{ start };
	auto& [xNext, yNext] = nextBodyPoint;
	for (int i = 1; i < length; ++i)
	{
		// Makes the snake "grow" in the opposite direction of the initial view direction
		auto [xNextModificator, yNextModificator] { Snake::getDirectionsRelativeCoords(
			Snake::getOpposite(direction) // Snake body grows in opposite direction of viewing direction
		) };
		// buffer because operator+= & operator- not overwritten
		xNext += xNextModificator;
		yNext += yNextModificator;
		mBody.push_back(nextBodyPoint);
	}

	// Extreme case: The snake spawns on a snack & does not grow because the length updates happening in move() TODO
	// Extreme case: Snake spawns around one game field edge
	std::for_each(mBody.begin(), mBody.end(), [this](auto& bodyPoint)
	{
	  Snake::changeToPlayFieldAwarePosition(bodyPoint);
	});

	// Logging
	mConsoleLogger->info("Created from {:d},{:d} to {:d},{:d}:{}",
		xStart, yStart, mBody.back().first, mBody.back().second, mBody);
}

Snake::Snake(const Point& gameFrameDimension, Direction direction, std::int32_t length)
	: Snake(gameFrameDimension, Point{ (gameFrameDimension.first) / 2, (gameFrameDimension.second) / 2 },
	direction, length)
{
}

auto Snake::move(const Point& snackPosition) -> bool
{
	// Counts down on every eaten snack, if == 0 the snake levels up
	static std::int32_t currentSpeedUpCountDown{ mCurrentSpeedUpCountDownStart };

	const auto [xRelativeMoving, yRelativeMoving]{ getDirectionsRelativeCoords(mViewDirection) };
	const auto [xCurrentHead, yCurrentHead] = mBody.front();
	Point newHeadPosition{ xCurrentHead + xRelativeMoving, yCurrentHead + yRelativeMoving };

	// Snake is moving about a border
	changeToPlayFieldAwarePosition(newHeadPosition);

	mBody.push_front(newHeadPosition);

	// Snake grows and maybe levels up if a snack is eaten, else the size stays the same.
	bool ateSnack{ false };
	if (this->isOnSnack(snackPosition))
	{
		ateSnack = true;
		mConsoleLogger->info("Ate snack on position {}. Adjusting size to {}", mBody.front(), this->getLength());
		// Level up semantic
		if (--currentSpeedUpCountDown == 0)
		{
			currentSpeedUpCountDown = ++mCurrentSpeedUpCountDownStart;
			this->levelUp();
		}
	}
	else
	{
		mBody.pop_back();
	}

	// The tail direction is set here to ensure the snake cant eat itself by turning 180°
	mTailDirection = getOpposite(mViewDirection);
	return ateSnack;
}

auto Snake::turn(Direction direction) -> void
{
	//assert(direction != mViewDirection);
	//assert(direction != static_cast<Direction>((directionAsInt + 2) % 4));
	if (mViewDirection == direction || mTailDirection == direction)
	{}
	else
	{
		mViewDirection = direction;
		//mConsoleLogger->info("Turned snake {}", static_cast<int>(direction));
	}
}

auto Snake::isOnSnack(const Point& snackPosition) -> bool
{
	bool onSnack{ false };
	std::for_each(mBody.begin(), mBody.end(), [&](auto& bodyPoint)
	{
	  onSnack += (bodyPoint == snackPosition);
	});

	return onSnack;
}

auto Snake::getBody() -> std::deque<Snake::Point>&
{
	return mBody;
}

auto Snake::getLength() -> std::size_t
{
	return mBody.size();
}

auto Snake::getDirectionsRelativeCoords(Direction direction) -> Point
{
	Point relativeCoordinates{};
	switch (direction)
	{
		using
		enum Direction;
	case NORTH:
	{
		relativeCoordinates = Point{ 0, -1 };
		break;
	}
	case EAST:
	{
		relativeCoordinates = Point{ 1, 0 };
		break;
	}
	case SOUTH:
	{
		relativeCoordinates = Point{ 0, 1 };
		break;
	}
	case WEST:
	{
		relativeCoordinates = Point{ -1, 0 };
		break;
	}
	}
	return relativeCoordinates;
}

auto Snake::isEatingItself() -> bool
{
	Point& head{ mBody.front() };
	bool eatsItself{ false };
	std::for_each(mBody.cbegin() + 1, mBody.cend(), [&](auto& body_point)
	{
	  eatsItself += body_point == head;
	});
	return eatsItself;
}

auto Snake::turnLeft() -> void
{
	int viewDirectionAsInt{ static_cast<int>(mViewDirection) };
	viewDirectionAsInt = (viewDirectionAsInt + 3) % 4;
	this->turn(static_cast<Direction>(viewDirectionAsInt));
}

auto Snake::turnRight() -> void
{
	int viewDirectionAsInt{ static_cast<int>(mViewDirection) };
	viewDirectionAsInt = (viewDirectionAsInt + 1) % 4;
	this->turn(static_cast<Direction>(viewDirectionAsInt));
}

auto Snake::getSpeed() -> double
{
	return mCurrentMovementSpeed;
}

auto Snake::getOpposite(Direction direction) -> Direction
{
	int directionAsInt{ static_cast<int>(direction) };
	return static_cast<Direction>((directionAsInt + 2) % 4);
}

auto Snake::getLevel() -> std::int32_t
{
	return mLevel;
}

// TODO Is this really the correct way of doing move directives? :/
Snake& Snake::operator=(Snake&& snake) noexcept
{
	mBody = std::move(snake.mBody);
	mGameFrameDimension = std::move(snake.mGameFrameDimension);

	mViewDirection = snake.mViewDirection;
	mTailDirection = snake.mTailDirection;
	mCurrentMovementSpeed = snake.mCurrentMovementSpeed;
	mCurrentSpeedUpCountDownStart = snake.mCurrentSpeedUpCountDownStart;
	mLevel = snake.mLevel;

	return *this;
}

auto Snake::levelUp() -> void
{
	++mLevel;
	mCurrentMovementSpeed *= 1.25;
	mConsoleLogger->info("It's time for a speedup! Snake now moves every {}s.",
		static_cast<double>(1) / mCurrentMovementSpeed);
}

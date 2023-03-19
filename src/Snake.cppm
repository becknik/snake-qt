module;

#include <spdlog/sinks/stdout_color_sinks.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

export module Snake;

export import <deque>;
export import <utility>;
export import <cstdint>;
import <algorithm>;
import <cassert>;

export class Snake
{
 public:
	enum class Direction
	{
		NORTH, EAST, SOUTH, WEST
	};
	/// Assuming simple (x,y) tuple
	using Point = std::pair<std::int32_t, std::int32_t>;

 private:
	static const std::shared_ptr<spdlog::logger> mConsoleLogger;

	std::deque<Point> mBody;
	/// (rightX,bottomY)), inclusive intervall [leftX,rightX], not exclusive!
	Point mGameFrameDimension;
	/// Speed calculated implicitly by using 1/movementSpeed as timeout intervall for snake moving
	double mCurrentMovementSpeed;
	/// Snacks to be eaten to speed up the snake, increases every time a full snack set is eaten. See move methods for details
	std::int32_t mCurrentSpeedUpCountDownStart;
	std::int32_t mLevel;
	Direction mViewDirection;
	/// To disallow the snakes head to "eat itself" by moving in its own body by doing a 180° flip
	Direction mTailDirection;

 public:
	explicit Snake(const Point& gameFrameDimension,
		const Point& start, Direction direction = Direction::EAST, std::int32_t length = 4, std::int32_t speedUpCountDown = 3);
	/// Automatically emits the middle of the playfiled & uses this point to spawn the snake
	explicit Snake(const Point& gameFrameDimension, Direction direction = Direction::EAST, std::int32_t length = 4);

	/**
	 * Moves the snake by deleting the last body point & adding a new one to the view direction. If isOnSnack() is true the tail is not trimmed & the snake grows
	 * If a game field border is coressed, the values are auomatically adjusted to spawn the new head point to the opposite border.
	 * If mCurrentSpeedUpCountDownStart snacks were eaten by the snake, the snake levels up resulting in a speed increase. Then the snake resets the level-up countdown to ++mCurrentSpeedUpCountDownStart
	 *
	 * @return true if the snake ate a snack, needed for the Qt QFrame TODO
	 */
	auto move(const Point& snackPosition) -> bool;

	/**
	 * Sets the viewing direction to the specified value. If the current direction is the same as the specified or the
	 * direction of the tail, nothing is done.
	 */
	auto turn(Direction direction) -> void;
	auto turnRight() -> void;
	auto turnLeft() -> void;

	/**
	 * Checks if the snakes head is on the same field like another body point
	 */
	auto isEatingItself() -> bool;
	/// Iterates through the wohle snake body to check if a snack is spawned under it or the snake just ate one
	auto isOnSnack(const Point& snackPosition) -> bool;
	auto getBody() -> std::deque<Snake::Point>&;
	auto getLength() -> std::size_t;
	auto getSpeed() -> double;
	auto getLevel() -> std::int32_t;

	/// Reinitialization of a snake when the game (re)starts TODO
	Snake& operator=(Snake&& snake) noexcept;

 private:
	static auto getDirectionsRelativeCoords(Direction direction) -> Point;
	static auto getOpposite(Direction direction) -> Direction;
	/**
	 * Return the relative position point to a given direction.
	 * E.g. if NORTH is the argument, the return value is (-1,0) which should be equals
	 * the northern direction in QT applications
	 * @param direction
	 * @return North = (0,-1), East (1,0), South (0,1), West (-1,0)
	 */
	auto changeToPlayFieldAwarePosition(Point& point) -> void;
	/// Enhances the speed by 1/4
	auto levelUp() -> void;
};

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

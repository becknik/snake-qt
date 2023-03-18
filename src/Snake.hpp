#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>

#include <deque>
#include <utility>
#include <cstdint>

class Snake
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
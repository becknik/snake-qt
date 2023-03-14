#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>

#include <deque>
#include <vector>
#include <utility>
#include <cstdint>

enum class Direction
{
	NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3
};

class Snake
{
 public:
	/// Assuming simple (x,y) tuple
	using Point = std::pair<std::int32_t, std::int32_t>;

 private:
	static const std::shared_ptr<spdlog::logger> m_consoleLogger;

	/// ((leftX,topY),(rightX,bottomY)), inclusive
	std::pair<Point, Point> m_gameFrameDimension;
	std::deque<Point> m_body;
	std::int32_t m_length;
	Direction m_viewDirection;
	Direction m_tailDirection;
	/// Speed calculated implicitly by using 1/movementSpeed as timeout intervall
	double m_movementSpeed;
	/// Snacks to be eaten to speed up the snake, increases every time a full snack set is eaten
	std::int32_t m_speedUpCountDown;
	std::int32_t m_level;

 public:
	/// GameFrameDimension is seen as inclusively
	explicit Snake(const std::pair<Point,Point>& gameFrameDimension,
		const Point& start, Direction direction = Direction::EAST, std::int32_t length = 4, std::int32_t speedUpCountDown = 3);
	/// GameFrameDimension is seen as inclusively
	explicit Snake(const std::pair<Point,Point>& gameFrameDimension,
		Direction direction = Direction::EAST, std::int32_t length = 4);

	/**
	 * Moves the snake by deleting the last body point & adding a new one to the view direction. If isOnSnack() is true the tail is not trimmed
	 * If a gamefield border is coressed, the values are auomatically adjusted to spawn the new head point to the opposite border
	 *
	 * @param gameFrameSize The size of the game frame the snake should move in. It differs from the QPoint specification. Formatting: ((leftX,topY),(rightX,bottomY))
	 * @return true if the snake ate a snack
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
	auto getBody() -> std::vector<Point>;
	auto getLength() const -> std::int32_t;
	auto getSpeed() -> double;
	auto getLevel() -> std::int32_t;

	Snake& operator=(Snake&& snake) noexcept;

 private:
	static constexpr auto getDirectionsRelativeCoords(Direction direction) -> Point;
	static constexpr auto getOpposite(Direction direction) -> Direction;
	auto isOnSnack(const Point& snackPosition) -> bool;
	/**
	 * Return the relative position point to a given direction.
	 * E.g. if NORTH is the argument, the return value is (-1,0) which should be equals
	 * the northern direction in QT applications
	 * @param direction
	 * @return North = (0,-1), East (1,0), South (0,1), West (-1,0)
	 */
	constexpr auto changeToPlayFieldAwarePosition(Point& point) -> void;
};
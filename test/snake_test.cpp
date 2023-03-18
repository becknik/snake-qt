#include "Snake.hpp"

#define BOOST_TEST_MODULE Snake Test
#include <boost/test/unit_test.hpp>


struct DefaultSnake
{
	Snake::Point imaginaryMap{ 10, 10 };
	Snake snake{ imaginaryMap, Snake::Direction::NORTH, 3 };
	auto setup() -> void
	{
		BOOST_TEST_MESSAGE("> Constructing Default Snake");
	}
	auto teardown() -> void
	{
		BOOST_TEST_MESSAGE("> Deconstructing Default Snake");
	}
};

struct LongSnake
{
	Snake::Point imaginaryMap{ 10, 10 };
	Snake snake{ imaginaryMap, Snake::Direction::NORTH, 6 };
	auto setup() -> void
	{
		BOOST_TEST_MESSAGE("> Constructing Long Snake");
	}
	auto teardown() -> void
	{
		BOOST_TEST_MESSAGE("> Deconstructing Long Snake");
	}
};

struct ExtremeSnake1
{
	Snake::Point imaginaryMap { 10, 10 };
	Snake snake{ imaginaryMap, { 0, 0 }, Snake::Direction::EAST, 10 };
	auto setup() -> void
	{
		BOOST_TEST_MESSAGE("> Constructing Extreme Snake");
	}
	auto teardown() -> void
	{
		BOOST_TEST_MESSAGE("> Deconstructing Extreme Snake");
	}
};

BOOST_AUTO_TEST_SUITE(main_test_suite);

	BOOST_FIXTURE_TEST_CASE(moving_and_turning_test, DefaultSnake)
	{
		std::deque<Snake::Point> referenceBody{{ 5, 5 }, { 5, 6 }, { 5, 7 }};
		BOOST_TEST(referenceBody == snake.getBody());

		// Moving snake to border
		snake.move({ 0, 0 });
		snake.move({ 0, 0 });
		snake.move({ 0, 0 });
		snake.move({ 0, 0 });
		snake.move({ 0, 0 });

		std::for_each(referenceBody.begin(), referenceBody.end(), [](auto& point)
		{
		  point.second -= 5;
		});

		BOOST_TEST(referenceBody == snake.getBody());

		// Moving snake around border
		snake.move({ 0, 0 });

		referenceBody[0] = { 5, 10 };
		referenceBody[1] = { 5, 0 };
		referenceBody[2] = { 5, 1 };

		BOOST_TEST(referenceBody == snake.getBody());

		// Turning snake to the right & moving it to the rightmost position
		snake.turnRight();
		snake.move({ 0, 0 });
		snake.move({ 0, 0 });
		snake.move({ 0, 0 });
		snake.move({ 0, 0 });
		snake.move({ 0, 0 });

		referenceBody[0] = { 10, 10 };
		referenceBody[1] = { 9, 10 };
		referenceBody[2] = { 8, 10 };

		BOOST_TEST(referenceBody == snake.getBody());
		snake.turnLeft();
	}

	BOOST_FIXTURE_TEST_CASE(moving_and_turning_test2, DefaultSnake)
	{
		std::deque<Snake::Point> referenceBody{{ 5, 5 }, { 5, 6 }, { 5, 7 }};
		BOOST_TEST(referenceBody == snake.getBody());
		fmt::print("{} - Initial snake \n", snake.getBody());

		snake.turnLeft();

		// Moving snake to border
		snake.move({ 0, 0 });
		//fmt::print("{} - Moving snake \n", snake.getBody());
		snake.move({ 0, 0 });
		//fmt::print("{} - Moving snake \n", snake.getBody());
		snake.move({ 0, 0 });
		//fmt::print("{} - Moving snake \n", snake.getBody());
		snake.move({ 0, 0 });
		//fmt::print("{} - Moving snake \n", snake.getBody());
		snake.move({ 0, 0 });
		//fmt::print("{} - Moving snake \n", snake.getBody());

		referenceBody = {{ 0, 5 }, { 1, 5 }, { 2, 5 }};

		BOOST_TEST(referenceBody == snake.getBody());

		// Moving snake around border
		snake.move({ 0, 0 });
		//fmt::print("{}\n", snake.getBody());
		referenceBody = {{ 10, 5 }, { 0, 5 }, { 1, 5 }};

		BOOST_TEST(referenceBody == snake.getBody());

		// Let the snake lurk on the right side
		snake.turnRight();
		snake.move({ 0, 0 });
		snake.turnRight();
		snake.move({ 0, 0 });

		fmt::print("{}\n", snake.getBody());
		referenceBody = {{ 0, 4 }, { 10, 4 }, { 10, 5}};

		BOOST_TEST(referenceBody == snake.getBody());
	}

	BOOST_FIXTURE_TEST_CASE(growing_test, DefaultSnake)
	{
		// Snake eats 3 snacks
		snake.move({ 5, 4 });
		snake.move({ 5, 3 });
		snake.move({ 5, 2 });

		std::deque<Snake::Point> referenceBody{{ 5, 2 }, { 5, 3 }, { 5, 4 }, { 5, 5 }, { 5, 6 }, { 5, 7 }};
		BOOST_TEST(referenceBody == snake.getBody());

		// Snack spawn in snakes head before moving
		snake.move({ 5, 2 });

		BOOST_TEST(snake.getLength() == 7);

		referenceBody.emplace_back();
		std::shift_right(referenceBody.begin(), referenceBody.end(), 1);
		referenceBody[0] = { 5, 1 };

		BOOST_TEST(referenceBody == snake.getBody());

		// Snack spawn in snakes last tail point before moving
		snake.move({ 5, 6 });

		referenceBody.emplace_back();
		std::shift_right(referenceBody.begin(), referenceBody.end(), 1);
		referenceBody[0] = { 5, 0 };

		BOOST_TEST(snake.getLength() == 8);

		BOOST_TEST(referenceBody == snake.getBody());
	}

	BOOST_FIXTURE_TEST_CASE(eat_itself_test, LongSnake)
	{
		//fmt::print("Initial snake: {}\n",snake.getBody());
		snake.turnRight();
		snake.move({ 0, 0 });
		//fmt::print("{}\n",snake.getBody());
		snake.turnRight();
		snake.move({ 0, 0 });
		//fmt::print("{}\n",snake.getBody());
		snake.turnRight();
		snake.move({ 0, 0 });
		//fmt::print("{}\n",snake.getBody());
		BOOST_TEST(snake.isEatingItself());
	}

	BOOST_FIXTURE_TEST_CASE(extreme_snake_test, ExtremeSnake1)
	{
		std::deque<Snake::Point> referenceBody{{ 0, 0 }, { 10, 0 }, { 9, 0 }, { 8, 0 }, { 7, 0 }, { 6, 0 }, { 5, 0 },
		                                        { 4, 0 }, { 3, 0 }, { 2, 0 }};
		fmt::print("{}\n", snake.getBody());
		BOOST_TEST(referenceBody == snake.getBody());
	}

BOOST_AUTO_TEST_SUITE_END();
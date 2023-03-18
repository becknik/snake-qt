#pragma once

#include "Snake.hpp"

#include <QFrame>
#include <QTimer>
#include <QSize>
#include <QPainter>
#include <QStatusBar>

#include <random>

// Standard namespace for classes generated from .ui files - I won't use this...
/*QT_BEGIN_NAMESPACE
namespace Ui {
	class SnakeGameFrame;
}
QT_END_NAMESPACE*/
//Q_OBJECT

class SnakeGameFrame : public QFrame
{
 public:
	explicit SnakeGameFrame(QWidget* parent = nullptr, QStatusBar* statusBar = nullptr, Snake::Point gameFrameSize = {
		14, 14 });

 protected:
	/// Uses the isGameRunning variable to determine if it should draw the title screen or the snake. Uses mPainter with different color setups to draw
	void paintEvent(QPaintEvent*) override;
	/// For actions on key press & timed printing of the game field. Uses the isGameRunning variable to determine if it has to listen on Space or WASD
	void keyPressEvent(QKeyEvent* qKeyEvent) override;

 private:
	static const std::shared_ptr<spdlog::logger> mConsoleLogger;

	/// To display the current game stats in a text filed below the play field
	QStatusBar* mGameStatusBar;
	QPainter mPainter;
	std::unique_ptr<QTimer> mSnakeMoveTimer;
	/// Used for distributing snacks along the x & y axis
	std::pair<std::uniform_int_distribution<int>, std::uniform_int_distribution<int>> mSnackDistribution;

	/// Used to display font in the middle of the screen
	QSize mGameQFrameSize;
	constexpr static QSize mTileDimension{ 20, 20 };
	/// Margin between two tiles, applied on every tile side (including towards the window borders)
	constexpr static int mTileMargin{ 3 };

	// Snake & moving stuff
	Snake mSnake;
	Snake::Point mGameFrameSize;
	/// The outest bottom left game coordinate the snake can reach
	Snake::Point mSnack;
	bool mIsGameRunning;

	/// Initially updates the game, starts the timer & sets the isGameRunning variable to true.
	/// The timer calls the paintEvent & snakeCoordinator functions to move the snake & draw it on the screen
	auto startGame() -> void;
	auto snakeCoordinator() -> void;
	/// Takes the bottom left point of the snake game frame (points on which the actual snake can move) & calculates the
	/// Qt game field size
	static auto calculateGameFrameSize(const Snake::Point& frameSize) -> QSize;
	auto getSnakeMovementIntervall() -> std::int32_t;
	/// Randomly generates a new position for the snack member using a mt wich is statically initialized
	auto generateNewSnack() -> void;
	static auto transformPointToDisplayTile(const Snake::Point& gameFieldPoint) -> QRect;
};

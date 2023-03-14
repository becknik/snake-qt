#pragma once

#include "Snake.hpp"

#include <QFrame>
#include <QPoint>
#include <QTimer>
#include <QSize>
#include <QStatusBar>
#include <QKeyEvent>

#include <random>
#include <QPainter>

// Standard namespace for classes generated from .ui files
/*QT_BEGIN_NAMESPACE
namespace Ui {
	class SnakeGameFrame;
}
QT_END_NAMESPACE*/
//Q_OBJECT

class SnakeGameFrame : public QFrame
{
 public:
	explicit SnakeGameFrame(QWidget* parent = nullptr, QStatusBar* statusBar = nullptr, Snake::Point gameFrameSize = {14,14});

 protected:
	void paintEvent(QPaintEvent*) override;
	void keyPressEvent(QKeyEvent* qKeyEvent) override;

 private:
	static const std::shared_ptr<spdlog::logger> m_consoleLogger;

	// Game filed setup & spawning logic
	QStatusBar* m_statusBar;
	QPainter m_painter;
	constexpr static QSize m_tile_dimension{20,20};
	constexpr static int m_tile_margin{3};
	static std::mt19937 m_snackMt;
	/// Used for distributing snacks along the x & y axis
	std::pair<std::uniform_int_distribution<int>,std::uniform_int_distribution<int>> m_snackDistribution;

	// Snake & moving stuff
	const Snake::Point m_gameFrameSize;
	std::unique_ptr<QTimer> m_snakeMoveTimer;
	Snake m_snake;
	Snake::Point m_snack;
	bool m_gameIsRunning;

	auto startGame() -> void;
	auto snakeOverlord() -> void;
	/// Takes the bottom left point of the snake game frame (points on which the actual snake can move) & calculates the
	/// Qt game field size
	auto calculateGameFrameSize(const Snake::Point& frameSize) -> QSize;
	auto getSnakeMovementIntervall() -> std::int32_t;
	/// Randomly generates a new position for the snack member
	auto generateNewSnack() -> void;
	static auto transformPointToDisplayTile(const Snake::Point& gameFieldPoint) -> QRect;
};

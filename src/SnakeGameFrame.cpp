#include "SnakeGameFrame.hpp"

#include <QPainter>
#include <QKeyEvent>
#include <QtCore/QCoreApplication>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <iostream>

std::shared_ptr<spdlog::logger> const SnakeGameFrame::m_consoleLogger{ spdlog::stderr_color_mt("Game Field") };
std::mt19937 SnakeGameFrame::m_snackMt{ std::random_device{}() };

SnakeGameFrame::SnakeGameFrame(QWidget* parent, QStatusBar* statusBar, const Snake::Point gameFrameSize)
	: QFrame(parent), m_statusBar(statusBar), m_painter(this),
	  m_snackDistribution(std::uniform_int_distribution{ 0, gameFrameSize.first },
		  std::uniform_int_distribution{ 0, gameFrameSize.second }),
	  m_gameFrameSize(gameFrameSize), m_snakeMoveTimer(new QTimer(this)),
	  m_snake({ std::pair(0, 0), gameFrameSize }),
	  m_gameIsRunning(false)
{
	// Setting the dimension of the game frame
	QSize qFrameSize{ calculateGameFrameSize(gameFrameSize) };
	this->setGeometry(0, 0, qFrameSize.width(), qFrameSize.height());

	// TODO remove some stuff here
	this->setFixedSize(qFrameSize.width(), qFrameSize.height());
	this->setFrameStyle(QFrame::Box | QFrame::Plain);
	this->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

	// Connecting the timer with the snake moving/game logic
	connect(m_snakeMoveTimer.get(), &QTimer::timeout, this, &SnakeGameFrame::snakeOverlord);

/*
	QKeyEvent* event1{ new QEvent::Type::KeyPress, Qt::Key::Key_W, Qt::KeyboardModifier::NoModifier};
	connect(event1, &QKeyEvent::accept, this, &SnakeGameFrame::show);

	QCoreApplication::postEvent(this, new QKeyEvent{QEvent::Type::KeyPress, Qt::Key::Key_D, Qt::KeyboardModifier::NoModifier});
	QCoreApplication::postEvent(this, new QKeyEvent{QEvent::Type::KeyPress, Qt::Key::Key_S, Qt::KeyboardModifier::NoModifier});
	QCoreApplication::postEvent(this, new QKeyEvent{QEvent::Type::KeyPress, Qt::Key::Key_A, Qt::KeyboardModifier::NoModifier});
*/

	this->update();
}

auto SnakeGameFrame::startGame() -> void
{
	// TODO implement start button or sth

	m_consoleLogger->info("Staring game.");
	m_gameIsRunning = true;

	this->generateNewSnack();

	// Initially paint snake on the frame by calling the overwritten paintEvent method
	update();
	// Setting status bar message
	std::string statusBarMessage {fmt::format("Length: {:d}, Level: {:d} / Snake Update Speed: {:d}ms", m_snake.getLength(), m_snake.getLevel(), SnakeGameFrame::getSnakeMovementIntervall())};
	m_statusBar->showMessage(QString{statusBarMessage.c_str()});

	m_snakeMoveTimer->start(SnakeGameFrame::getSnakeMovementIntervall());
}

auto SnakeGameFrame::getSnakeMovementIntervall() -> std::int32_t
{
	const double buffer{ (static_cast<double>(1) / m_snake.getSpeed()) * 1'000 };
	return static_cast<int>(buffer);
}

auto SnakeGameFrame::snakeOverlord() -> void
{
	bool ateSnack = m_snake.move(m_snack);
	if (ateSnack) {
		this->generateNewSnack();
		m_snakeMoveTimer->setInterval(getSnakeMovementIntervall());

		// Updating status bar message
		std::string statusBarMessage {fmt::format("Length: {:d}, Level: {:d} / {:d}ms", m_snake.getLength(), m_snake.getLevel(), SnakeGameFrame::getSnakeMovementIntervall())};
		m_statusBar->showMessage(QString{statusBarMessage.c_str()});
	}
	if (m_snake.isEatingItself()) {
		m_snakeMoveTimer->stop();

		m_gameIsRunning = false;
		this->update();

		m_statusBar->showMessage("");
	}

	this->update();
}

auto SnakeGameFrame::paintEvent(QPaintEvent*) -> void
{

/* Debugging
	painter.setPen(Qt::GlobalColor::cyan);
	painter.drawRect(SnakeGameFrame::transformPointToDisplayTile({ 0, 0 }));
	painter.drawRect(SnakeGameFrame::transformPointToDisplayTile(m_gameFieldSize));
*/
	m_painter.begin(this);
	if (m_gameIsRunning) {
		// Painting snakes body
		const std::vector<Snake::Point> snakesBody{ m_snake.getBody() };
		// Painting snakes head to make sure it is drawn over the body when eating itself
		m_painter.setPen(Qt::GlobalColor::blue);
		m_painter.drawRect(SnakeGameFrame::transformPointToDisplayTile(m_snack));

		m_painter.setPen(Qt::GlobalColor::green);
		// Draw the snakes body
		std::for_each(snakesBody.begin() + 1, snakesBody.end(), [this](auto snakeBodyPoint)
		{
		  QRect snakeBodyRect{ SnakeGameFrame::transformPointToDisplayTile(snakeBodyPoint) };
		  m_painter.drawRect(snakeBodyRect);
		});

		// Draw the snakes head
		m_painter.setPen(Qt::GlobalColor::red);
		m_painter.drawRect(SnakeGameFrame::transformPointToDisplayTile(snakesBody.front()));
	} else {
		m_painter.setFont(QFont("Arial", 20));
		m_painter.drawText(0,100,"Please press Space to start");
	}
	m_painter.end();
}

auto SnakeGameFrame::calculateGameFrameSize(const Snake::Point& frameSize) -> QSize
{
	// "+2" for displaying the first & last margin gap for the outer rectangle margins
	const int width{ (frameSize.first + 1) * m_tile_dimension.width() + (frameSize.first - 1) * m_tile_margin + 2 };
	const int length{ (frameSize.second + 1) * m_tile_dimension.height() + (frameSize.second - 1) * m_tile_margin + 2 };
	return { width, length };
}

auto SnakeGameFrame::generateNewSnack() -> void
{
	m_snack.first = m_snackDistribution.first(m_snackMt);
	m_snack.second = m_snackDistribution.second(m_snackMt);
	m_consoleLogger->info("Generating new snack on ({},{})", m_snack.first, m_snack.second);
}

auto SnakeGameFrame::transformPointToDisplayTile(const Snake::Point& gameFieldPoint) -> QRect
{
	const QPoint qRectStartingPoint{
		// To avoid tiles being placed into the negative due to the fact of n-1 gaps between n rows, the std::max is used
		gameFieldPoint.first * m_tile_dimension.width() + (std::max(gameFieldPoint.first - 1, 0)) * m_tile_margin + 1,
		gameFieldPoint.second * m_tile_dimension.height() + (std::max(gameFieldPoint.second - 1, 0)) * m_tile_margin + 1
	};
	return { qRectStartingPoint, m_tile_dimension };
}

void SnakeGameFrame::keyPressEvent(QKeyEvent* qKeyEvent)
{
	//std::cout << qKeyEvent->key() << '\n';
	switch (qKeyEvent->key())
	{
	case Qt::Key::Key_W:
	{
		m_snake.turn(Direction::NORTH);
		break;
	}
	case Qt::Key::Key_D:
	{
		m_snake.turn(Direction::EAST);
		break;
	}
	case Qt::Key::Key_S:
	{
		m_snake.turn(Direction::SOUTH);
		break;
	}
	case Qt::Key::Key_A:
	{
		m_snake.turn(Direction::WEST);
		break;
	}
	case Qt::Key::Key_Space:
	{
		if (!m_gameIsRunning)
		{
			m_snake = Snake{{ std::pair(0, 0), m_gameFrameSize }},

			this->startGame();
		}
		break;
	}
	}
}
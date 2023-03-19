module;

#include <QFrame>
#include <QTimer>
#include <QSize>
#include <QPainter>
#include <QStatusBar>
#include <QKeyEvent>
#include <QtCore/QCoreApplication>
#include <spdlog/sinks/stdout_color_sinks.h>

export module SnakeGameFrame;
import Snake;
import <random>;
import <chrono>;

export class SnakeGameFrame : public QFrame
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

std::shared_ptr<spdlog::logger> const SnakeGameFrame::mConsoleLogger{ spdlog::stderr_color_mt("Game Field") };

SnakeGameFrame::SnakeGameFrame(QWidget* parent, QStatusBar* statusBar, const Snake::Point gameFrameSize)
	: QFrame(parent), mGameStatusBar(statusBar), mPainter(this), mSnakeMoveTimer(new QTimer(this)),
	  mSnackDistribution(
		  std::uniform_int_distribution{ 0, gameFrameSize.first },
		  std::uniform_int_distribution{ 0, gameFrameSize.second }),
	  mSnake(gameFrameSize), mGameFrameSize(gameFrameSize), mIsGameRunning(false)
{
	// Setting the dimension of the game frame
	QSize qFrameSize{ SnakeGameFrame::calculateGameFrameSize(gameFrameSize) };
	mGameQFrameSize = qFrameSize;
	this->setGeometry(0, 0, qFrameSize.width(), qFrameSize.height());

	// TODO remove some stuff here
	this->setFixedSize(qFrameSize.width(), qFrameSize.height());
	//this->setFrameStyle(QFrame::Box | QFrame::Plain);
	this->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

	mPainter.setRenderHint(QPainter::Antialiasing);

	// Connecting the timer with the snake moving/game logic
	connect(mSnakeMoveTimer.get(), &QTimer::timeout, this, &SnakeGameFrame::snakeCoordinator);
	// Initially execute draw event
	//this->update(); // Seems to be done implicitly by the main windows in the main method
}

auto SnakeGameFrame::startGame() -> void
{
	mConsoleLogger->info("Staring game.");
	mIsGameRunning = true;

	this->generateNewSnack();

	// Initially paint snake on the frame by calling the overwritten paintEvent method
	update();
	// Setting status bar message
	std::string statusBarMessage{
		fmt::format("Length: {:d}, Level: {:d} / {:d}ms", mSnake.getLength(), mSnake
			.getLevel(), SnakeGameFrame::getSnakeMovementIntervall()) };
	mGameStatusBar->showMessage(QString{ statusBarMessage.c_str() });

	mSnakeMoveTimer->start(SnakeGameFrame::getSnakeMovementIntervall());
}

auto SnakeGameFrame::getSnakeMovementIntervall() -> std::int32_t
{
	const double buffer{ (static_cast<double>(1) / mSnake.getSpeed()) * 1'000 };
	return static_cast<int>(buffer);
}

auto SnakeGameFrame::snakeCoordinator() -> void
{
	bool ateSnack = mSnake.move(mSnack);
	if (ateSnack)
	{
		this->generateNewSnack();
		mSnakeMoveTimer->setInterval(getSnakeMovementIntervall());

		// Updating status bar message
		std::string statusBarMessage{ fmt::format("Length: {:d}, Level: {:d} / {:d}ms", mSnake.getLength(), mSnake
			.getLevel(), SnakeGameFrame::getSnakeMovementIntervall()) };
		mGameStatusBar->showMessage(QString{ statusBarMessage.c_str() });
	}
	if (mSnake.isEatingItself())
	{
		mSnakeMoveTimer->stop();

		mIsGameRunning = false;
		this->update();

		mGameStatusBar->showMessage("");
	}

	this->update();
}

auto SnakeGameFrame::paintEvent(QPaintEvent*) -> void
{
/* Debugging:
	painter.setPen(Qt::GlobalColor::cyan);
	painter.drawRect(SnakeGameFrame::transformPointToDisplayTile({ 0, 0 }));
	painter.drawRect(SnakeGameFrame::transformPointToDisplayTile(m_gameFieldSize));
*/
	mPainter.begin(this); // Painter is now ready
	// Painting snakes body
	if (mIsGameRunning)
	{
		// Painting snack to make sure it is drawn over the body when eating itself
		mPainter.setBrush(Qt::GlobalColor::green);
		mPainter.setPen(Qt::GlobalColor::green);
		mPainter.drawRect(SnakeGameFrame::transformPointToDisplayTile(mSnack));

		mPainter.setPen(Qt::GlobalColor::black);
		mPainter.setBrush(Qt::GlobalColor::black);
		// Draw the snakes body
		auto& snakesBody{ mSnake.getBody() };
		std::for_each(snakesBody.begin() + 1, snakesBody.end(), [this](auto& snakeBodyPoint)
		{
		  const QRect snakeBodyRect{ SnakeGameFrame::transformPointToDisplayTile(snakeBodyPoint) };
		  mPainter.drawRect(snakeBodyRect);
		});

		// Draw the snakes head
		mPainter.setBrush(Qt::GlobalColor::red);
		mPainter.setPen(Qt::GlobalColor::red);
		mPainter.drawRect(SnakeGameFrame::transformPointToDisplayTile(snakesBody.front()));

		// Draw game border
		mPainter.setBrush(Qt::NoBrush);
		mPainter.setPen(Qt::GlobalColor::yellow);
		mPainter.drawRect(0, 0, mGameQFrameSize.width() - 1, mGameQFrameSize.height() - 1); // Idk...
	}
	else
	{ // The start game screen is shown
		const auto& [x, y] = mGameQFrameSize;
		const QRect titleRect{ 0, static_cast<int>(0.1 * y), x, static_cast<int>(0.9 * y) };

		mPainter.setFont(QFont("Arial", 16));
		mPainter.drawText(titleRect, Qt::AlignHCenter, "Please press Space to start!");
		//mPainter.setFont(QFont("Arial", 12)); TODO
		//mPainter.drawText(titleRect, Qt::AlignCenter, "High Scores:\n");
	}
	mPainter.end();
}

auto SnakeGameFrame::calculateGameFrameSize(const Snake::Point& frameSize) -> QSize
{
	// "+2" for displaying the first & last margin gap for the outer rectangle margins
	const int width{ (frameSize.first + 1) * mTileDimension.width() + (frameSize.first - 1) * mTileMargin + 2 };
	const int length{ (frameSize.second + 1) * mTileDimension.height() + (frameSize.second - 1) * mTileMargin + 2 };

	mConsoleLogger->info("Setting window dimension to x={}, y={}", width, length);

	return { width, length };
}

auto SnakeGameFrame::generateNewSnack() -> void
{
	static std::random_device rd{};
	static std::seed_seq seedSeq{
		static_cast<unsigned int> (std::chrono::high_resolution_clock::now().time_since_epoch().count()),
		rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
	static std::mt19937 snackMT{ seedSeq }; // TODO use a more performant thing here

	auto& [firstDirstr, secondDistr] = mSnackDistribution;
	// Prevents the snack from spawning in the snake
	while (mSnake.isOnSnack(mSnack))
	{
		mSnack = { firstDirstr(snackMT), secondDistr(snackMT) };
	}

	mConsoleLogger->info("Generating new snack on ({},{})", mSnack.first, mSnack.second);
}

auto SnakeGameFrame::transformPointToDisplayTile(const Snake::Point& gameFieldPoint) -> QRect
{
	const QPoint qRectStartingPoint{
		// To avoid tiles being placed into the negative due to the fact of n-1 gaps between n rows, the std::max is used
		gameFieldPoint.first * mTileDimension.width() + (std::max(gameFieldPoint.first - 1, 0)) * mTileMargin + 1,
		gameFieldPoint.second * mTileDimension.height() + (std::max(gameFieldPoint.second - 1, 0)) * mTileMargin + 1
	};
	return { qRectStartingPoint, mTileDimension };
}

void SnakeGameFrame::keyPressEvent(QKeyEvent* qKeyEvent)
{
	if (mIsGameRunning)
	{
		switch (qKeyEvent->key())
		{
			using
			enum Snake::Direction;
		case Qt::Key::Key_W:
		{
			mSnake.turn(NORTH);
			break;
		}
		case Qt::Key::Key_D:
		{
			mSnake.turn(EAST);
			break;
		}
		case Qt::Key::Key_S:
		{
			mSnake.turn(SOUTH);
			break;
		}
		case Qt::Key::Key_A:
		{
			mSnake.turn(WEST);
			break;
		}
		}
	}
	else
	{
		switch (qKeyEvent->key())
		{
		case Qt::Key::Key_Space:
		{
			mSnake = Snake{ mGameFrameSize };
			this->startGame();
		}
		}
	}
}

#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QStatusBar>
#include <QBoxLayout>

import SnakeGameFrame;

auto main(int argc, char* argv[]) -> int
{
	QApplication app{ argc, argv };

	QMainWindow mainWindow{};
	mainWindow.setWindowTitle(QString{ "Snake Qt" });
	// No needed due to the snake frame setting the window dimensions below
	//mainWindow.setGeometry(500,200,400,400);

	QStatusBar gameStatusBar{ &mainWindow };
	mainWindow.setStatusBar(&gameStatusBar);

	SnakeGameFrame snakeGameFrame{ &mainWindow, &gameStatusBar };
	// Using the game snakeGameFrame's size to determine the main window dimension
	QSize snakeGameFrameSize{ snakeGameFrame.size() };
	mainWindow.setGeometry(0, 0, snakeGameFrameSize.width(), snakeGameFrameSize.height()+ gameStatusBar.height());

	mainWindow.show();

	return QApplication::exec();
}

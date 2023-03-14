#include "SnakeGameFrame.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QStatusBar>
#include <QProgressBar>
#include <QBoxLayout>

auto main(int argc, char* argv[]) -> int
{
	QApplication app{ argc, argv };

	QMainWindow mainWindow{};
	mainWindow.setWindowTitle(QString{ "Snake Qt" });
	mainWindow.setGeometry(500,200,400,400);

	//QToolBar toolBar{&mainWindow};

	QStatusBar statusBar{&mainWindow};
	statusBar.showMessage(QString{ "This is the place where sometime stats might be displayed..." });
	mainWindow.setStatusBar(&statusBar);

/*
	QProgressBar progressBar{&statusBar};
	progressBar.setMaximum(100);
	progressBar.setValue(42);

	statusBar.addWidget(&progressBar);
*/

	SnakeGameFrame snakeGameFrame{ &mainWindow, &statusBar};
	// Using the game snakeGameFrame's size to determine the main window dimension
	QSize snakeGameFrameSize{ snakeGameFrame.size()};
	mainWindow.setGeometry(0,0,snakeGameFrameSize.width(), snakeGameFrameSize.height());

/*
	QApplication::postEvent(&snakeGameFrame, new QKeyEvent{QEvent::Type::KeyPress, Qt::Key::Key_W, Qt::KeyboardModifier::NoModifier});
	QApplication::postEvent(&snakeGameFrame, new QKeyEvent{QEvent::Type::KeyPress, Qt::Key::Key_D, Qt::KeyboardModifier::NoModifier});
	QApplication::postEvent(&snakeGameFrame, new QKeyEvent{QEvent::Type::KeyPress, Qt::Key::Key_S, Qt::KeyboardModifier::NoModifier});
	QApplication::postEvent(&snakeGameFrame, new QKeyEvent{QEvent::Type::KeyPress, Qt::Key::Key_A, Qt::KeyboardModifier::NoModifier});
*/

	//QBoxLayout layout{QBoxLayout::Direction::TopToBottom};
	//layout.addWidget(&snakeGameFrame,0);

	mainWindow.show();

	return QApplication::exec();
}

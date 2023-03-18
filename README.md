# snake-qt

A simple (less good looking) snake game written in C++20 & Qt6, tested under Linux.

The used modern C++ set of features isn't that large due to the project not really being that complicated...

## Useful References

Here are some references (besides basic doc) I used for this mini project:
- [Geeks4Geeks article](https://www.geeksforgeeks.org/pyqt5-snake-game/) about how to create a snake game in Python
  - I don't like and know Python, but the step-by-step scheme was a little help for me
  - It also recommended useing a [QFrame](https://doc.qt.io/qt-6/qframe.html) object insted of a plain [QWidget](https://doc.qt.io/qt-6/qwidget.html), which is more appropiate
- A [StackOverflow post](https://stackoverflow.com/questions/3309708/draw-in-a-qframe-on-clicking-a-button) whith a relative example
- [cfanatic's repo](https://github.com/cfanatic/qt-snake) using Qt5
  - Only caused confusion & bugs... The user name should have been a warning to me...
- [Learn Cpp Mersenne Twister tutorial](https://www.learncpp.com/cpp-tutorial/generating-random-numbers-using-mersenne-twister/)
  - This site really is worth gold!

## Building

### Local dependencies

```
- GCC 13.0 / Clang 15.0
- CMake 2.24
- Qt6 6.4
- Boost 1.81 | for testing
- spdlog 1.11 | for minimal logging
```

### Instructions

- clone repo

```bash
cmake -G Ninja -D CMAKE_BUILD_TYPE=Release -S ./snake-qt -B ./snake-qt/build snake

cmake --build ./snake-qt/build --target snake -j 8
# "--target snake_unit_tests" for the Snake classes unit tests

./snake-qt/build/snake
```
### Preview

![Preview Picture - What a beauty!|400](.preview/Screenshot%20from%202023-03-19%2000-10-59.png)

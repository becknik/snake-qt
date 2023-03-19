find_package(Boost REQUIRED COMPONENTS unit_test_framework)
#set(Boost_THREADAPI pthread)

include(CTest)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./)
file(GLOB RELATIVE UNIT_TEST_FILES *.cpp *.hpp) # https://cmake.org/cmake/help/latest/command/file.html?highlight=file#glob

include_directories(${PROJECT_SOURCE_DIR}/src)
add_executable(snake_unit_tests snake_test.cpp)
target_link_libraries(snake_unit_tests PRIVATE fmt Boost::unit_test_framework)

add_test(NAME snake_unit_tests COMMAND snake_unit_tests)
cmake_minimum_required(VERSION 3.26)


set(YES_PLEASE_GIVE_ME_WARNINGS ON) # Enables advanced compiler warning output
set(ENABLE_IWYU OFF) # Enable the include-what-you-use analyzer
set(ENABLE_CCACHE OFF)


# Automated setup of mold & GNU gold linker if detected
if (UNIX)
	execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=mold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE mold_version_info)
	execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE gold_version_info)
	if ("${mold_version_info}" MATCHES "mold")
		message(STATUS "Setting mold as linker")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold -Wl,--disable-new-dtags")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=mold -Wl,--disable-new-dtags")
	elseif ("${gold_version_info}" MATCHES "GNU gold")
		message(STATUS "Setting GNU gold as linker")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
	else ()
		message(STATUS "Using default linker. Neither mold nor GNU gold linker were found.")
	endif ()
endif ()

if (YES_PLEASE_GIVE_ME_WARNINGS)
	message(STATUS "Enabling advanced compiler warning flags")
	add_compile_options(-Wall -Wextra -pedantic -Weffc++ -Wsign-conversion -Wshadow)
endif ()

# Automated ccache setup
if (ENABLE_CCACHE)
	find_program(CCACHE_FOUND ccache)
	if (CCACHE_FOUND)
		message(STATUS "Enabling ccache support")
		set_property(GLOBAL PROPERTY CMAKE_CXX_COMPILER_LAUNCHER ccache)
		#set_property(GLOBAL PROPERTY CMAKE_CXX_LINKER_LAUNCHER ccache) # Linking doesn't benefit from ccache
	endif ()
endif ()

# Automated include-what-you-use setup
if (ENABLE_IWYU)
	#if (CMAKE_CXX_COMPILER_ID MATCHES "Clang") # Doesn't work...
	find_program(IWYU_EXE include-what-you-use)
	if (${IWYU_EXE})
		#message(STATUS "Found 'include-what-you-use' - your compiler is Clang!")
		message(STATUS "Enabling 'include-what-you-use' analyzer")
		set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${IWYU_EXE}")
	else()
		message(STATUS "'include-what-you-use' analyzer not found on system")
	endif()
	#endif()
endif ()

## C++20 Modules configuration

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
#set(CMAKE_CXX_EXTENSIONS OFF) Clang
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	include(gcc_modules.cmake)
endif()

# https://gitlab.kitware.com/cmake/cmake/-/blob/master/.gitlab/ci/cxx_modules_rules_gcc.cmake
#set(CMake_TEST_CXXModules_UUID "a246741c-d067-4019-a8fb-3d16b0c9d1d3")

string(CONCAT CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE # GCC
	"<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -E -x c++ <SOURCE>"
	" -MT <DYNDEP_FILE> -MD -MF <DEP_FILE>"
	" -fmodules-ts -fdep-file=<DYNDEP_FILE> -fdep-output=<OBJECT> -fdep-format=trtbd"
	" -o <PREPROCESSED_SOURCE>")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FORMAT "gcc")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FLAG "-fmodules-ts -fmodule-mapper=<MODULE_MAP_FILE> -fdep-format=trtbd -x c++")

## Project configuration

project(snake VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# Library setup

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
#add_library(fmt SHARED IMPORTED)
#set_target_properties(fmt PROPERTIES IMPORTED_LOCATION /lib/libfmt.so)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

## Target setup

file(GLOB EXE_INCLUDE_FILES ${PROJECT_SOURCE_DIR}/src/*.?pp*) # include .cpp, .hpp, .mpp & .cppm files
add_library(snake.lib)
target_sources(snake.lib PUBLIC
	FILE_SET cxx_modules TYPE CXX_MODULES FILES
	src/Snake.cppm src/SnakeGameFrame.cppm
	)

add_executable(snake src/main.cpp)
target_link_libraries(snake PRIVATE Qt6::Widgets fmt) # PUBLIC snake.lib

# Test setup

enable_testing()
add_subdirectory(${PROJECT_SOURCE_DIR}/test)

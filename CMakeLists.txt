cmake_minimum_required(VERSION 4.0)

project(RVemu)

option(WITH_GUI "Compile Emulator with GUI" OFF)
option(WITH_ELF "Compile Emulator with ELFIO" ON)
option(WITH_TESTS "Build unit tests (requires GoogleTest)" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

IF(WITH_GUI)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)
	find_package(Qt5 COMPONENTS Widgets REQUIRED)
ENDIF(WITH_GUI)

IF(WITH_ELF)
	find_package(elfio CONFIG REQUIRED)
ENDIF(WITH_ELF)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
set(INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)

set(PROJECT_NAME RVemu)

add_subdirectory(src)

IF(WITH_TESTS AND NOT WITH_GUI)
	enable_testing()
	add_subdirectory(tests)
ENDIF()

cmake_minimum_required(VERSION 3.2)

project(RVemu)

option(WITH_GUI "Compile Emulator with GUI" OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

IF(WITH_GUI)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)
	find_package(Qt5 COMPONENTS Widgets REQUIRED)
ENDIF(WITH_GUI)


set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src)



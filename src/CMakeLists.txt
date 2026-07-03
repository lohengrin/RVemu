
# Core library: all device + CPU sources, reusable by the emulator
# executable and the unit-test executables. The application entrypoint
# (main.cpp / mainGUI.cpp) is NOT part of the library.

set(CORE_FILES
	Defines.h
	Device.h
	Cpu.h
	Cpu.cpp
	CpuUtils.cpp
	Memory.h
	Memory.cpp
	Bus.h
	Bus.cpp
	Trap.h
	Trap.cpp
	Plic.h
	Plic.cpp
	Clint.h
	Clint.cpp
	Uart.h
	Uart.cpp
	VirtIO.h
	VirtIO.cpp
)

IF(WITH_ELF)
	set(ELF_SOURCE_FILES
		ElfLoader.h
		ElfLoader.cpp
	)
ENDIF(WITH_ELF)

add_library(RVemuCore STATIC
	${CORE_FILES}
	${ELF_SOURCE_FILES}
)

IF(NOT WIN32)
	target_link_libraries(RVemuCore	pthread)
ENDIF(NOT WIN32)

IF(WITH_ELF)
	target_link_libraries(RVemuCore elfio::elfio)
	target_compile_definitions(RVemuCore PUBLIC WITH_ELFIO)
ENDIF(WITH_ELF)

target_include_directories(RVemuCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


# Application entrypoint selection
IF(WITH_GUI)
	set(GUI_FILES
		GUI/MainWindow.cpp
		GUI/MainWindow.h
		GUI/MainWindow.ui
		GUI/RVemu.qrc
		GUI/ComputerThread.h
		GUI/ComputerThread.cpp
		GUI/Terminal.h
		GUI/Terminal.cpp
	)
	set(MAIN_FILE GUI/mainGUI.cpp)
ELSE(WITH_GUI)
	set(MAIN_FILE main.cpp)
ENDIF(WITH_GUI)

add_executable(RVemu
	${MAIN_FILE}
	${GUI_FILES}
)

target_link_libraries(RVemu PRIVATE RVemuCore)

IF(WITH_GUI)
	find_package(Qt5 COMPONENTS Widgets REQUIRED)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)
	target_include_directories(RVemu PRIVATE ./GUI/)
	target_compile_definitions(RVemu PRIVATE WITH_GUI)
	target_link_libraries(RVemu PRIVATE Qt5::Widgets)
ENDIF(WITH_GUI)

install(TARGETS RVemu DESTINATION ${INSTALL_BIN_DIR})

if(WITH_BENCHMARK)
	add_executable(RVemuBench benchmark_main.cpp)
	target_link_libraries(RVemuBench PRIVATE RVemuCore)
	install(TARGETS RVemuBench DESTINATION ${INSTALL_BIN_DIR})
endif()


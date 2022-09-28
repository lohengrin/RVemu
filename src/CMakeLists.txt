
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
	SET(MAIN_FILE GUI/mainGUI.cpp)
ELSE(WITH_GUI)
	SET(MAIN_FILE main.cpp)
ENDIF(WITH_GUI)

set(SOURCE_FILES 
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


add_executable(RVemu 
${ELF_SOURCE_FILES}
${SOURCE_FILES}
${GUI_FILES}
${MAIN_FILE}
)

IF(NOT WIN32)
	target_link_libraries(RVemu	pthread)
ENDIF(NOT WIN32)

IF(WITH_GUI)
	target_include_directories(RVemu PRIVATE . ./GUI/)
	target_compile_definitions(RVemu PRIVATE WITH_GUI)
	target_link_libraries(RVemu Qt5::Widgets)
ENDIF(WITH_GUI)

IF(WITH_ELF)
	target_link_libraries(RVemu elfio::elfio)
	target_compile_definitions(RVemu PRIVATE WITH_ELFIO)
ENDIF(WITH_ELF)

install(TARGETS RVemu DESTINATION ${INSTALL_BIN_DIR})

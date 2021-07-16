
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
	main.cpp
 )

add_executable(RVemu ${SOURCE_FILES})

IF(WIN32)
target_link_libraries(RVemu
	Ws2_32.lib
)
ENDIF(WIN32)

install(TARGETS RVemu DESTINATION ${RVEMU_INSTALL_BIN_DIR})


set(SOURCE_FILES main.cpp "Cpu.h" "Cpu.cpp" "Memory.h" "Memory.cpp" "Bus.h" "Bus.cpp" )

add_executable(RVemu ${SOURCE_FILES})

install(TARGETS RVemu DESTINATION ${RVEMU_INSTALL_BIN_DIR})


set(SOURCE_FILES main.cpp "Cpu.h" "Cpu.cpp")

add_executable(RVemu ${SOURCE_FILES})

install(TARGETS RVemu DESTINATION ${RVEMU_INSTALL_BIN_DIR})

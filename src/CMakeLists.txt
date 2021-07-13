
set(SOURCE_FILES main.cpp)

add_executable(RVemu ${SOURCE_FILES})

install(TARGETS divider DESTINATION ${RVEMU_INSTALL_BIN_DIR})

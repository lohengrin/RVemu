find_package(GTest CONFIG REQUIRED)

include(GoogleTest)

set(TEST_SOURCES
	MemoryTest.cpp
	BusTest.cpp
	ClintTest.cpp
	PlicTest.cpp
	VirtIOTest.cpp
	UartTest.cpp
	CpuInstructionTest.cpp
	InstructionNameTest.cpp
	ElfLoaderTest.cpp
)

add_executable(RVemuTests ${TEST_SOURCES})

target_link_libraries(RVemuTests PRIVATE
	RVemuCore
	GTest::gtest
	GTest::gtest_main
)

# Some toolchains require pthread for GTest's thread-safe death tests.
find_package(Threads REQUIRED)
target_link_libraries(RVemuTests PRIVATE Threads::Threads)

# Copy test ELF files to build directory
add_custom_command(TARGET RVemuTests POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/test_elfs
	${CMAKE_CURRENT_BINARY_DIR}/test_elfs
	COMMENT "Copying test ELF files to build directory"
)

gtest_discover_tests(RVemuTests WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

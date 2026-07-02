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

gtest_discover_tests(RVemuTests)

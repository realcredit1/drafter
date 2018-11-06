cmake_minimum_required(VERSION 3.5)
set(CTEST_PROJECT_NAME "Drafter")
set(CTEST_BUILD_NAME "Win/${CTEST_BUILD_TYPE}")

set(CTEST_SOURCE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
set(CTEST_BINARY_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/build")

set(CTEST_USE_LAUNCHERS 1)

set(Drafter_CONFIG_OPTIONS "-DINTEGRATION_TESTS=ON")

ctest_start("Continuous")
ctest_configure(OPTIONS "${Drafter_CONFIG_OPTIONS}")
ctest_build()
ctest_test()
ctest_submit()

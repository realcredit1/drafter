cmake_minimum_required(VERSION 3.5)

set(CTEST_PROJECT_NAME "Drafter")
if("${CTEST_BUILD_NAME}" STREQUAL "")
    set(CTEST_BUILD_NAME "${CTEST_BUILD_TYPE}")
else()
    set(CTEST_BUILD_NAME "${CTEST_BUILD_TYPE}/${CTEST_BUILD_NAME}")
endif()

set(CTEST_SOURCE_DIRECTORY ".")
set(CTEST_BINARY_DIRECTORY "build")

set(CTEST_USE_LAUNCHERS 1)
set(CTEST_MODEL "Continuous")

ctest_start(${CTEST_MODEL})
ctest_configure(OPTIONS -DBUILD_SHARED_LIBS=OFF)
ctest_build(TARGET drafter-test-suite)
ctest_test()
ctest_submit(RETURN_VALUE ret_sub)

if(NOT ret_sub EQUAL 0)
    message(WARNING "Unable to submit results to CDash")
endif()

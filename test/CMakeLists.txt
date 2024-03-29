include(../cmake/find_or_fetch_package.cmake)

set(INSTALL_GTEST OFF)
find_or_fetch_package(GTest https://github.com/google/googletest GIT_TAG release-1.11.0)

set(tests_SRCS tests_axis_config.cpp
               tests_basic_context.cpp
               tests_context.cpp
               tests_basic_distribution.cpp
               tests_distribution.cpp
               tests_observable.cpp)

add_executable(gtests ${tests_SRCS})

target_include_directories(gtests PRIVATE ${CMAKE_BINARY_DIR})

if(TARGET GTest::gtest_main)
  # new cmake provided
  set(GTEST_TRG GTest::gtest_main)
else()
  # old cmake and pc files
  set(GTEST_TRG gtest gtest_main)
endif()

target_link_libraries(gtests PRIVATE Midas::Midas pandora::pandora ${GTEST_TRG} ${FMT_TARGET})
if(ENABLE_ADVANCE_TOOLS)
  target_code_coverage(gtests ALL)
endif()

gtest_discover_tests(gtests)

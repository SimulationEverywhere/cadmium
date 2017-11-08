cmake_minimum_required(VERSION 3.6.1)
project(Cadmium)
# The version number.
set (Cadmium_VERSION_MAJOR 0)
set (Cadmium_VERSION_MINOR 0)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

add_library(Cadmium INTERFACE)

include_directories(include ${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -std=c++14")

enable_testing()
FILE(GLOB TestSources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*_test.cpp)
foreach(testSrc ${TestSources})
        get_filename_component(testName ${testSrc} NAME_WE)
        add_executable(${testName} test/main-test.cpp ${testSrc})
        target_link_libraries(${testName} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
	add_test(${testName} ${testName})
endforeach(testSrc)

add_executable(clock_example example/main-clock.cpp)
add_executable(count_fives_example example/main-count-fives.cpp)

# Building time tests (/compile-test)
# These invoke "cmake --build ..."
add_executable(empty_coupled_model_is_a_valid_model_test test-compile/empty_coupled_model_is_a_valid_model_test.cpp)
set_target_properties(empty_coupled_model_is_a_valid_model_test PROPERTIES
                      EXCLUDE_FROM_ALL TRUE
                      EXCLUDE_FROM_DEFAULT_BUILD TRUE)
add_test(NAME empty_coupled_model_is_a_valid_model_test_compile
         COMMAND ${CMAKE_COMMAND} --build . --target empty_coupled_model_is_a_valid_model_test
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})



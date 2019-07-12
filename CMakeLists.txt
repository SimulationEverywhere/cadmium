cmake_minimum_required(VERSION 3.6.1)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version")
project(Cadmium)
# The version number.
set (Cadmium_VERSION_MAJOR 0)
set (Cadmium_VERSION_MINOR 2)
set (Cadmium_VERSION_PATCH 4)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

include(CheckCXXCompilerFlag)
IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("debug mode")
    IF(NOT WIN32)
        include(CodeCoverage)
    ENDIF()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -O0 -fprofile-arcs -ftest-coverage")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS unit_test_framework system thread REQUIRED)

add_library(Cadmium INTERFACE)

include_directories(include ${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")

# Check for standard to use
check_cxx_compiler_flag(-std=c++17 HAVE_FLAG_STD_CXX17)
if(HAVE_FLAG_STD_CXX17)
    message(STATUS "Using C++17 flag")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17")
else()
    message(STATUS "Using C++1z flag")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++1z")
endif()

enable_testing()

#run all tests for sequential and concurrent versions
set(EXEC_TYPES "seq" "par")
foreach (exec_type ${EXEC_TYPES})
    # Unit tests
    FILE(GLOB TestSources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*_test.cpp)
    foreach(testSrc ${TestSources})
            get_filename_component(testName ${testSrc} NAME_WE)
            set(testName "${testName}_${exec_type}")
            add_executable(${testName} test/main-test.cpp ${testSrc})
            if(exec_type STREQUAL "par")
                target_compile_definitions(${testName} PUBLIC CADMIUM_EXECUTE_CONCURRENT)
            endif()
            target_link_libraries(${testName} PUBLIC ${Boost_LIBRARIES})
    	      add_test(${testName} ${testName})
    endforeach(testSrc)

    # Tests that should compile
    FILE(GLOB TestCompileSources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test-compile/compiles/*_test.cpp)
    foreach(testCompSrc ${TestCompileSources})
            get_filename_component(testCompName ${testCompSrc} NAME_WE)
            set(testCompName "${testCompName}_${exec_type}")
            add_executable(${testCompName} ${testCompSrc})
            set_target_properties(${testCompName} PROPERTIES
                                  EXCLUDE_FROM_ALL TRUE
                                  EXCLUDE_FROM_DEFAULT_BUILD TRUE)
            if(exec_type STREQUAL "par")
                target_compile_definitions(${testCompName} PUBLIC CADMIUM_EXECUTE_CONCURRENT)
            endif()
            target_include_directories(${testCompName} PUBLIC test-compile/compiles)
            add_test(NAME ${testCompName}
                     COMMAND ${CMAKE_COMMAND} --build . --target ${testCompName}
                     WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endforeach(testCompSrc)

    # Tests that should fail compilation
    FILE(GLOB TestCompileFailSources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test-compile/compile-fails/*_test.cpp)
    foreach(testCmpFailSrc ${TestCompileFailSources})
            get_filename_component(testCmpFailName ${testCmpFailSrc} NAME_WE)
            set(testCmpFailName "${testCmpFailName}_${exec_type}")
            add_executable(${testCmpFailName} ${testCmpFailSrc})
            set_target_properties(${testCmpFailName} PROPERTIES
                                  EXCLUDE_FROM_ALL TRUE
                                  EXCLUDE_FROM_DEFAULT_BUILD TRUE)
            if(exec_type STREQUAL "par")
                target_compile_definitions(${testCmpFailName} PUBLIC CADMIUM_EXECUTE_CONCURRENT)
            endif()
            add_test(NAME ${testCmpFailName}
                     COMMAND ${CMAKE_COMMAND} --build . --target ${testCmpFailName}
                     WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
            set_tests_properties(${testCmpFailName} PROPERTIES WILL_FAIL TRUE)
    endforeach(testCmpFailSrc)
endforeach(exec_type)

# Examples
add_executable(clock_example example/main-clock.cpp)
add_executable(count_fives_example example/main-count-fives.cpp)

#Library Headers
add_executable(cadmium_headers include)
set_target_properties(cadmium_headers PROPERTIES
                                                 EXCLUDE_FROM_ALL TRUE
                                                 EXCLUDE_FROM_DEFAULT_BUILD TRUE
                                                 LINKER_LANGUAGE CXX)

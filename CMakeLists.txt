cmake_minimum_required(VERSION 3.10)
project(swing_analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build: Debug or Release" FORCE)
endif()

# Add build type-specific flags
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
pkg_check_modules(GOOCANVASMM goocanvasmm-2.0)

find_package(OpenCV REQUIRED)

link_directories(
        ${GTKMM_LIBRARY_DIRS}
        ${GOOCANVASMM_LIBRARY_DIRS}
)

include_directories(
        ${GTKMM_INCLUDE_DIRS}
        ${GOOCANVASMM_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        inc
)

add_executable(swing_analyzer   
    src/main.cpp
    src/AnalyzerWindow.cpp
    src/AnalyzerController.cpp
    src/AnalyzerGui.cpp
    src/VideoSynchronizationEngine.cpp
    src/SyncManager.cpp
    src/Synchronizer.cpp
    src/SynchronizerStateSync.cpp
    src/SynchronizerStateSyncToPlayback.cpp
    src/SynchronizerStatePlaybackPaused.cpp
    src/SynchronizerStatePlaybackPlaying.cpp
    src/VideoChannel.cpp
    src/VideoChannelState.cpp
    src/OpenCVVideoDecoder.cpp
    src/GTKImageUpdater.cpp
    src/VideoFrameManager.cpp
    src/VideoPlaybackController.cpp
    src/ThreadSafeFrameBuffer.cpp
    src/Logger.cpp
    src/Profiler.cpp)

target_link_libraries(swing_analyzer ${GTKMM_LIBRARIES} ${GOOCANVASMM_LIBRARIES} ${OpenCV_LIBS})

# GoogleTest setup (download if not found)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Add test executable for all test/*.cpp files
file(GLOB TEST_SOURCES test/*.cpp)
add_executable(unit_tests ${TEST_SOURCES}
    src/OpenCVVideoDecoder.cpp
    src/GTKImageUpdater.cpp
    src/Logger.cpp
    src/ThreadSafeFrameBuffer.cpp
    src/VideoFrameManager.cpp
    src/VideoChannelState.cpp
    src/VideoPlaybackController.cpp
    src/VideoChannel.cpp
    src/Profiler.cpp
    src/Synchronizer.cpp
    src/SynchronizerStateSync.cpp
    src/SynchronizerStateSyncToPlayback.cpp
    src/SynchronizerStatePlaybackPaused.cpp
    src/SynchronizerStatePlaybackPlaying.cpp)

target_include_directories(unit_tests PRIVATE test)

target_link_libraries(unit_tests
    gtest_main
    gmock
    ${GTKMM_LIBRARIES}
    ${GOOCANVASMM_LIBRARIES}
    ${OpenCV_LIBS}
)

include(GoogleTest)
gtest_discover_tests(unit_tests)

# Add a Coverage build type
set(CMAKE_CXX_FLAGS_COVERAGE "-g -O0 --coverage")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")

# Add a custom target for generating coverage reports
if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    message(STATUS "Enabling coverage flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_COVERAGE}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_COVERAGE}")

    # Add a custom target to generate coverage reports
    add_custom_target(coverage_report
        COMMAND lcov --capture --directory . --output-file coverage.info --rc lcov_branch_coverage=1
        COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage.info --rc lcov_branch_coverage=1
        COMMAND genhtml coverage.info --output-directory coverage
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating HTML coverage report"
    )
endif()

# Rename the 'coverage' target to 'unit_tests_coverage' to avoid conflicts
add_executable(unit_tests_coverage ${TEST_SOURCES}
    src/AnalyzerWindow.cpp
    src/AnalyzerController.cpp
    src/AnalyzerGui.cpp
    src/VideoSynchronizationEngine.cpp
    src/SyncManager.cpp
    src/Synchronizer.cpp
    src/SynchronizerStateSync.cpp
    src/SynchronizerStateSyncToPlayback.cpp
    src/SynchronizerStatePlaybackPaused.cpp
    src/SynchronizerStatePlaybackPlaying.cpp
    src/VideoChannel.cpp
    src/VideoChannelState.cpp
    src/OpenCVVideoDecoder.cpp
    src/GTKImageUpdater.cpp
    src/VideoFrameManager.cpp
    src/VideoPlaybackController.cpp
    src/ThreadSafeFrameBuffer.cpp
    src/Logger.cpp
    src/Profiler.cpp)

target_include_directories(unit_tests_coverage PRIVATE test)

target_link_libraries(unit_tests_coverage
    gtest_main
    gmock
    ${GTKMM_LIBRARIES}
    ${GOOCANVASMM_LIBRARIES}
    ${OpenCV_LIBS}
)

# Enable coverage flags for the 'unit_tests_coverage' target
set_target_properties(unit_tests_coverage PROPERTIES COMPILE_FLAGS "--coverage" LINK_FLAGS "--coverage")

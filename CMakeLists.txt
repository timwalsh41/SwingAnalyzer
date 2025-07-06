cmake_minimum_required(VERSION 3.10)
project(swing_analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_BUILD_TYPE Debug)

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

add_executable(swing_analyzer   src/main.cpp
                                src/AnalyzerWindow.cpp
                                src/Analyzer.cpp
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
    src/VideoFrameManager.cpp)

target_link_libraries(unit_tests
    gtest_main
    ${GTKMM_LIBRARIES}
    ${GOOCANVASMM_LIBRARIES}
    ${OpenCV_LIBS}
)

include(GoogleTest)
gtest_discover_tests(unit_tests)

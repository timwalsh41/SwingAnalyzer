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

add_executable(swing_analyzer src/main.cpp src/AnalyzerWindow.cpp src/Analyzer.cpp src/Logger.cpp)

target_link_libraries(swing_analyzer ${GTKMM_LIBRARIES} ${GOOCANVASMM_LIBRARIES} ${OpenCV_LIBS})

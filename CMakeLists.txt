cmake_minimum_required(VERSION 3.10)
project(gtk_gstreamer_test)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_BUILD_TYPE Debug)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)

find_package(OpenCV REQUIRED)

link_directories(
        ${GTKMM_LIBRARY_DIRS} 
)

include_directories(
        ${GTKMM_INCLUDE_DIRS} 
        ${OpenCV_INCLUDE_DIRS}
        inc
)

add_executable(gtk_gstreamer_test src/main.cpp src/VideoWindow.cpp)

target_link_libraries(gtk_gstreamer_test ${GTKMM_LIBRARIES} ${OpenCV_LIBS})

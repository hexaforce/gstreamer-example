cmake_minimum_required(VERSION 3.10)
project(Receiver)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

add_executable(receiver receiver.cpp)
target_include_directories(receiver PRIVATE ${GST_INCLUDE_DIRS})
target_link_libraries(receiver ${GST_LIBRARIES})

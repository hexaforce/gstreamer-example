cmake_minimum_required(VERSION 3.10)
project(Sender)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

add_executable(sender sender.cpp)
target_include_directories(sender PRIVATE ${GST_INCLUDE_DIRS})
target_link_libraries(sender ${GST_LIBRARIES})

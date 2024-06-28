cmake_minimum_required(VERSION 3.10)
project(${PARENT_PROJECT_NAME}Receiver)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${GST_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${GST_LIBRARIES})

cmake_minimum_required(VERSION 3.27)

project(zwave-flashing-tool)

# set(CXX_STANDARD 17)

# find_package(Curses REQUIRED)
# include_directories(${CURSES_INCLUDE_DIR})
include_directories(inc)

file (GLOB SOURCE_FILES src/*.cpp)

add_executable(zft ${SOURCE_FILES})
target_link_libraries(zft ${CURSES_LIBRARY})

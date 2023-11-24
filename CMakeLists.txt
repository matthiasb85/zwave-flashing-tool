cmake_minimum_required(VERSION 3.27)

project(zwave-flashing-tool)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

find_package(nlohmann_json REQUIRED)

file (GLOB SOURCE_FILES src/*.cpp)

add_executable(zft ${SOURCE_FILES})

target_include_directories(zft 
    PUBLIC 
        inc 
        ${SODIUM_INCLUDE_DIRS}
)

target_link_libraries(zft
    PRIVATE
        ${SODIUM_LIBRARIES}
        nlohmann_json::nlohmann_json
)

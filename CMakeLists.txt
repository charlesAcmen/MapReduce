cmake_minimum_required(VERSION 3.10)
project(map_reduce C CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(spdlog REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/include 
    ${CMAKE_SOURCE_DIR}/include/rpc
)


file(GLOB SOURCE_FILES
    "src/*.cpp"
    "src/rpc/*.cpp"
)

file(GLOB HEADER_FILES
    "include/*.h"
    "include/rpc/*.h"
)

add_library(mp_lib ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(mp_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/rpc
)



# coordinator exe file
add_executable(coordinator src/main_coordinator.cpp)
#difference between target_link_libraries and link_directories:
#link library:libraries to link against when building a given target
#link_directories:directories to search for libraries when linking a given target
#.a or .so files

target_link_libraries(coordinator PRIVATE mp_lib spdlog::spdlog)

# worker exe file
add_executable(worker src/main_worker.cpp )
target_link_libraries(worker PRIVATE mp_lib spdlog::spdlog)
file(COPY ${CMAKE_SOURCE_DIR}/data/ DESTINATION ${CMAKE_BINARY_DIR}/data/)

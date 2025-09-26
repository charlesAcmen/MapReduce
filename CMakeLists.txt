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
# coordinator exe file
add_executable(coordinator src/main_coordinator.cpp ${OURCE_FILES} ${HEADER_FILES})
target_include_directories(
    coordinator PRIVATE 
    ${CMAKE_SOURCE_DIR}/include 
    ${CMAKE_SOURCE_DIR}/include/rpc)
target_link_libraries(coordinator PRIVATE spdlog::spdlog)

# worker exe file
add_executable(worker src/main_worker.cpp ${OURCE_FILES} ${HEADER_FILES})
target_include_directories(
    worker PRIVATE 
    ${CMAKE_SOURCE_DIR}/include 
    ${CMAKE_SOURCE_DIR}/include/rpc)
target_link_libraries(worker PRIVATE spdlog::spdlog)

# add_executable(map_reduce ${SOURCE_FILES} ${HEADER_FILES})
# target_include_directories(
#     map_reduce PRIVATE 
#     ${CMAKE_SOURCE_DIR}/include 
#     ${CMAKE_SOURCE_DIR}/include/rpc)
# target_link_libraries(map_reduce PRIVATE spdlog::spdlog)
file(COPY ${CMAKE_SOURCE_DIR}/data/ DESTINATION ${CMAKE_BINARY_DIR}/data/)

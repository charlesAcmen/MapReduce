cmake_minimum_required(VERSION 3.10)
project(map_reduce C CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(spdlog REQUIRED)


file(GLOB SOURCE_FILES
    "src/*.cpp"
)
file(GLOB HEADER_FILES
    "include/*.h"
)
add_executable(map_reduce ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(map_reduce PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(map_reduce PRIVATE spdlog::spdlog)
file(COPY ${CMAKE_SOURCE_DIR}/data/ DESTINATION ${CMAKE_BINARY_DIR}/data/)

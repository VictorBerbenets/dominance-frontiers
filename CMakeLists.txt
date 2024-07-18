cmake_minimum_required(VERSION 3.15)

project(dom_frontiers)

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/dominance-frontiers.cpp)

set_target_properties(
  ${PROJECT_NAME} PROPERTIES
    COMPILE_FLAGS " -Wall -Wextra "
)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})

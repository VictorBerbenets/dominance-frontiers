cmake_minimum_required(VERSION 3.15)

project(dom_frontiers)

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(CMAKE_CXX_STANDARD 20)

#add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/dominance/main.cpp)
add_executable(graph_build ${CMAKE_CURRENT_SOURCE_DIR}/src/graph-builder/main.cpp)
add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/driver/main.cpp)

set_target_properties(
  graph_build ${PROJECT_NAME} PROPERTIES
    COMPILE_FLAGS " -Wall -Wextra "
)

#target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})
target_include_directories(graph_build PUBLIC ${INCLUDE_DIR})

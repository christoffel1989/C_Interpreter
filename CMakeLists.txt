#cmake version
cmake_minimum_required(VERSION 2.8.11)

#project name
PROJECT(C_INTERPRETER)

#static compile
set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Od")
set(CMAKE_CXX_FLAGS_RELEASE "/MT")

#include current path
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#main
set(PROJECT_MAIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src" CACHE PATH "MAIN directory")
file(GLOB project_main_source_files ${PROJECT_MAIN_DIR}/*.cpp)
source_group("Source Files" FILES ${project_main_source_files})
#algorithm
set(PROJECT_GRAMMA_DIR "${PROJECT_MAIN_DIR}/gramma" CACHE PATH "GRAMMA directory")
file(GLOB project_gramma_header_files ${PROJECT_GRAMMA_DIR}/*.h)
file(GLOB project_gramma_source_files ${PROJECT_GRAMMA_DIR}/*.cpp)
source_group("Header Files\\gramma" FILES ${project_gramma_header_files})
source_group("Source Files\\gramma" FILES ${project_gramma_source_files})

# Add the include directories
include_directories(${PROJECT_MAIN_DIR}
					${PROJECT_GRAMMA_DIR}
					${Qt5Widgets_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
			   ${project_gramma_header_files}
			   ${project_gramma_source_files}
			   ${project_main_source_files})

#multi-core compile and use iso c++17
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/MP /std:c++17")

#bin
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin) 
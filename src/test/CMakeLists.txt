cmake_minimum_required(VERSION 3.5)

#project name
PROJECT(TEST)

add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_options(${PROJECT_NAME} PRIVATE /std:c++latest)
target_link_libraries(${PROJECT_NAME} 
                      PUBLIC 
                      gtest_main
                      GRAMMA)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
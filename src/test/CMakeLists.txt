cmake_minimum_required(VERSION 3.5)

#project name
PROJECT(C_INTERPRETER_TEST)

add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(C_INTERPRETER_TEST 
                      PUBLIC 
                      gtest_main
                      C_INTERPRETER_GRAMMA)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
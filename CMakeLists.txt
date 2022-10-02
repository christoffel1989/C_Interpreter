#cmake version
cmake_minimum_required(VERSION 3.1.0)

#project name
PROJECT(C_INTERPRETER)
add_subdirectory(src/gramma)
add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE C_INTERPRETER_GRAMMA)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/MP")
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin) 

#unit test
#use dynamic msvc runtime
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(gtest_hide_internal_symbols ON CACHE BOOL "" FORCE)
enable_testing()
#import google_test
add_subdirectory(tool/google_test)
#add test case
add_subdirectory(src/test)
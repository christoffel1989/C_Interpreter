#cmake version
cmake_minimum_required(VERSION 3.1.0)

#project name
PROJECT(C_INTERPRETER LANGUAGES CXX)
add_subdirectory(src/gramma)
add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE GRAMMA)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/MP")

#unit test
#use dynamic msvc runtime
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(gtest_hide_internal_symbols ON CACHE BOOL "" FORCE)
enable_testing()
#import google_test
add_subdirectory(toolkit/google_test)

#add test case
add_subdirectory(src/test)

#creat project folder
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(GRAMMA PROPERTIES FOLDER modules)
set_target_properties(TEST PROPERTIES FOLDER modules)
set_target_properties(gtest_main PROPERTIES FOLDER toolkit/gtest)
set_target_properties(gmock PROPERTIES FOLDER toolkit/gtest)
set_target_properties(gmock_main PROPERTIES FOLDER toolkit/gtest)
set_target_properties(gtest PROPERTIES FOLDER toolkit/gtest)
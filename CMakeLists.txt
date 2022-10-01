#cmake version
cmake_minimum_required(VERSION 3.1.0)

#project name
PROJECT(C_INTERPRETER)

#引入google测试框架
enable_testing()
add_subdirectory(tool/google_test)
#强制google_test库采用动态运行时(与后面库编译采用相同的运行时)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(src/gramma)
add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE C_INTERPRETER_GRAMMA)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/MP")

#bin
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin) 

#添加测试案例
add_subdirectory(src/test)
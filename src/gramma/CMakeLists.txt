#project name
PROJECT(C_INTERPRETER_GRAMMA)

file(GLOB gramma_header_files src/*.h)
file(GLOB gramma_source_files src/*.cpp)

add_library(${PROJECT_NAME} STATIC
			${gramma_header_files}
			${gramma_source_files})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME} PUBLIC src)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/MP")
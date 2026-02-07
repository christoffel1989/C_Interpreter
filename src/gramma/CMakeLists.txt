#project name
PROJECT(GRAMMA)

file(GLOB gramma_header_files CONFIGURE_DEPENDS src/*.h)
file(GLOB gramma_source_files CONFIGURE_DEPENDS src/*.cpp)

add_library(${PROJECT_NAME} STATIC
			${gramma_header_files}
			${gramma_source_files})
target_compile_options(${PROJECT_NAME} PRIVATE /std:c++latest)
target_include_directories(${PROJECT_NAME} PUBLIC src)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/MP")
cmake_minimum_required(VERSION 3.17)
project(zadeh VERSION "2.0.2" LANGUAGES CXX)

# Source files
file(GLOB_RECURSE SOURCE_FILES "src/binding/*.cc" "src/binding/*.h" "src/*.h")

# .node file target
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_compile_features(zadeh INTERFACE cxx_std_17)

# Include paths
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INCL})

# linking libs
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB})

cmake_minimum_required(VERSION 3.14)
project(kaleidoscope)
set(CMAKE_CXX_STANDARD 14)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(SOURCE_FILES main.cpp)
add_subdirectory(tests)
add_subdirectory(gtest-lib)
add_subdirectory(kaleidoscope)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader mcjit native)

## Link against LLVM libraries
target_link_libraries(kaleidoscope_run ${llvm_libs})

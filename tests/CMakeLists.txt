include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
add_executable(tests_run kaleidoscope_test.cpp
        ../kaleidoscope/src/lexer.cpp
        ../kaleidoscope/src/parser.cpp
        ../kaleidoscope/src/ast.cpp
        ../kaleidoscope/src/interpreter.cpp
        ../kaleidoscope/src/code_visitor.cpp
        )
target_link_libraries(tests_run gtest gtest_main)
target_link_libraries(tests_run gmock gmock_main)


# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader mcjit native)

## Link against LLVM libraries
target_link_libraries(tests_run ${llvm_libs})
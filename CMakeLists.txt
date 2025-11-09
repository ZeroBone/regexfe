cmake_minimum_required(VERSION 3.16)

# Project name
project(regexfe)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add Mimir dependency
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/mimir/install/lib/cmake/mim")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/mimir/install/lib64/cmake/mim")
find_package(mim REQUIRED)

# Create the executable
add_executable(${PROJECT_NAME} src/Parser.h src/main.cpp
        src/lexer.hpp
        src/token.hpp
        src/lexer.cpp
        src/strqueue.hpp
        src/mimir.hpp
        src/ast.hpp
        src/mimir_codegen.hpp
        src/mimir_codegen.cpp
        src/ast.cpp
        src/tests.cpp
        src/regexfe.cpp
        src/regexfe.hpp
        src/tests.hpp)


if(NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_compile_options(${PROJECT_NAME} PRIVATE /Wall)
endif()

# target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Prevent some runtime library not found issues: set RPATH to find all the libraries
# If you still see issues, consider setting the environment variable LD_LIBRARY_PATH to include the mimir/install/lib directory
# On Windows, you might need to set PATH to include the mimir/install/bin directory
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    message(STATUS "Setting linker options to make sure RPATH is used instead of RUNPATH")
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--disable-new-dtags)
endif()

# Link the Mimir library
target_link_libraries(${PROJECT_NAME} PRIVATE mim::libmim)


cmake_minimum_required(VERSION 2.8)

project(bfjit)

set(ASMJIT_STATIC TRUE)
add_subdirectory(asmjit)

if(NOT MSVC)
	add_compile_options(-std=c++11 -Wall)
endif()

include_directories(asmjit/src)
link_directories(asmjit)
add_executable(bfjit bfjit.cpp asmjit/src/asmjit)
target_link_libraries(bfjit asmjit)
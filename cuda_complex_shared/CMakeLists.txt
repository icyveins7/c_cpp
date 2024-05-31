cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(cuda_complex_shared LANGUAGES CXX CUDA)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
  ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)

add_executable(
  cuda_complex_shared
  src/cuda_complex_shared.cu
)
target_compile_features(cuda_complex_shared PUBLIC cxx_std_11)
set_property(TARGET cuda_complex_shared 
             PROPERTY CUDA_SEPARABLE_COMPILATION ON)
 

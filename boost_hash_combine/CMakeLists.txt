cmake_minimum_required(VERSION 3.20)

project(boost_hash_combine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Boost)
if (${Boost_FOUND})
	message("Boost found " ${Boost_VERSION})
	message(${Boost_INCLUDE_DIRS})
endif()

include_directories(${Boost_INCLUDE_DIRS} ../)


add_executable(main main.cpp)

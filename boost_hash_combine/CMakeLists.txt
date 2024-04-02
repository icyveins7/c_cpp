cmake_minimum_required(VERSION 3.20)

project(boost_hash_combine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost)
if (${Boost_FOUND})
	message("Boost found " ${Boost_VERSION})
	message(${Boost_INCLUDE_DIRS})
	message(${Boost_LIBRARY_DIRS})
	message(${Boost_LIBRARIES})
endif()

add_executable(main main.cpp)

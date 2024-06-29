cmake_minimum_required(VERSION 3.13)

project(redis-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_ROOT ${_VCPKG_INSTALLED_DIR})
set(Boost_DIR ${_VCPKG_INSTALLED_DIR})
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(nlohman-json_IMPLICIT_CONVERSIONS OFF)

find_package(Boost 1.84.0 COMPONENTS program_options)
find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(server ${SOURCE_FILES})

target_include_directories(server PUBLIC inc)
target_include_directories(server PUBLIC ${Boost_INCLUDE_DIRS}) 

target_link_libraries(server PRIVATE asio asio::asio)
target_link_libraries(server PRIVATE Threads::Threads)
target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(server PRIVATE ${Boost_LIBRARIES})
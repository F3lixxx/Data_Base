cmake_minimum_required(VERSION 3.20)

set(PostgreSQL_ADDITIONAL_VERSIONS "16")

project(DataBase)

add_subdirectory("C:\\Program Files\\libpqxx-7.9.0" libpqxx-build)
include_directories(${CMAKE_SOURCE_DIR}/qpxx)


add_executable(DataBase main.cpp)

target_compile_features(DataBase PRIVATE cxx_std_17)

target_link_libraries(DataBase PRIVATE pqxx)
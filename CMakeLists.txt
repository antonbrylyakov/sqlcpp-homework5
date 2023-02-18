cmake_minimum_required(VERSION 3.20)

set(PostgreSQL_ADDITIONAL_VERSIONS "15")

find_package(Catch2 REQUIRED)

set(CMAKE_CXX_STANDARD 20)

project(homework5)

add_subdirectory("../libpqxx-7.7.4" build-pqxx)

add_executable(homework5 "test_main.cpp" "ClientsDb.h" "Phone.h" "Misc.h" "Client.h"  "ClientsDb.cpp"  "ClientsDb_Tests.cpp")

target_link_libraries(homework5 PRIVATE pqxx)

target_link_libraries(homework5 PRIVATE Catch2::Catch2)
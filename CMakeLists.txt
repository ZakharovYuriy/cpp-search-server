cmake_minimum_required(VERSION 3.11)

project(search_server CXX)
set(CMAKE_CXX_STANDARD 17) 

set(SOURCE_DIR search-server)

if (CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

file(GLOB sources
  ${SOURCE_DIR}/*.cpp
  ${SOURCE_DIR}/*.h
)

add_executable(search_server ${sources}) 


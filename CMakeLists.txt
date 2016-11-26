cmake_minimum_required(VERSION 2.8)
project(SeleneJS)

set(CMAKE_CXX_STANDARD 14)
add_definitions(-DDUK_OPT_CPP_EXCEPTIONS)

#[[
if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -stdlib=libc++")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
elseif(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
endif(APPLE)
]]

include_directories(include)
INCLUDE_DIRECTORIES(duktape)
file(GLOB headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  include/*.h include/selenejs/*.h)

add_executable(test_runner ${CMAKE_CURRENT_SOURCE_DIR}/test/Test.cpp ${headers}
    ../duktape/duktape.c
    ../duktape/duktape.h
    ../duktape/duk_config.h
)
SET_SOURCE_FILES_PROPERTIES( ../duktape/duktape.c PROPERTIES LANGUAGE CXX )

target_link_libraries(test_runner ${LUA_LIBRARIES})

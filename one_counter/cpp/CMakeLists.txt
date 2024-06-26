set(OPT_FLAGS "-Ofast -march=native -fomit-frame-pointer -flto")

set(src_engine
        src/V1CA.cpp
        src/learner.cpp
        src/language.cpp
        src/teachers/teacher.cpp
        src/dataframe.cpp
        src/teachers/manual_teacher.cpp
        src/teachers/semi_manual_teacher.cpp
        src/teachers/automatic_v1ca_teacher.cpp
        src/R1CA.cpp
        src/teachers/automaton_teacher.cpp
        src/one_counter_automaton.cpp
        src/behaviour_graph.cpp
        src/alphabet.cpp
        src/dot_writers.cpp
        src/V1CA_reader.cpp
        )

include_directories(includes)

cmake_minimum_required(VERSION 3.15)
project(v1c2al)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -pedantic -ldl")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPT_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3 -fsanitize=address -ldl --coverage")

set(CMAKE_CXX_STANDARD 20)

add_executable(v1c2al src/main.cpp)
target_sources(v1c2al PRIVATE ${src_engine})
#target_link_libraries(v1c2al PRIVATE includes)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(Boost_USE_STATIC_LIBS ON)
endif ()

set(Boost_INCLUDE_DIR c:/mingw/include/)
set(Boost_LIBRARY_DIR c:/mingw/lib/)
find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

#target_include_directories(v1c2al PUBLIC includes)

cmake_minimum_required(VERSION 3.6)
project(pongo_agent)

set(CMAKE_CXX_STANDARD 11)

find_package(PythonLibs 2.7 REQUIRED)
if (${PYTHONLIBS_FOUND})
    message(STATUS "Find python version: ${PYTHONLIBS_VERSION_STRING}")
    include_directories(${PYTHON_INCLUDE_DIRS})
endif ()

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost 1.58.0 COMPONENTS iostreams date_time filesystem system program_options thread python)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

set(SOURCE_FILES main.cpp utils/thread_pool.h python_container.cpp python_container.h python_interpreter.cpp python_interpreter.h
        py_object/py_config.cpp py_object/py_config.h py_object/py_cookies.cpp py_object/py_cookies.h
        py_object/py_logs.cpp py_object/py_logs.h utils/crow.cpp utils/crow.h pongo_connect.cpp pongo_connect.h
        pongo_protocol.cpp pongo_protocol.h utils/sub_process.h utils/sub_process.cpp py_object/py_statistics.cpp py_object/py_statistics.h utils/archive_decompress.cpp utils/archive_decompress.h utils/hash_tools.cpp utils/hash_tools.h utils/singleton.h)

add_subdirectory(zRPC)
include_directories(zRPC/include)

add_executable(pongo_agent ${SOURCE_FILES})

if(${PYTHONLIBS_FOUND})
    target_link_libraries(pongo_agent ${PYTHON_LIBRARIES})
endif()

if(Boost_FOUND)
    target_link_libraries(pongo_agent ${Boost_LIBRARIES})
endif()

target_link_libraries(pongo_agent pthread zRPC archive crypto z)
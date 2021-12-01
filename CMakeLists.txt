cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set( CMAKE_CXX_FLAGS "-std=c++11" )
project(test_boost_log)
set(Boost_USE_STATIC_LIBS ON)
add_definitions(-DBOOST_LOG_DYN_LINK)
find_package(Threads REQUIRED)
find_package(Boost 1.54.0 REQUIRED COMPONENTS log)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# set(CMAKE_TOOLCHAIN_FILE robot.cmake)
# include_directories(
#         ${CMAKE_SYSROOT}/usr/include
#         ${CMAKE_SYSROOT}/usr/include/python2.7
#         ${CMAKE_SYSROOT}/usr/include/aarch64-linux-gnu/python2.7
#         ${CMAKE_SYSROOT}/usr/local/include
#         ${CMAKE_SYSROOT}/usr/local/sbin/zbar
# )
# link_directories(
#         ${CMAKE_SYSROOT}/usr/lib
#         ${CMAKE_SYSROOT}/usr/lib/python2.7/config-aarch64-linux-gnu
#         ${CMAKE_SYSROOT}/usr/local/lib64
#         ${CMAKE_SYSROOT}/usr/local/lib
# )
### kubot_utils ###
# add_library(utils_lib
#         # utils.h
#         json.hpp
#         log.cpp
#         log.h
#         )
set( CMAKE_CXX_FLAGS "-std=c++11" )
add_executable(test_boost_log test_boost_log.cpp log.cpp)
target_link_libraries(test_boost_log 
                        # utils_lib   
                        boost_log 
                        boost_log_setup 
                        boost_date_time
                        boost_filesystem
                        boost_system
                        boost_serialization
                        boost_thread
                        pthread
                        # ${CMAKE_THREAD_LIBS_INIT}
                        ${Boost_LIBRARIES} 
                        # Threads::Threads
                        )
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(test_boost_log)
set( CMAKE_CXX_FLAGS "-std=c++11" )
add_executable(test_boost_log test_boost_log.cpp)
target_link_libraries(test_boost_log    boost_log 
                                        boost_log_setup 
                                        boost_date_time
                                        boost_filesystem
                                        boost_system
                                        boost_serialization
                                        boost_thread)
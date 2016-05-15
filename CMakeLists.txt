cmake_minimum_required(VERSION 2.8.12)
project(indi_gphoto_ng_ccd)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

find_package(INDI REQUIRED)
include_directories(${INDI_INCLUDE_DIR})

add_subdirectory(INDIProperties/)
add_executable(indi_gphoto_ng_ccd gphoto_ccd.cpp)

target_link_libraries(indi_gphoto_ng_ccd indidriver)


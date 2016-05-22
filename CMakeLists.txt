cmake_minimum_required(VERSION 2.8.12)
project(indi_gphoto_ng_ccd)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

find_package(INDI REQUIRED)
include_directories(${INDI_INCLUDE_DIR})

add_subdirectory(INDIProperties/)
include_directories(${INDI_PROPERTIES_INCLUDE_DIRS})
add_subdirectory(libgphoto-cpp)
include_directories(${GPHOTO_CPP_INCLUDE_DIRS})
add_executable(indi_gphoto_ng_ccd gphoto_ccd.cpp realcamera.cpp simulationcamera.cpp)

target_link_libraries(indi_gphoto_ng_ccd indi_properties gphoto++ indidriver gphoto2 gphoto2_port pthread jpeg raw)

install(TARGETS indi_gphoto_ng_ccd RUNTIME DESTINATION bin )
install(FILES indi_gphoto_ng.xml DESTINATION ${INDI_DATA_DIR})
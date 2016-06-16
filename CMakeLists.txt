cmake_minimum_required(VERSION 2.8.12)
project(indi_gphoto_ng_ccd)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/gulinux-commons/cmake/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
find_package(INDI REQUIRED)
find_package(JPEG REQUIRED)
find_package(LibRaw REQUIRED)
find_package(GPHOTO2 REQUIRED)
include_directories(${INDI_INCLUDE_DIR} gulinux-commons/)

set(gphoto_ng_major 0)
set(gphoto_ng_minor 1)
set(gphoto_ng_patch 0)
set(gphoto_ng_version_str "${gphoto_ng_major}.${gphoto_ng_minor}.${gphoto_ng_patch}")
add_subdirectory(INDIProperties/)
include_directories(${INDI_PROPERTIES_INCLUDE_DIRS})
add_subdirectory(libgphoto-cpp)
include_directories(${GPHOTO_CPP_INCLUDE_DIRS})
add_executable(indi_gphoto_ng_ccd gphoto_ccd.cpp realcamera.cpp simulationcamera.cpp)

target_link_libraries(indi_gphoto_ng_ccd indi_properties gphoto++ ${INDI_DRIVER_LIBRARIES} ${Gphoto2_LIBRARIES} ${JPEG_LIBRARY} ${LIBRAW_LIBRARIES} pthread)

install(TARGETS indi_gphoto_ng_ccd RUNTIME DESTINATION bin )

option(INDI_PREV_XML "INDI <1.2.0 xml file" Off)
if(INDI_PREV_XML)
  configure_file(indi_gphoto_ng_old.xml ${CMAKE_CURRENT_BINARY_DIR}/indi_gphoto_ng.xml)
else(INDI_PREV_XML)
  configure_file(indi_gphoto_ng.xml ${CMAKE_CURRENT_BINARY_DIR}/indi_gphoto_ng.xml)
endif(INDI_PREV_XML)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/indi_gphoto_ng.xml DESTINATION ${INDI_DATA_DIR})


SET(CPACK_PACKAGE_VERSION_MAJOR "${gphoto_ng_major}")
SET(CPACK_PACKAGE_VERSION_MINOR "${gphoto_ng_minor}")
SET(CPACK_PACKAGE_VERSION_PATCH "${gphoto_ng_patch}")
SET(CPACK_GENERATOR DEB)
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Marco Gulino <marco@gulinux.net>")
SET(CPACK_PACKAGE_CONTACT "marco@gulinux.net")
set(UBUNTU_TARGET_VERSION "none" CACHE STRING "Target version for ubuntu package creation")
if(${UBUNTU_TARGET_VERSION} STREQUAL "wily")
  SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libjpeg8, libraw10, libindi1 (>= 1.2.0), libgphoto2-6")
endif(${UBUNTU_TARGET_VERSION} STREQUAL "wily")

if(${UBUNTU_TARGET_VERSION} STREQUAL "xenial")
  SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libjpeg9, libraw15, libindi1 (>= 1.2.0), libgphoto2-6")
endif(${UBUNTU_TARGET_VERSION} STREQUAL "xenial")
INCLUDE(CPack)
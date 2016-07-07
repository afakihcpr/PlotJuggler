project(SuperPlotterCmake)
cmake_minimum_required(VERSION 2.8)


find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5PrintSupport)
find_package(Qt5Concurrent)
find_package(Qt5Xml)
find_package(Qt5Charts)

set(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

include_directories(
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Concurrent_INCLUDE_DIRS}
    ${Qt5PrintSupport_INCLUDE_DIRS}
    ${Qt5Xml_INCLUDE_DIRS}
    ${Qt5Charts_INCLUDE_DIRS}
    )

include_directories( ./common ./color_widgets/include ./plotter_gui)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory( common)
add_subdirectory( color_widgets )
add_subdirectory( plotter_gui )

add_subdirectory( plugins/DataLoadCSV )
add_subdirectory( plugins/DataStreamSample )


SET ( ROS_DIRECTORY /opt/ros/kinetic/)
include_directories( ${ROS_DIRECTORY}/include  plugins/ros-type-parser)

add_subdirectory( plugins/ros-type-parser )
add_subdirectory( plugins/DataLoadROS )
add_subdirectory( plugins/DataStreamROS )


INSTALL(FILES plugins/statepublisher_base.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/SuperPlotter )
INSTALL(FILES plugins/dataloader_base.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/SuperPlotter )

SET(CMAKE_INSTALL_PREFIX  ${CMAKE_BINARY_DIR} )

#########################  create a package ####################################

SET(CPACK_GENERATOR "TGZ;DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Davide Faconti")
SET(CPACK_PACKAGE_CONTACT "faconti@icarustechnology.com")


SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local/")
SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SuperPlotter: juggle with data")
SET(CPACK_PACKAGE_VENDOR "Icarus Technology")

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "2")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_VERSION "0.2.0")

SET(CPACK_PACKAGE_NAME "superplotter${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

SET(CPACK_DEBIAN_PACKAGE_DEPENDS "qt5-default libqt5core5a libqt5concurrent5 libqt5gui5 libqt5opengl5 libqt5printsupport5 libqt5xml5")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "SuperPlotter")

INCLUDE(CPack)

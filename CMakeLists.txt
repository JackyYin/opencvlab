cmake_minimum_required(VERSION 2.8)
project( OpenCV_Lab )
include(ExternalProject)
include(FetchContent)

ExternalProject_Add(opencv
	PREFIX opencv
	GIT_REPOSITORY https://github.com/opencv/opencv.git
	GIT_TAG 4.7.0
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/opencv
)
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/opencv/include/opencv4 )

file(GLOB L4_SHARED_LIBRARYS
	${CMAKE_CURRENT_BINARY_DIR}/opencv/lib/*.dylib)

add_executable( bgcut bgcut.cpp )
target_link_libraries( bgcut ${L4_SHARED_LIBRARYS} )
set_target_properties( bgcut PROPERTIES CXX_STANDARD 14 )

cmake_minimum_required (VERSION 3.8)
project(DepthRendering)

## Required packages
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(OpenCV COMPONENTS imgcodecs REQUIRED)
find_package(OpenCV COMPONENTS highgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Boost COMPONENTS program_options)

add_executable(
	DepthRendering 
	Renderer.cpp
	glew.c
	main.cpp
	)
add_definitions(-DGLEW_STATIC)

target_include_directories(
	DepthRendering 
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	)

target_link_libraries(DepthRendering 
	PRIVATE
	${OPENGL_gl_LIBRARY}
	glfw
	${OpenCV_LIBRARIES}
	${Boost_LIBRARIES}
	)

## include
#declare_include_path("${OPENCV_DIR}/build/include")
#declare_include_path("${GLEW_DIR}/include")
#declare_include_path("${GLM_DIR}")
#declare_include_path("${GLFW_DIR}/include")

## link path
#declare_link_path("${OPENCV_DIR}/build/x64/vc14/lib")
#declare_link_path("${GLEW_DIR}/lib/Release/x64")
#declare_link_path("${GLFW_DIR}/lib-vc2015")

## libs
#list(APPEND LIBRARIES debug "opencv_world${OPENCV_VERSION}d.lib" 
#	optimized "opencv_world${OPENCV_VERSION}.lib")
#declare_lib("glew32s")
#declare_lib("glfw3")
#declare_lib("opengl32")

# the search paths
#include_directories(${INCLUDE_DIRS})
#link_directories(${LIBRARY_DIRS})
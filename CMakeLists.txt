cmake_minimum_required (VERSION 3.8)
project(DepthRendering)

## User-settable variables
set(OPENCV_DIR "D:/Program\ Files/OpenCV" CACHE PATH "root to opencv")
set(OPENCV_VERSION "310" CACHE STRING "opencv version")
set(GLEW_DIR "D:/Program\ Files/glew-2.0.0" CACHE PATH "root to glew")
set(GLFW_DIR "D:/Program\ Files/glfw-3.2.1-64" CACHE PATH "root to glfw")
set(GLM_DIR "D:/Program\ Files/glm" CACHE PATH "root to glm")
set(RENDER_TOOL_DIR "D:/CodeRepos/Tools" CACHE PATH "root to rendering tools")

set(INCLUDE_DIRS)
set(LIBRARY_DIRS)
set(LIBRARIES)

macro(declare_include_path include_path)
	list(APPEND INCLUDE_DIRS ${include_path})
endmacro()

macro(declare_link_path link_path)
	list(APPEND LIBRARY_DIRS ${link_path})
endmacro()

macro(declare_lib lib)
	list(APPEND LIBRARIES "${lib}.lib")
endmacro()


## include
declare_include_path("${OPENCV_DIR}/build/include")
declare_include_path("${GLEW_DIR}/include")
declare_include_path("${GLM_DIR}")
declare_include_path("${GLFW_DIR}/include")
declare_include_path("${RENDER_TOOL_DIR}")

## link path
declare_link_path("${OPENCV_DIR}/build/x64/vc14/lib")
declare_link_path("${GLEW_DIR}/lib/Release/x64")
declare_link_path("${GLFW_DIR}/lib-vc2015")

## libs
list(APPEND LIBRARIES debug "opencv_world${OPENCV_VERSION}d.lib" 
	optimized "opencv_world${OPENCV_VERSION}.lib")
declare_lib("glew32s")
declare_lib("glfw3")
declare_lib("opengl32")

# the search paths
include_directories(${INCLUDE_DIRS})
link_directories(${LIBRARY_DIRS})

add_executable(
	DepthRendering 
	Renderer.cpp
	main.cpp)
target_link_libraries(DepthRendering ${LIBRARIES})
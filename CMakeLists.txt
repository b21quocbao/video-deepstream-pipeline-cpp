
cmake_minimum_required(VERSION 3.10)

project(deepstream-test2) #Project name

set(CMAKE_CXX_STANDARD 14) #setting C++ 14 standard
find_package(PkgConfig) #finding pkg-config is a helper tool
find_package(CUDA REQUIRED)
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

#using pkg-config to getting Gstreamer
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

#including GStreamer header files directory
include_directories(
        ${GLIB_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
        "/opt/nvidia/deepstream/deepstream-5.1/sources/includes/"
)

#linking GStreamer library directory
link_directories(
        ${GLIB_LIBRARY_DIRS}
        ${GSTREAMER_LIBRARY_DIRS}
        "/opt/nvidia/deepstream/deepstream-5.1/lib/"
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

#building target executable
add_executable(${PROJECT_NAME} deepstream_test2_app.cpp)
#linking Gstreamer library with target executable
target_link_libraries(${PROJECT_NAME} ${GSTREAMER_LIBRARIES})
target_link_libraries(${PROJECT_NAME} "${TORCH_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} "nvdsgst_meta" "nvds_meta" "nvbufsurface" "nvbufsurftransform" "nvdsgst_helper" "nvds_batch_jpegenc")

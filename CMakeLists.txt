cmake_minimum_required(VERSION 3.0)

# set project name from current directory
get_filename_component(BASENAME ${CMAKE_SOURCE_DIR} NAME)
project(${BASENAME})

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Wall")

# define executable to build
include_directories("inc")
aux_source_directory(src V4L2CAMERA_SRC_FILES)
add_executable(${PROJECT_NAME} ${V4L2CAMERA_SRC_FILES})

# libv4l2rtspserver
add_subdirectory(v4l2rtspserver EXCLUDE_FROM_ALL)
target_link_libraries (${PROJECT_NAME} libv4l2rtspserver)

# libhttpjsonserver
add_subdirectory(libhttpjsonserver EXCLUDE_FROM_ALL)
target_link_libraries (${PROJECT_NAME} httpjsonserver)

# v4l2tools
include_directories("v4l2tools/include")

# install
install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

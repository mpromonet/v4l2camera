cmake_minimum_required(VERSION 3.0)

# set project name from current directory
get_filename_component(BASENAME ${CMAKE_SOURCE_DIR} NAME)
project(${BASENAME})

set (CMAKE_CXX_STANDARD 11)

find_package(Git)
if(GIT_FOUND)
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive)
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty OUTPUT_VARIABLE PROJECTVERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    set (VERSION "${PROJECTVERSION}/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
endif()

# define executable to build
include_directories("inc")
aux_source_directory(src V4L2CAMERA_SRC_FILES)
add_executable(${PROJECT_NAME} ${V4L2CAMERA_SRC_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE VERSION=\"${VERSION}\")

# libv4l2rtspserver
add_subdirectory(v4l2rtspserver EXCLUDE_FROM_ALL)
target_link_libraries (${PROJECT_NAME} libv4l2rtspserver)

# libhttpjsonserver
add_subdirectory(libhttpjsonserver EXCLUDE_FROM_ALL)
target_link_libraries (${PROJECT_NAME} httpjsonserver)

# v4l2tools
include_directories("v4l2tools/include")

# ui
add_custom_target(${PROJECT_NAME}-ui COMMAND npm install -C vuejs && npm run build -C vuejs)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-ui)

# install
install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install (DIRECTORY webroot DESTINATION data)

SET(CPACK_GENERATOR "TGZ")
SET(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
SET(CPACK_PACKAGE_CONTACT "michel.promonet@free.fr") 
if(PROJECTVERSION)
    SET(CPACK_PACKAGE_VERSION "${PROJECTVERSION}")
endif()
INCLUDE(CPack)

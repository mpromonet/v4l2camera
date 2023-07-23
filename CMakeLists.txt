cmake_minimum_required(VERSION 3.0)

option (WITH_COMPRESS "Enable compression" ON)
option (BUILD_UI "Build ui" ON)

project(v4l2camera)


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
if (WITH_COMPRESS)
	include_directories("v4l2tools/include")
	target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_COMPRESS=1)
endif()

# ui
if (BUILD_UI)
	add_custom_target(${PROJECT_NAME}-ui COMMAND npm install -C ${CMAKE_CURRENT_SOURCE_DIR}/vuejs && npm run build -C ${CMAKE_CURRENT_SOURCE_DIR}/vuejs)
	add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-ui)
endif()

# install
install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install (DIRECTORY vuejs/dist/ DESTINATION share/${PROJECT_NAME})

SET(CPACK_GENERATOR "TGZ")
SET(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
SET(CPACK_PACKAGE_CONTACT "michel.promonet@free.fr") 
if(PROJECTVERSION)
    SET(CPACK_PACKAGE_VERSION "${PROJECTVERSION}")
endif()
INCLUDE(CPack)

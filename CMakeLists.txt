cmake_minimum_required (VERSION 2.6)
project (motor CXX)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -g -Wall -Wextra -Wno-unused-parameter" )



if( WIN32 )
    set( PLATFORM_SOURCE "${PROJECT_SOURCE_DIR}/src/win" )
    add_definitions( -DGLEW_H=<glew.h> -DSDL2_H=<SDL.h>)
else()
    set( PLATFORM_SOURCE "${PROJECT_SOURCE_DIR}/src/posix" )
    add_definitions( -DGLEW_H=<GL/glew.h> -DSDL2_H=<SDL2/SDL.h> )
endif()

include_directories( "${PROJECT_SOURCE_DIR}/src/common" )
include_directories( "${PLATFORM_SOURCE}" )


file( GLOB_RECURSE src_common "${PROJECT_SOURCE_DIR}/src/common/*.*" )
file( GLOB_RECURSE src_platform "${PLATFORM_SOURCE}/*.*" )



add_executable( motor
	${src_platform}
	${src_common}
)

target_link_libraries( motor SDL2 GLEW GL )

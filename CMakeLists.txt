cmake_minimum_required (VERSION 2.6)
project (motor CXX)


set( DEPS_INCLUDE_DIR "" CACHE STRING "Where to look for include dependencies" )
set( DEPS_LIB_DIR "" CACHE STRING "Where to look for lib dependencies" )
set( DEPS_BIN_DIR "" CACHE STRING "Where to look for binary dependencies" )
if( NOT "${DEPS_INCLUDE_DIR}" STREQUAL "" )
    include_directories( "${DEPS_INCLUDE_DIR}" )
endif()
if( NOT "${DEPS_LIB_DIR}" STREQUAL "" )
    link_directories( "${DEPS_LIB_DIR}" )
endif()

if( WIN32 )
    #From http://stackoverflow.com/a/31423421/670358
    function(assign_source_group)
        foreach(_source IN ITEMS ${ARGN})
            if (IS_ABSOLUTE "${_source}")
                file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
            else()
                set(source_rel "${_source}")
            endif()
            get_filename_component(_source_path "${_source_rel}" PATH)
            string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
            source_group("${_source_path_msvc}" FILES "${_source}")
        endforeach()
    endfunction(assign_source_group)

    set( PLATFORM_SOURCE "${PROJECT_SOURCE_DIR}/src/win" )
    add_definitions( -DGLEW_H=<GL/glew.h> -DSDL2_H=<SDL2/SDL.h> )
    add_definitions( -DUNICODE -D_UNICODE )
    set( DEPS_LIB_NAMES glew32 SDL2 opengl32 )
    configure_file("${DEPS_BIN_DIR}/glew32.dll" "${PROJECT_BINARY_DIR}/Debug/glew32.dll" COPYONLY)
    configure_file("${DEPS_BIN_DIR}/glew32.dll" "${PROJECT_BINARY_DIR}/Release/glew32.dll" COPYONLY)
    configure_file("${DEPS_BIN_DIR}/SDL2.dll" "${PROJECT_BINARY_DIR}/Debug/SDL2.dll" COPYONLY)
    configure_file("${DEPS_BIN_DIR}/SDL2.dll" "${PROJECT_BINARY_DIR}/Release/SDL2.dll" COPYONLY)
else()
    set( PLATFORM_SOURCE "${PROJECT_SOURCE_DIR}/src/posix" )
    add_definitions( -DGLEW_H=<GL/glew.h> -DSDL2_H=<SDL2/SDL.h> )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -O2 -Wall -Wextra -Wno-unused-parameter" )
    set( DEPS_LIB_NAMES SDL2 GLEW GL )
    function(assign_source_group)
    endfunction(assign_source_group)
endif()

include_directories( "${PROJECT_SOURCE_DIR}/src/common" )
include_directories( "${PLATFORM_SOURCE}" )


file( GLOB_RECURSE src_common "${PROJECT_SOURCE_DIR}/src/common/*.*"  )
file( GLOB_RECURSE src_platform "${PLATFORM_SOURCE}/*.*"  )


add_executable( motor
	${src_platform}
	${src_common}
)

target_link_libraries( motor ${DEPS_LIB_NAMES} )

assign_source_group( ${src_common} ${src_platform} )

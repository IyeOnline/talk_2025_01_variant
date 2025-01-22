cmake_minimum_required( VERSION 3.22 )
project( TALK_stuff_framework )


add_executable( exe src/main.cpp )
target_include_directories( exe PUBLIC include )
target_compile_features( exe PUBLIC cxx_std_23 )
set_target_properties( exe PROPERTIES CXX_EXTENSIONS OFF )

find_package( fmt )
target_link_libraries( exe PUBLIC fmt::fmt )

set( TALK_SOURCE_LIST "" CACHE STRING "List of source files")
file(GLOB_RECURSE TALK_SOURCE_LIST CONFIGURE_DEPENDS src/*.cpp )
message( STATUS "collected sources:" )
foreach( _file ${TALK_SOURCE_LIST} )
	message( STATUS "\t${_file}" )
endforeach()
target_sources( exe PUBLIC ${TALK_SOURCE_LIST} )


set( TALK_SANITIZE True CACHE BOOL "Whether to add the sanitizer")
if ( TALK_SANITIZE )
	message(STATUS "Adding Sanitizer")
	target_compile_options( exe PUBLIC -fsanitize=address,undefined)
	target_link_options( exe PUBLIC -fsanitize=address,undefined)
endif ()

set( TALK_LIBSTDCPP_DEBUG True CACHE BOOL "Whether to debugging to libstdc++")
if ( TALK_LIBSTDCPP_DEBUG )
	message(STATUS "Adding libstdc++ debug checks")
	target_compile_definitions( exe PUBLIC D_GLIBCXX_DEBUG)
endif ()


set( TALK_ENABLE_WARNINGS True CACHE BOOL "Whether to enable warnings")
if ( TALK_ENABLE_WARNINGS )
	set( _TALK_CXX_WARN_FLAGS "" CACHE STRING "compiler (warning) flags")
	if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
		message( STATUS "Adding g++ & clang++ shared warning flags" )
		set( _TALK_CXX_WARN_FLAGS -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wzero-as-null-pointer-constant)
		set( _TALK_CXX_WARN_FLAGS ${_TALK_CXX_WARN_FLAGS} -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion -Wfloat-conversion -Wpessimizing-move )
		set( _TALK_CXX_WARN_FLAGS ${_TALK_CXX_WARN_FLAGS} -Wformat=2)
	endif ()

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		message(STATUS "Adding g++ specific warning flags")
		set(_TALK_CXX_WARN_FLAGS ${_TALK_CXX_WARN_FLAGS} -Werror=vla -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches)
		set(_TALK_CXX_WARN_FLAGS ${_TALK_CXX_WARN_FLAGS} -Wlogical-op -Wnull-dereference -Wno-aggressive-loop-optimizations )
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		message(STATUS "Adding clang specific warning flags")
		set(_TALK_CXX_WARN_FLAGS ${_TALK_CXX_WARN_FLAGS} -Wpointer-bool-conversion -Wempty-body -Wno-undefined-var-template )
	elseif( CMAKE_CXX_COMPILER_ID STREQUAL "Intel" )
		message(STATUS "Adding ICC warning warning flags")
		set(_TALK_CXX_WARN_FLAGS ${_TALK_CXX_WARN_FLAGS} -Weffc++ -W3)
	endif()

	target_compile_options( exe PUBLIC ${_TALK_CXX_WARN_FLAGS} )
endif()

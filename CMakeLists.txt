cmake_minimum_required( VERSION 3.15 )
project( x4cat VERSION 0.0.1 )

enable_language( CXX )
set( CMAKE_CONFIGURATION_TYPES "Debug;Release;Native" )

set( CMAKE_CXX_FLAGS         "-std=c++20" )
set( CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wextra -Wpedantic -Og -march=core2 -mtune=native -ggdb3" )
set( CMAKE_CXX_FLAGS_RELEASE "-w -O2 -march=core2 -mtune=generic -DNDEBUG" )
set( CMAKE_CXX_FLAGS_NATIVE  "-w -Ofast -march=native -mtune=native -DNDEBUG" )

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE Release )
endif()
message( "Using build configuration: ${CMAKE_BUILD_TYPE}" )

add_executable( x4encat "${CMAKE_SOURCE_DIR}/src/encat.cc" "${CMAKE_SOURCE_DIR}/src/md5.cc" )
install( TARGETS x4encat RUNTIME DESTINATION "bin" )

add_executable( x4decat "${CMAKE_SOURCE_DIR}/src/decat.cc" "${CMAKE_SOURCE_DIR}/src/md5.cc" )
install( TARGETS x4decat RUNTIME DESTINATION "bin" )

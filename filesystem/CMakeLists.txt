cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

include( "filesystem_stdio/filesystem_stdio.cmake" )
if( NOT ${IS_LINUX} )
include( "filesystem_stdio/filesystem_steam.cmake" )
endif()

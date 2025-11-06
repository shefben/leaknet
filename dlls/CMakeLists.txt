cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

foreach( GAME IN LISTS GAMES_TO_BUILD_LIST )
	include( "server_${GAME}.cmake" )
endforeach()


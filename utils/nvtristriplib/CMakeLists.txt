cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( NVTRISTRIP_SOURCE_FILES )
BEGIN_SRC( NVTRISTRIP_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"NvTriStrip.cpp"
			"NvTriStripObjects.cpp"
		#}
	)
END_SRC( NVTRISTRIP_SOURCE_FILES "Source Files" )

set( NVTRISTRIP_HEADER_FILES )
BEGIN_SRC( NVTRISTRIP_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"NvTriStrip.h"
			"NvTriStripObjects.h"
			"VertexCache.h"
		#}
	)
END_SRC( NVTRISTRIP_HEADER_FILES "Header Files" )

add_object(
	TARGET nvtristrip
	STATIC
	PROPERTY_FOLDER "Utils//Static Libs"
	SOURCES ${NVTRISTRIP_SOURCE_FILES} ${NVTRISTRIP_HEADER_FILES}
)
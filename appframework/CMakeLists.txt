cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( APPFRAMEWORK_SOURCE_FILES )
BEGIN_SRC( APPFRAMEWORK_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"
			"AppSystemGroup.cpp"
			"WinApp.cpp"
		#}
	)
END_SRC( APPFRAMEWORK_SOURCE_FILES "Source Files" )

set( APPFRAMEWORK_HEADER_FILES )
BEGIN_SRC( APPFRAMEWORK_HEADER_FILES "Interface" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/appframework/AppFramework.h"
			"${SRCDIR}/public/appframework/IAppSystem.h"
			"AppSystemGroup.h"
		#}
	)
END_SRC( APPFRAMEWORK_HEADER_FILES "Interface" )

add_object(
	TARGET appframework
	STATIC
	SOURCES ${APPFRAMEWORK_SOURCE_FILES} ${APPFRAMEWORK_HEADER_FILES}
)

target_include_directories(
	appframework PRIVATE

	"${SRCDIR}/appframework"
)

target_compile_definitions(
	appframework PRIVATE

	fopen=dont_use_fopen
)

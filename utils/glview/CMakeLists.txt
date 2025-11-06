cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( GLVIEW_SOURCE_FILES )
BEGIN_SRC( GLVIEW_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/mathlib.cpp"

			"glview.cpp"
		#}
	)

	SRC_GRP(
		SUBGROUP "common files"
		SOURCES
		#{
			"${SRCDIR}/utils/common/cmdlib.cpp"
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/filesystem_tools.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/physdll.cpp"
		#}
	)
END_SRC( GLVIEW_SOURCE_FILES "Source Files" )

set( GLVIEW_HEADER_FILES )
BEGIN_SRC( GLVIEW_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			#"../../../../../../../../../Program Files/Microsoft Visual Studio/VC98/Include/BASETSD.H"
			"${SRCDIR}/public/mathlib.h"

			"${SRCDIR}/utils/common/cmdlib.h"

			"glos.h"
		#}
	)
END_SRC( GLVIEW_HEADER_FILES "Header Files" )

add_object(
	TARGET glview
	EXECUTABLE WIN32
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${GLVIEW_SOURCE_FILES} ${GLVIEW_HEADER_FILES}
)

target_include_directories(
	glview PRIVATE

	"${SRCDIR}/utils/common"
)

target_link_libraries(
	glview PRIVATE

	"$<${IS_WINDOWS}:glu32>"
	"$<${IS_WINDOWS}:opengl32>"
	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
	"$<${IS_WINDOWS}:winmm>"
)

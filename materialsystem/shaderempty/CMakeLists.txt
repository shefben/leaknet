cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( SHADERAPIEMPTY_SOURCE_FILES )
BEGIN_SRC( SHADERAPIEMPTY_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.cpp"
			"shaderapiempty.cpp"
		#}
	)
END_SRC( SHADERAPIEMPTY_SOURCE_FILES "Source Files" )

set( SHADERAPIEMPTY_HEADER_FILES )
BEGIN_SRC( SHADERAPIEMPTY_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/tier0/platform.h"
		#}
	)
END_SRC( SHADERAPIEMPTY_HEADER_FILES "Header Files" )

add_object(
	TARGET shaderapiempty
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${SHADERAPIEMPTY_SOURCE_FILES} ${SHADERAPIEMPTY_HEADER_FILES}
)

target_include_directories(
	shaderapiempty PRIVATE

	"${SRCDIR}/materialsystem"
)

target_compile_definitions(
	shaderapiempty PRIVATE

	SHADER_DLL_EXPORT
	PROTECTED_THINGS_ENABLE
)

target_link_libraries(
	shaderapiempty PRIVATE
)
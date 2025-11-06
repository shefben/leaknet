cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VTFDIFF_SOURCE_FILES )
BEGIN_SRC( VTFDIFF_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/imageloader.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"

			"vtfdiff.cpp"
		#}
	)
END_SRC( VTFDIFF_SOURCE_FILES "Source Files" )

add_object(
	TARGET vtfdiff
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${VTFDIFF_SOURCE_FILES}
)

# @HACK(SanyaSho): TODO: Resolve SEH link error with s3tc.lib
target_link_options(
	vtfdiff PRIVATE

	/SAFESEH:NO
)

target_link_libraries(
	vtfdiff PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"

	vtf
)
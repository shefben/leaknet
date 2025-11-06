cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( STUDIORENDER_SOURCE_FILES )
BEGIN_SRC( STUDIORENDER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/imageloader.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/vmatrix.cpp"

			"cstudiorender.cpp"
			"flexrenderdata.cpp"
			"r_studio.cpp"
			"r_studiodecal.cpp"
			"r_studiodraw.cpp"
			"r_studioflex.cpp"
			"r_studiolight.cpp"
			"studiostats.cpp"
		#}
	)
END_SRC( STUDIORENDER_SOURCE_FILES "Source Files" )

set( STUDIORENDER_HEADER_FILES )
BEGIN_SRC( STUDIORENDER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/optimize.h"

			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/bspflags.h"
			"${SRCDIR}/public/clientstats.h"
			"${SRCDIR}/public/cmodel.h"
			"${SRCDIR}/public/convar.h"
			"${SRCDIR}/public/imageloader.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/istudiorender.h"
			"${SRCDIR}/public/mathlib.h"
			"${SRCDIR}/public/measure_section.h"
			"${SRCDIR}/public/model_types.h"
			"${SRCDIR}/public/pixelwriter.h"
			"${SRCDIR}/public/s3_intrf.h"
			"${SRCDIR}/public/studio.h"
			"${SRCDIR}/public/utllinkedlist.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vcollide.h"
			"${SRCDIR}/public/vector.h"
			"${SRCDIR}/public/vector2d.h"
			"${SRCDIR}/public/vector4d.h"

			"cstudiorender.h"
			"flexrenderdata.h"
			"studiostats.h"
		#}
	)
END_SRC( STUDIORENDER_HEADER_FILES "Header Files" )

add_object(
	TARGET studiorender
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${STUDIORENDER_SOURCE_FILES} ${STUDIORENDER_HEADER_FILES}
)

target_compile_definitions(
	studiorender PRIVATE

	"$<$<CONFIG:Debug>:fopen=dont_use_fopen>"

	IMAGE_LOADER_NO_DXTC
	STUDIORENDER_EXPORTS
	PROTECTED_THINGS_ENABLE
)

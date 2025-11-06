cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( MDL35DECOMPIL_SOURCE_FILES )
BEGIN_SRC( MDL35DECOMPIL_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/engine/CSharedModelLoader.cpp"

			"${SRCDIR}/game_shared/bone_setup.cpp"

			"${SRCDIR}/public/interface.cpp" # SanyaSho: added this

			"${SRCDIR}/public/collisionutils.cpp"
			"${SRCDIR}/public/mathlib.cpp"

			"mdl35decompil.cpp"
			"vtfconvert.cpp"

    		"TODO.txt"
		#}
	)
END_SRC( MDL35DECOMPIL_SOURCE_FILES "Source Files" )

set( MDL35DECOMPIL_HEADER_FILES )
BEGIN_SRC( MDL35DECOMPIL_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/engine/CSharedModelLoader.h"

			"${SRCDIR}/game_shared/bone_setup.h"

			"${SRCDIR}/public/interface.h" # SanyaSho: added this

			"${SRCDIR}/public/collisionutils.h"
			"${SRCDIR}/public/engine/ISharedModelLoader.h"
			"${SRCDIR}/public/mathlib.h"
			"${SRCDIR}/public/studio.h"

			"vtfconvert.h"
		#}
	)
END_SRC( MDL35DECOMPIL_HEADER_FILES "Header Files" )

add_object(
	TARGET mdl35decompil
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${MDL35DECOMPIL_SOURCE_FILES} ${MDL35DECOMPIL_HEADER_FILES}
)

target_include_directories(
	mdl35decompil PRIVATE

	"${SRCDIR}/public/engine"
	"${SRCDIR}/engine"
	"${SRCDIR}/game_shared"
	"${SRCDIR}/utils/common"
)

# ?????????????
#target_link_libraries(
#	mdl35decompil PRIVATE
#
#	havana_constraints
#	"${LIBCOMMON}/havok_mopp.lib"
#	hk_base
#	hk_math
#	ivp_compact_builder
#	ivp_physics
#	mxtk
#	"${LIBCOMMON}/s3tc.lib"
#	"${LIBCOMMON}/Steam.lib"
#	"${LIBCOMMON}/vtuneapi.lib"
#	appframework
#	"${LIBPUBLIC}/expdlln.lib"
#	game_controls
#	nvtristrip
#	shaderlib
#	tier0
#	"${LIBPUBLIC}/trktooln.lib"
#	unitlib
#	vgui_controls
#	vgui_surfacelib
#	vstdlib
#	vtf
#	vmpi
#)
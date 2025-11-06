cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( STUDIOMDL_SOURCE_FILES )
BEGIN_SRC( STUDIOMDL_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/game_shared/bone_setup.cpp"

			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/collisionutils.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/FileSystem_Tools.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/physdll.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"

			"${SRCDIR}/utils/common/cmdlib.cpp"
			"${SRCDIR}/utils/common/mstristrip.cpp"
			"${SRCDIR}/utils/common/scriplib.cpp"

			"collisionmodel.cpp"
			"HardwareMatrixState.cpp"
			"HardwareVertexCache.cpp"
			"matsys.cpp"
			"mrmsupport.cpp"
			"optimize.cpp"
			"perfstats.cpp"
			"simplify.cpp"
			"studiomdl.cpp"
			"UnifyLODs.cpp"
			"v1support.cpp"
			"write.cpp"
		#}
	)
END_SRC( STUDIOMDL_SOURCE_FILES "Source Files" )

set( STUDIOMDL_HEADER_FILES )
BEGIN_SRC( STUDIOMDL_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/optimize.h"

			"${SRCDIR}/game_shared/bone_setup.h"

			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basehandle.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/bspflags.h"
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/cmodel.h"
			"${SRCDIR}/public/collisionutils.h"
			"${SRCDIR}/public/commonmacros.h"
			"${SRCDIR}/public/const.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/FileSystem_Tools.h"
			"${SRCDIR}/public/gametrace.h"
			"${SRCDIR}/public/ihandleentity.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/istudiorender.h"
			"${SRCDIR}/public/materialsystem/imaterial.h"
			"${SRCDIR}/public/materialsystem/imaterialsystem.h"
			"${SRCDIR}/public/materialsystem/imaterialvar.h"
			"${SRCDIR}/public/materialsystem/materialsystem_config.h"
			"${SRCDIR}/public/mathlib.h"
			"${SRCDIR}/public/phyfile.h"
			"${SRCDIR}/public/physdll.h"
			"${SRCDIR}/public/protected_things.h"
			"${SRCDIR}/public/string_t.h"
			"${SRCDIR}/public/studio.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/tier0/vprof.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utllinkedlist.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/utlsymbol.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vcollide.h"
			"${SRCDIR}/public/vcollide_parse.h"
			"${SRCDIR}/public/vector.h"
			"${SRCDIR}/public/vector2d.h"
			"${SRCDIR}/public/vector4d.h"
			"${SRCDIR}/public/vphysics/constraints.h"
			"${SRCDIR}/public/vphysics_interface.h"
			"${SRCDIR}/public/vstdlib/random.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"

			"${SRCDIR}/utils/common/cmdlib.h"
			"${SRCDIR}/utils/common/mstristrip.h"
			"${SRCDIR}/utils/common/scriplib.h"

			"${SRCDIR}/utils/NvTriStripLib/NvTriStrip.h"

			"collisionmodel.h"
			"FileBuffer.h"
			"HardwareMatrixState.h"
			"HardwareVertexCache.h"
			"matsys.h"
			"perfstats.h"
			"studiomdl.h"
		#}
	)
END_SRC( STUDIOMDL_HEADER_FILES "Header Files" )

add_object(
	TARGET studiomdl
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${STUDIOMDL_SOURCE_FILES} ${STUDIOMDL_HEADER_FILES}
)

target_compile_definitions(
	studiomdl PRIVATE

	PROTECTED_THINGS_DISABLE
)

target_include_directories(
	studiomdl PRIVATE

	"." # cbase.h
	"${SRCDIR}/utils/common"
	"${SRCDIR}/utils/nvtristriplib"
	"${SRCDIR}/game_shared"
)

target_link_libraries(
	studiomdl PRIVATE

	nvtristrip
)

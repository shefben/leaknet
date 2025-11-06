cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

# @TODO(SanyaSho): Add more source groups

set( VBSP_SOURCE_FILES )
BEGIN_SRC( VBSP_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/builddisp.cpp"
			"${SRCDIR}/common/disp_common.cpp"
			"${SRCDIR}/common/disp_powerinfo.cpp"

			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/checksum_crc.cpp"
			"${SRCDIR}/public/ChunkFile.cpp"
			"${SRCDIR}/public/collisionutils.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/filesystem_tools.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/keyvalues.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/mempool.cpp"
			"${SRCDIR}/public/physdll.cpp"
			"${SRCDIR}/public/scratchpad3d.cpp"
			"${SRCDIR}/public/TokenReader.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"
			"${SRCDIR}/public/vmatrix.cpp"

			"${SRCDIR}/utils/common/bsplib.cpp"
			"${SRCDIR}/utils/common/cmdlib.cpp"
			"${SRCDIR}/utils/common/map_shared.cpp"
			"${SRCDIR}/utils/common/mstristrip.cpp"
			"${SRCDIR}/utils/common/pacifier.cpp"
			"${SRCDIR}/utils/common/polylib.cpp"
			"${SRCDIR}/utils/common/scriplib.cpp"
			"${SRCDIR}/utils/common/threads.cpp"
			"${SRCDIR}/utils/common/utilmatlib.cpp"

			"brushbsp.cpp"
			"csg.cpp"
			"cubemap.cpp"
			"detail.cpp"
			"detailObjects.cpp"
			"disp_ivp.cpp"
			"disp_lightmap_alpha.cpp"
			"disp_vbsp.cpp"
			"faces.cpp"
			"glfile.cpp"
			"ivp.cpp"
			"leakfile.cpp"
			"map.cpp"
			"materialpatch.cpp"
			"nodraw.cpp"
			"normals.cpp"
			"overlay.cpp"
			"portals.cpp"
			"prtfile.cpp"
			"StaticProp.cpp"
			"textures.cpp"
			"tree.cpp"
			"vbsp.cpp"
			"writebsp.cpp"

    		"notes.txt"
		#}
	)
END_SRC( VBSP_SOURCE_FILES "Source Files" )

set( VBSP_HEADER_FILES )
BEGIN_SRC( VBSP_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/builddisp.h"
			"${SRCDIR}/common/disp_common.h"
			"${SRCDIR}/common/disp_powerinfo.h"
			"${SRCDIR}/common/disp_vertindex.h"

			"${SRCDIR}/public/arraystack.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/bspfile.h"
			"${SRCDIR}/public/bspflags.h"
			"${SRCDIR}/public/bsptreedata.h"
			"${SRCDIR}/public/bumpvects.h"
			"${SRCDIR}/public/ChunkFile.h"
			"${SRCDIR}/public/cmodel.h"
			"${SRCDIR}/public/collisionutils.h"
			"${SRCDIR}/public/commonmacros.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/FileSystem_Tools.h"
			"${SRCDIR}/public/gamebspfile.h"
			"${SRCDIR}/public/imageloader.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/iscratchpad3d.h"
			"${SRCDIR}/public/mathlib.h"
			"${SRCDIR}/public/nmatrix.h"
			"${SRCDIR}/public/NTree.h"
			"${SRCDIR}/public/nvector.h"
			"${SRCDIR}/public/phyfile.h"
			"${SRCDIR}/public/physdll.h"
			"${SRCDIR}/public/plane.h"
			"${SRCDIR}/public/scratchpad3d.h"
			"${SRCDIR}/public/studio.h"
			"${SRCDIR}/public/TokenReader.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utllinkedlist.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/utlsymbol.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vcollide.h"
			"${SRCDIR}/public/vector.h"
			"${SRCDIR}/public/vector2d.h"
			"${SRCDIR}/public/vector4d.h"
			"${SRCDIR}/public/vmatrix.h"
			"${SRCDIR}/public/vphysics_interface.h"
			"${SRCDIR}/public/vplane.h"
			"${SRCDIR}/public/wadtypes.h"
			"${SRCDIR}/public/worldsize.h"

			"${SRCDIR}/utils/common/bsplib.h"
			"${SRCDIR}/utils/common/cmdlib.h"
			"${SRCDIR}/utils/common/map_shared.h"
			"${SRCDIR}/utils/common/mstristrip.h"
			"${SRCDIR}/utils/common/pacifier.h"
			"${SRCDIR}/utils/common/polylib.h"
			"${SRCDIR}/utils/common/qfiles.h"
			"${SRCDIR}/utils/common/scriplib.h"
			"${SRCDIR}/utils/common/threads.h"
			"${SRCDIR}/utils/common/utilmatlib.h"
			"${SRCDIR}/utils/vmpi/vmpi.h"

			"detail.h"
			"disp_ivp.h"
			"disp_lightmap_alpha.h"
			"disp_vbsp.h"
			"faces.h"
			"ivp.h"
			"materialpatch.h"
			"vbsp.h"
			"writebsp.h"
		#}
	)
END_SRC( VBSP_HEADER_FILES "Header Files" )

add_object(
	TARGET vbsp
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${VBSP_SOURCE_FILES} ${VBSP_HEADER_FILES}
)

target_include_directories(
	vbsp PRIVATE

	"${SRCDIR}/utils/common"
	"${SRCDIR}/utils/vmpi"
)

target_compile_definitions(
	vbsp PRIVATE

	MACRO_MATHLIB
	PROTECTED_THINGS_DISABLE
)

# @HACK(SanyaSho): TODO: Resolve SEH link error with s3tc.lib
target_link_options(
	vbsp PRIVATE

	/SAFESEH:NO
)

target_link_libraries(
	vbsp PRIVATE

	vtf
)

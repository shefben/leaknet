cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VRAD_DLL_SOURCE_FILES )
BEGIN_SRC( VRAD_DLL_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/builddisp.cpp"
			"${SRCDIR}/common/dispcoll_common.cpp"
			"${SRCDIR}/common/disp_common.cpp"
			"${SRCDIR}/common/disp_powerinfo.cpp"

			"${SRCDIR}/public/bitbuf.cpp"
			"${SRCDIR}/public/bsptreedata.cpp"
			"${SRCDIR}/public/bumpvects.cpp"
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/checksum_crc.cpp"
			"${SRCDIR}/public/checksum_md5.cpp"
			"${SRCDIR}/public/ChunkFile.cpp"
			"${SRCDIR}/public/collisionutils.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/imageloader.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/physdll.cpp"
			"${SRCDIR}/public/scratchpad3d.cpp"
			"${SRCDIR}/public/ScratchPadUtils.cpp"
			"${SRCDIR}/public/tgaloader.cpp"
			"${SRCDIR}/public/TokenReader.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"
			"${SRCDIR}/public/vmatrix.cpp"

			"${SRCDIR}/utils/common/bsplib.cpp"
			"${SRCDIR}/utils/common/cmdlib.cpp"
			"${SRCDIR}/utils/common/map_shared.cpp"
			"${SRCDIR}/utils/common/mpi_stats.cpp"
			"${SRCDIR}/utils/common/MySqlDatabase.cpp"
			"${SRCDIR}/utils/common/pacifier.cpp"
			"${SRCDIR}/utils/common/polylib.cpp"
			"${SRCDIR}/utils/common/scriplib.cpp"
			"${SRCDIR}/utils/common/threads.cpp"
			"${SRCDIR}/utils/common/utilmatlib.cpp"
			"${SRCDIR}/utils/common/vmpi_tools_shared.cpp"

			"${SRCDIR}/utils/vmpi/mysql_wrapper.cpp"
			"${SRCDIR}/utils/vmpi/vmpi_distribute_work.cpp"

			"disp_vrad.cpp"
			"incremental.cpp"
			"lightmap.cpp"
			"macro_texture.cpp"
			"mpivrad.cpp"
			"radial.cpp"
			"SampleHash.cpp"
			"trace.cpp"
			"vismat.cpp"
			"vrad.cpp"
			"VradDetailProps.cpp"
			"VRadDisps.cpp"
			"vraddll.cpp"
			"VRadStaticProps.cpp"
			"VRAD_DispColl.cpp"

			"notes.txt"
		#}
	)
END_SRC( VRAD_DLL_SOURCE_FILES "Source Files" )

set( VRAD_DLL_HEADER_FILES )
BEGIN_SRC( VRAD_DLL_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/builddisp.h"
			"${SRCDIR}/common/dispcoll_common.h"
			"${SRCDIR}/common/disp_common.h"
			"${SRCDIR}/common/disp_powerinfo.h"
			"${SRCDIR}/common/disp_vertindex.h"
			"${SRCDIR}/common/ivraddll.h"
			"${SRCDIR}/common/optimize.h"

			"${SRCDIR}/public/anorms.h"
			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/bitbuf.h"
			"${SRCDIR}/public/bitvec.h"
			"${SRCDIR}/public/bspfile.h"
			"${SRCDIR}/public/bspflags.h"
			"${SRCDIR}/public/bsptreedata.h"
			"${SRCDIR}/public/bumpvects.h"
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/checksum_crc.h"
			"${SRCDIR}/public/checksum_md5.h"
			"${SRCDIR}/public/ChunkFile.h"
			"${SRCDIR}/public/cmodel.h"
			"${SRCDIR}/public/collisionutils.h"
			"${SRCDIR}/public/commonmacros.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/gamebspfile.h"
			"${SRCDIR}/public/imageloader.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/iscratchpad3d.h"
			"${SRCDIR}/public/mathlib.h"
			"${SRCDIR}/public/mpi/mpi.h"
			"${SRCDIR}/public/mpi/mpidefs.h"
			"${SRCDIR}/public/mpi/mpio.h"
			"${SRCDIR}/public/mpi/mpi_errno.h"
			"${SRCDIR}/public/phyfile.h"
			"${SRCDIR}/public/physdll.h"
			"${SRCDIR}/public/plane.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/scratchpad3d.h"
			"${SRCDIR}/public/ScratchPadUtils.h"
			"${SRCDIR}/public/studio.h"
			"${SRCDIR}/public/terrainmod.h"
			"${SRCDIR}/public/tgaloader.h"
			"${SRCDIR}/public/TokenReader.h"
			"${SRCDIR}/public/trace.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utldict.h"
			"${SRCDIR}/public/UtlHash.h"
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
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"
			"${SRCDIR}/public/vtf/vtf.h"
			"${SRCDIR}/public/wadtypes.h"
			"${SRCDIR}/public/worldsize.h"

			"${SRCDIR}/utils/common/bsplib.h"
			"${SRCDIR}/utils/common/cmdlib.h"
			"${SRCDIR}/utils/common/ISQLDBReplyTarget.h"
			"${SRCDIR}/utils/common/map_shared.h"
			"${SRCDIR}/utils/vmpi/messagemgr.h"
			"${SRCDIR}/utils/common/mpi_stats.h"
			"${SRCDIR}/utils/common/MySqlDatabase.h"
			"${SRCDIR}/utils/common/pacifier.h"
			"${SRCDIR}/utils/common/polylib.h"
			"${SRCDIR}/utils/common/scriplib.h"
			"${SRCDIR}/utils/vmpi/tcpsocket.h"
			"${SRCDIR}/utils/vmpi/threadhelpers.h"
			"${SRCDIR}/utils/common/threads.h"
			"${SRCDIR}/utils/common/vmpi_tools_shared.h"

			"${SRCDIR}/utils/vmpi/messbuf.h"
			"${SRCDIR}/utils/vmpi/mysql_wrapper.h"
			"${SRCDIR}/utils/vmpi/vmpi_defs.h"
			"${SRCDIR}/utils/vmpi/vmpi_dispatch.h"
			"${SRCDIR}/utils/vmpi/vmpi_distribute_work.h"
			"${SRCDIR}/utils/vmpi/vmpi_filesystem.h"

			"disp_vrad.h"
			"iincremental.h"
			"incremental.h"
			"lightmap.h"
			"macro_texture.h"
			"mpivrad.h"
			"radial.h"
			"vismat.h"
			"vrad.h"
			"vraddll.h"
			"VRAD_DispColl.h"
		#}
	)
END_SRC( VRAD_DLL_HEADER_FILES "Header Files" )

add_object(
	TARGET vrad_dll
	MODULE
	PROPERTY_FOLDER "Utils//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${VRAD_DLL_SOURCE_FILES} ${VRAD_DLL_HEADER_FILES}
)

target_include_directories(
	vrad_dll PRIVATE

	"${SRCDIR}/utils/common"
	"${SRCDIR}/utils/vmpi"
	"${SRCDIR}/utils/vmpi/mysql/mysqlqq/include"
	"${SRCDIR}/utils/vmpi/mysql/include"
)

target_compile_definitions(
	vrad_dll PRIVATE

	MPI
	PROTECTED_THINGS_DISABLE
)

target_link_libraries(
	vrad_dll PRIVATE

	"$<${IS_WINDOWS}:ws2_32>"

	"$<${IS_WINDOWS}:${LIBCOMMON}/s3tc.lib>"
	vtf
	vmpi
	"$<${IS_WINDOWS}:${SRCDIR}/utils/vmpi/mysql/lib/opt/libmySQL.lib>"
)

install( FILES "${SRCDIR}/utils/vmpi/mysql/lib/opt/libmySQL.dll" DESTINATION "${GAMEDIR}/bin" )

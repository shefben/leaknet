cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VVIS_DLL_SOURCE_FILES )
BEGIN_SRC( VVIS_DLL_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/checksum_crc.cpp"
			"${SRCDIR}/public/checksum_md5.cpp"
			"${SRCDIR}/public/collisionutils.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"${SRCDIR}/utils/common/bsplib.cpp"
			"${SRCDIR}/utils/common/cmdlib.cpp"
			"${SRCDIR}/utils/common/mpi_stats.cpp"
			"${SRCDIR}/utils/common/MySqlDatabase.cpp"
			"${SRCDIR}/utils/common/pacifier.cpp"
			"${SRCDIR}/utils/common/scriplib.cpp"
			"${SRCDIR}/utils/common/threads.cpp"
			"${SRCDIR}/utils/common/vmpi_tools_shared.cpp"

			"${SRCDIR}/utils/vmpi/mysql_wrapper.cpp"
			"${SRCDIR}/utils/vmpi/vmpi_distribute_work.cpp"

			"flow.cpp"
			"mpivis.cpp"
			"vvis.cpp"
			"WaterDist.cpp"
		#}
	)
END_SRC( VVIS_DLL_SOURCE_FILES "Source Files" )

set( VVIS_DLL_HEADER_FILES )
BEGIN_SRC( VVIS_DLL_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/bspfile.h"
			"${SRCDIR}/public/bspflags.h"
			"${SRCDIR}/public/bsptreedata.h"
			"${SRCDIR}/public/bumpvects.h"
			"${SRCDIR}/public/checksum_crc.h"
			"${SRCDIR}/public/checksum_md5.h"
			"${SRCDIR}/public/cmodel.h"
			"${SRCDIR}/public/commonmacros.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/gamebspfile.h"
			"${SRCDIR}/public/mathlib.h"
			"${SRCDIR}/public/mpi/mpi.h"
			"${SRCDIR}/public/mpi/mpidefs.h"
			"${SRCDIR}/public/mpi/mpio.h"
			"${SRCDIR}/public/mpi/mpi_errno.h"
			"${SRCDIR}/public/plane.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utllinkedlist.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/utlsymbol.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vcollide.h"
			"${SRCDIR}/public/vector.h"
			"${SRCDIR}/public/vector2d.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"
			"${SRCDIR}/public/wadtypes.h"

			"${SRCDIR}/utils/common/bsplib.h"
			"${SRCDIR}/utils/common/cmdlib.h"
			"${SRCDIR}/utils/common/ISQLDBReplyTarget.h"
			"${SRCDIR}/utils/common/ivvisdll.h"
			"${SRCDIR}/utils/common/MySqlDatabase.h"
			"${SRCDIR}/utils/common/pacifier.h"
			"${SRCDIR}/utils/common/scriplib.h"
			"${SRCDIR}/utils/common/threads.h"

			"${SRCDIR}/utils/vmpi/vmpi_distribute_work.h"

			"mpivis.h"
			"vis.h"
		#}
	)
END_SRC( VVIS_DLL_HEADER_FILES "Header Files" )

add_object(
	TARGET vvis_dll
	MODULE
	PROPERTY_FOLDER "Utils//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${VVIS_DLL_SOURCE_FILES} ${VVIS_DLL_HEADER_FILES}
)

target_include_directories(
	vvis_dll PRIVATE

	"${SRCDIR}/utils/common"
	"${SRCDIR}/utils/vmpi"
	"${SRCDIR}/utils/vmpi/mysql/include"
)

target_compile_definitions(
	vvis_dll PRIVATE

	MPI
	PROTECTED_THINGS_DISABLE
)

target_link_libraries(
	vvis_dll PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
	"$<${IS_WINDOWS}:ws2_32>"

	vmpi

	"${SRCDIR}/utils/vmpi/mysql/lib/opt/libmySQL.lib"
)

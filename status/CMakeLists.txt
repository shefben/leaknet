cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( STATUS_SOURCE_FILES )
BEGIN_SRC( STATUS_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/filesystem_tools.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/keyvalues.cpp"
			"${SRCDIR}/public/mempool.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"${SRCDIR}/utils/common/cmdlib.cpp"

			"AddEmailDialog.cpp"
			"BaseDlg.cpp"
			"EmailDialog.cpp"
			"EmailManager.cpp"
			"Graph.cpp"
			"LoadDialog.cpp"
			"MessageBuffer.cpp"
			"mod.cpp"
			"ModInfoSocket.cpp"
			"ODStatic.cpp"
			"OptionsDlg.cpp"
			"ProjectionWnd.cpp"
			"SchedulerDialog.cpp"
			"Status.cpp"
			"StatusDlg.cpp"
			"StatusWindow.cpp"
			"StdAfx.cpp"
			"util.cpp"
			"WarnDialog.cpp"

			"Status.rc"
			"res/Status.ico"
			"res/Status.rc2"
			"todo.txt"
		#}
	)
END_SRC( STATUS_SOURCE_FILES "Source Files" )

set( STATUS_HEADER_FILES )
BEGIN_SRC( STATUS_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/proto_oob.h"
			"${SRCDIR}/common/proto_version.h"

			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/Color.h"
			"${SRCDIR}/public/commonmacros.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/filesystem_tools.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/keyvalues.h"
			"${SRCDIR}/public/mempool.h"
			"${SRCDIR}/public/protected_things.h"
			"${SRCDIR}/public/string_t.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utllinkedlist.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/utlsymbol.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"

			"${SRCDIR}/utils/common/cmdlib.h"

			"AddEmailDialog.h"
			"BaseDlg.h"
			"EmailDialog.h"
			"EmailManager.h"
			"Graph.h"
			"HLAsyncSocket.h"
			"LoadDialog.h"
			"MessageBuffer.h"
			"mod.h"
			"ModInfoSocket.h"
			"modupdate.h"
			"ODStatic.h"
			"OptionsDlg.h"
			"ProjectionWnd.h"
			"resource.h"
			"Scheduler.h"
			"SchedulerDialog.h"
			"Status.h"
			"StatusDlg.h"
			"StatusWindow.h"
			"status_colors.h"
			"status_protocol.h"
			"StdAfx.h"
			"util.h"
			"WarnDialog.h"
		#}
	)
END_SRC( STATUS_HEADER_FILES "Header Files" )

add_object(
	TARGET status
	EXECUTABLE WIN32
	INSTALL_DEST "${GAMEDIR}/bin"
	INSTALL_OUTNAME "Status"
	PROPERTY_FOLDER "Utils//Executables"
	SOURCES ${STATUS_SOURCE_FILES} ${STATUS_HEADER_FILES}
)

target_include_directories(
	status PRIVATE

	"${SRCDIR}/utils/common"
)

target_compile_definitions(
	status PRIVATE

	LAUNCHERONLY
	PROTECTED_THINGS_DISABLE
)

target_link_libraries(
	status PRIVATE

	"$<${IS_WINDOWS}:winmm>"
	"$<${IS_WINDOWS}:rpcrt4>"

	"$<$<AND:$<CONFIG:Release>,${IS_WINDOWS}>:nafxcw>"
	"$<$<AND:$<CONFIG:Release>,${IS_WINDOWS}>:Libcmt>"
	"$<$<AND:$<CONFIG:Debug>,${IS_WINDOWS}>:nafxcwd>"
	"$<$<AND:$<CONFIG:Debug>,${IS_WINDOWS}>:Libcmtd>"
)

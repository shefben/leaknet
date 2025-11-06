cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VGUI_DLL_SOURCE_FILES )
BEGIN_SRC( VGUI_DLL_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/keyvalues.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"Bitmap.cpp"
			"Border.cpp"
			"fileimage.cpp"
			"HtmlWindow.cpp"
			"InputWin32.cpp"
			"LocalizedStringTable.cpp"
			"MemoryBitmap.cpp"
			"MessageListener.cpp"
			"Scheme.cpp"
			"Surface.cpp"
			"System.cpp"
			"vgui.cpp"
			"vgui_internal.cpp"
			"vgui_key_translation.cpp"
			"VPanel.cpp"
			"VPanelWrapper.cpp"
		#}
	)
END_SRC( VGUI_DLL_SOURCE_FILES "Source Files" )

set( VGUI_DLL_HEADER_FILES )
BEGIN_SRC( VGUI_DLL_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/SteamBootStrapper.h"
			"${SRCDIR}/common/vgui/HtmlWindow.h"
			"${SRCDIR}/common/vgui_surfacelib/FontAmalgam.h"
			"${SRCDIR}/common/vgui_surfacelib/FontManager.h"
			"${SRCDIR}/common/vgui_surfacelib/Win32Font.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/Color.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/filesystem.h"
			#"${SRCDIR}/public/Ikeyvalues.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utllinkedlist.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlpriorityqueue.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vector2d.h"
			#"${SRCDIR}/public/vgui/keyvalues.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"

			"bitmap.h"
			"fileimage.h"
			#"IInputInternal.h"
			"IMessageListener.h"
			"Memorybitmap.h"
			"vgui_internal.h"
			"VPanel.h"
		#}
	)
END_SRC( VGUI_DLL_HEADER_FILES "Header Files" )

BEGIN_SRC( VGUI_DLL_HEADER_FILES "Interfaces" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/vgui/IBorder.h"
			"${SRCDIR}/public/vgui/IClientPanel.h"
			"${SRCDIR}/public/vgui/IHTML.h"
			"${SRCDIR}/public/vgui/IImage.h"
			"${SRCDIR}/public/vgui/IInput.h"
			"${SRCDIR}/public/vgui/ILocalize.h"
			"${SRCDIR}/public/vgui/IPanel.h"
			"${SRCDIR}/public/vgui/IScheme.h"
			"${SRCDIR}/public/vgui/ISurface.h"
			"${SRCDIR}/public/vgui/ISystem.h"
			"${SRCDIR}/public/vgui/IVGui.h"
			"VGUI_Border.h"
		#}
	)
END_SRC( VGUI_DLL_HEADER_FILES "Interfaces" )

add_object(
	TARGET vgui_dll
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	INSTALL_OUTNAME "vgui2"
	SOURCES ${VGUI_DLL_SOURCE_FILES} ${VGUI_DLL_HEADER_FILES}
)

target_include_directories(
	vgui_dll PRIVATE
)

target_compile_definitions(
	vgui_dll PRIVATE

	DONT_PROTECT_FILEIO_FUNCTIONS
)

target_link_libraries(
	vgui_dll PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
	"$<${IS_WINDOWS}:winmm>"

	vgui_surfacelib
)


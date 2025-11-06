cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VGUIMATSURFACE_SOURCE_FILES )
BEGIN_SRC( VGUIMATSURFACE_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/checksum_crc.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/vmatrix.cpp"
			"${SRCDIR}/vgui2/src/htmlwindow.cpp"
			"${SRCDIR}/vgui2/src/MemoryBitmap.cpp"
			"${SRCDIR}/vgui2/src/vgui_key_translation.cpp"

			"Clip2D.cpp"
			"Cursor.cpp"
			"FontTextureCache.cpp"
			"Input.cpp"
			"MatSystemSurface.cpp"
			"TextureDictionary.cpp"
			"vguimatsurfacestats.cpp"
		#}
	)
END_SRC( VGUIMATSURFACE_SOURCE_FILES "Source Files" )

set( VGUIMATSURFACE_HEADER_FILES )
BEGIN_SRC( VGUIMATSURFACE_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/vgui/HtmlWindow.h"
			"${SRCDIR}/common/vgui_surfacelib/FontAmalgam.h"
			"${SRCDIR}/common/vgui_surfacelib/FontManager.h"
			"${SRCDIR}/common/vgui_surfacelib/Win32Font.h"

			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/checksum_crc.h"
			"${SRCDIR}/public/commonmacros.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/icreatevfont.h"
			"${SRCDIR}/public/imageloader.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/materialsystem/imaterial.h"
			"${SRCDIR}/public/materialsystem/imaterialsystem.h"
			"${SRCDIR}/public/materialsystem/imaterialvar.h"
			"${SRCDIR}/public/materialsystem/imesh.h"
			"${SRCDIR}/public/materialsystem/itexture.h"
			"${SRCDIR}/public/pixelwriter.h"
			"${SRCDIR}/public/utllinkedlist.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vector.h"
			"${SRCDIR}/public/vector2d.h"
			"${SRCDIR}/public/vector4d.h"
			"${SRCDIR}/public/VGuiMatSurface/IMatSystemSurface.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"
			"${SRCDIR}/public/vtf/vtf.h"

			"${SRCDIR}/vgui2/src/vgui_key_translation.h"

			"Clip2D.h"
			"Cursor.h"
			"FontTextureCache.h"
			"Input.h"
			"MatSystemSurface.h"
			"TextureDictionary.h"
			"vguimatsurface.h"
			"vguimatsurfacestats.h"
		#}
	)
END_SRC( VGUIMATSURFACE_HEADER_FILES "Header Files" )

add_object(
	TARGET vguimatsurface
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${VGUIMATSURFACE_SOURCE_FILES} ${VGUIMATSURFACE_HEADER_FILES}
)

target_include_directories(
	vguimatsurface PRIVATE
)

target_compile_definitions(
	vguimatsurface PRIVATE

	BENCHMARK
	fopen=dont_use_fopen
	VGUIMATSURFACE_DLL_EXPORT
	GAMEUI_EXPORTS
	DONT_PROTECT_FILEIO_FUNCTIONS
	ENABLE_HTMLWINDOW
	PROTECTED_THINGS_ENABLE
)

target_link_libraries(
	vguimatsurface PRIVATE

	vgui_controls
	vgui_surfacelib
)


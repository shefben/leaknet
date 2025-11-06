cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VGUI_SURFACELIB_SOURCE_FILES )
BEGIN_SRC( VGUI_SURFACELIB_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/utlbuffer.cpp"
			"FontAmalgam.cpp"
			"FontManager.cpp"
			"Win32Font.cpp"
		#}
	)
END_SRC( VGUI_SURFACELIB_SOURCE_FILES "Source Files" )

set( VGUI_SURFACELIB_HEADER_FILES )
BEGIN_SRC( VGUI_SURFACELIB_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/vgui_surfacelib/FontAmalgam.h"
			"${SRCDIR}/common/vgui_surfacelib/FontManager.h"
			"${SRCDIR}/common/vgui_surfacelib/Win32Font.h"
			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vector2d.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"
		#}
	)
END_SRC( VGUI_SURFACELIB_HEADER_FILES "Header Files" )

add_object(
	TARGET vgui_surfacelib
	STATIC
	SOURCES ${VGUI_SURFACELIB_SOURCE_FILES} ${VGUI_SURFACELIB_HEADER_FILES}
)
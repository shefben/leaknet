cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( MATERIALSYSTEM_SOURCE_FILES )
BEGIN_SRC( MATERIALSYSTEM_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/imageloader.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/keyvalues.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/mempool.cpp"
			"${SRCDIR}/public/tgaloader.cpp"
			"${SRCDIR}/public/tgawriter.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"
			"${SRCDIR}/public/vmatrix.cpp"

			"cmaterial.cpp"
			"cmaterialsystem.cpp"
			"cmaterialvar.cpp"
			"colorspace.cpp"
			"ctexture.cpp"
			"imagepacker.cpp"
			"materialsystem_global.cpp"
			"matrendertexture.cpp"
			"shadersystem.cpp"
			"texturemanager.cpp"
			"wireframe.cpp"

			"MaterialSystem.rc"
		#}
	)
END_SRC( MATERIALSYSTEM_SOURCE_FILES "Source Files" )

set( MATERIALSYSTEM_HEADER_FILES )
BEGIN_SRC( MATERIALSYSTEM_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/cstringhash.h"
			"${SRCDIR}/common/materialsystem/IShaderSystem.h"

			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/bumpvects.h"
			"${SRCDIR}/public/const.h"
			"${SRCDIR}/public/convar.h"
			"${SRCDIR}/public/crtmemdebug.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/imageloader.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/keyvalues.h"
			"${SRCDIR}/public/materialsystem/imaterial.h"
			"${SRCDIR}/public/materialsystem/imaterialproxy.h"
			"${SRCDIR}/public/materialsystem/imaterialproxyfactory.h"
			"${SRCDIR}/public/materialsystem/imaterialsystem.h"
			"${SRCDIR}/public/materialsystem/imaterialsystemhardwareconfig.h"
			"${SRCDIR}/public/materialsystem/imaterialvar.h"
			"${SRCDIR}/public/materialsystem/imesh.h"
			"${SRCDIR}/public/materialsystem/IShader.h"
			"${SRCDIR}/public/materialsystem/ishaderapi.h"
			"${SRCDIR}/public/materialsystem/itexture.h"
			"${SRCDIR}/public/materialsystem/materialsystem_config.h"
			"${SRCDIR}/public/mathlib.h"
			"${SRCDIR}/public/mempool.h"
			"${SRCDIR}/public/pixelwriter.h"
			"${SRCDIR}/public/s3_intrf.h"
			"${SRCDIR}/public/tgaloader.h"
			"${SRCDIR}/public/tgawriter.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/utlsymbol.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vector.h"
			"${SRCDIR}/public/vector2d.h"
			"${SRCDIR}/public/vector4d.h"
			"${SRCDIR}/public/vmatrix.h"
			"${SRCDIR}/public/vplane.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"
			"${SRCDIR}/public/vtf/vtf.h"

			"colorspace.h"
			"IHardwareConfigInternal.h"
			"imagepacker.h"
			"imaterialinternal.h"
			"imaterialsysteminternal.h"
			"ishaderutil.h"
			"itextureinternal.h"
			"materialsystem_global.h"
			"matrendertexture.h"
			"resource.h"
			"shaderapi.h"
			"shadersystem.h"
			"texturemanager.h"
		#}
	)
END_SRC( MATERIALSYSTEM_HEADER_FILES "Header Files" )

add_object(
	TARGET materialsystem
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	#INSTALL_OUTNAME "MaterialSystem"
	SOURCES ${MATERIALSYSTEM_SOURCE_FILES} ${MATERIALSYSTEM_HEADER_FILES}
)

target_compile_definitions(
	materialsystem PRIVATE

	"$<$<CONFIG:Debug>:fopen=dont_use_fopen>"

	DEFINE_MATERIALSYSTEM_INTERFACE
	MATERIALSYSTEM_EXPORTS
	strncpy=use_Q_strncpy_instead
	_snprintf=use_Q_snprintf_instead
	PROTECTED_THINGS_ENABLE

	"$<${IS_LINUX}:IMAGE_LOADER_NO_DXTC>" # @TODO(SanyaSho): Linux support
	"$<${IS_LINUX}:_malloca=alloca>"
)

target_link_libraries(
	materialsystem PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"

	"${LIBCOMMON}/s3tc.lib"
	shaderlib
	vtf
)

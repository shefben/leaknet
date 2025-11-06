cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( SHADERLIB_SOURCE_FILES )
BEGIN_SRC( SHADERLIB_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/convar.cpp"
			"${SRCDIR}/public/interface.cpp"

			"BaseShader.cpp"
			"ShaderDLL.cpp"
			"shaderlib_cvar.cpp"
		#}
	)
END_SRC( SHADERLIB_SOURCE_FILES "Source Files" )

set( SHADERLIB_HEADER_FILES )
BEGIN_SRC( SHADERLIB_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/shaderlib/BaseShader.h"
			"${SRCDIR}/public/shaderlib/cshader.h"
			"${SRCDIR}/public/shaderlib/ShaderDLL.h"

			"shaderDLL_Global.h"
			"shaderlib_cvar.h"
		#}
	)
END_SRC( SHADERLIB_HEADER_FILES "Header Files" )

add_object(
	TARGET shaderlib
	STATIC
	SOURCES ${SHADERLIB_SOURCE_FILES} ${SHADERLIB_HEADER_FILES}
)

target_compile_definitions(
	shaderlib PRIVATE

	fopen=dont_use_fopen
)

set( STDSHADER_DX9_DIR ${CMAKE_CURRENT_LIST_DIR} )

include( "${SRCDIR}/cmake/misc/parse_shaders.cmake" )

set( STDSHADER_DX9_SOURCE_FILES )
BEGIN_SRC( STDSHADER_DX9_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/bumpvects.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/vmatrix.cpp"

			"${STDSHADER_DX9_DIR}/BaseVSShader.cpp"
			"${STDSHADER_DX9_DIR}/hsv.cpp"
			"${STDSHADER_DX9_DIR}/lightmappedgeneric.cpp"
			"${STDSHADER_DX9_DIR}/refract.cpp"
			"${STDSHADER_DX9_DIR}/shadow.cpp"
			"${STDSHADER_DX9_DIR}/shatteredglass.cpp"
			"${STDSHADER_DX9_DIR}/sky.cpp"
			"${STDSHADER_DX9_DIR}/stdshader_dx9.cpp"
			"${STDSHADER_DX9_DIR}/teeth.cpp"
			"${STDSHADER_DX9_DIR}/unlitgeneric.cpp"
			"${STDSHADER_DX9_DIR}/vertexlitgeneric.cpp"
			"${STDSHADER_DX9_DIR}/water.cpp"
			"${STDSHADER_DX9_DIR}/worldvertexalpha.cpp"
			"${STDSHADER_DX9_DIR}/worldvertextransition.cpp"
		#}
	)
END_SRC( STDSHADER_DX9_SOURCE_FILES "Source Files" )

set( STDSHADER_DX9_HEADER_FILES )
BEGIN_SRC( STDSHADER_DX9_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${STDSHADER_DX9_DIR}/BaseVSShader.h"
			"${STDSHADER_DX9_DIR}/common_fxc.h"
			"${STDSHADER_DX9_DIR}/common_hlsl_cpp_consts.h"
			"${STDSHADER_DX9_DIR}/common_ps_fxc.h"
			"${STDSHADER_DX9_DIR}/common_vs_fxc.h"
		#}
	)
END_SRC( STDSHADER_DX9_HEADER_FILES "Header Files" )

add_object(
	TARGET stdshader_dx9
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${STDSHADER_DX9_SOURCE_FILES} ${STDSHADER_DX9_HEADER_FILES}
)

parse_shaders( TARGET stdshader_dx9 WORKING_DIRECTORY "${STDSHADER_DX9_DIR}" FILE "stdshader_dx9.txt" )

target_include_directories(
	stdshader_dx9 PRIVATE

	"${STDSHADER_DX9_DIR}/fxctmp9"
)

target_compile_definitions(
	stdshader_dx9 PRIVATE

	fopen=dont_use_fopen

	STDSHADER_DX9_DLL_EXPORT
)

target_link_libraries(
	stdshader_dx9 PRIVATE

	shaderlib
)

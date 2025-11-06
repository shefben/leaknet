set( SHADER_NVFX_DIR ${CMAKE_CURRENT_LIST_DIR} )

include( "${SRCDIR}/cmake/misc/parse_shaders.cmake" )

set( SHADER_NVFX_SOURCE_FILES )
BEGIN_SRC( SHADER_NVFX_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/bumpvects.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/vmatrix.cpp"

			"${SHADER_NVFX_DIR}/BaseVSShader.cpp"
			"${SHADER_NVFX_DIR}/hsv.cpp"
			"${SHADER_NVFX_DIR}/lightmappedgeneric.cpp"
			"${SHADER_NVFX_DIR}/refract.cpp"
			"${SHADER_NVFX_DIR}/shader_nvfx.cpp"
			"${SHADER_NVFX_DIR}/shadow.cpp"
			"${SHADER_NVFX_DIR}/shatteredglass.cpp"
			"${SHADER_NVFX_DIR}/sky.cpp"
			"${SHADER_NVFX_DIR}/teeth.cpp"
			"${SHADER_NVFX_DIR}/unlitgeneric.cpp"
			"${SHADER_NVFX_DIR}/vertexlitgeneric.cpp"
			"${SHADER_NVFX_DIR}/water.cpp"
			"${SHADER_NVFX_DIR}/worldvertexalpha.cpp"
			"${SHADER_NVFX_DIR}/worldvertextransition.cpp"
		#}
	)
END_SRC( SHADER_NVFX_SOURCE_FILES "Source Files" )

set( SHADER_NVFX_HEADER_FILES )
BEGIN_SRC( SHADER_NVFX_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SHADER_NVFX_DIR}/BaseVSShader.h"
			"${SHADER_NVFX_DIR}/common_fxc.h"
			"${SHADER_NVFX_DIR}/common_hlsl_cpp_consts.h"
			"${SHADER_NVFX_DIR}/common_ps_fxc.h"
			"${SHADER_NVFX_DIR}/common_vs_fxc.h"
		#}
	)
END_SRC( SHADER_NVFX_HEADER_FILES "Header Files" )

add_object(
	TARGET shader_nvfx
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${SHADER_NVFX_SOURCE_FILES} ${SHADER_NVFX_HEADER_FILES}
)

parse_shaders( TARGET shader_nvfx WORKING_DIRECTORY "${SHADER_NVFX_DIR}" CUSTOM_OUTDIR "fxctmp9_nv3x" FILE "shader_nvfx_ps20a.txt" )

target_compile_definitions(
	shader_nvfx PRIVATE

	PS20A
	NV3X

	fopen=dont_use_fopen

	shader_nvfx_DLL_EXPORT
)

target_link_libraries(
	shader_nvfx PRIVATE

	shaderlib
)

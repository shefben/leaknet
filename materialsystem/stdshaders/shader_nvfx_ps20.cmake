set( SHADER_NVFX_PS20_DIR ${CMAKE_CURRENT_LIST_DIR} )

include( "${SRCDIR}/cmake/misc/parse_shaders.cmake" )

set( SHADER_NVFX_PS20_SOURCE_FILES )
BEGIN_SRC( SHADER_NVFX_PS20_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/bumpvects.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/vmatrix.cpp"

			"${SHADER_NVFX_PS20_DIR}/BaseVSShader.cpp"
			"${SHADER_NVFX_PS20_DIR}/hsv.cpp"
			"${SHADER_NVFX_PS20_DIR}/lightmappedgeneric.cpp"
			"${SHADER_NVFX_PS20_DIR}/refract.cpp"
			"${SHADER_NVFX_PS20_DIR}/shader_nvfx.cpp"
			"${SHADER_NVFX_PS20_DIR}/shadow.cpp"
			"${SHADER_NVFX_PS20_DIR}/shatteredglass.cpp"
			"${SHADER_NVFX_PS20_DIR}/sky.cpp"
			"${SHADER_NVFX_PS20_DIR}/teeth.cpp"
			"${SHADER_NVFX_PS20_DIR}/unlitgeneric.cpp"
			"${SHADER_NVFX_PS20_DIR}/vertexlitgeneric.cpp"
			"${SHADER_NVFX_PS20_DIR}/water.cpp"
			"${SHADER_NVFX_PS20_DIR}/worldvertexalpha.cpp"
			"${SHADER_NVFX_PS20_DIR}/worldvertextransition.cpp"
		#}
	)
END_SRC( SHADER_NVFX_PS20_SOURCE_FILES "Source Files" )

set( SHADER_NVFX_PS20_HEADER_FILES )
BEGIN_SRC( SHADER_NVFX_PS20_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SHADER_NVFX_PS20_DIR}/BaseVSShader.h"
			"${SHADER_NVFX_PS20_DIR}/common_fxc.h"
			"${SHADER_NVFX_PS20_DIR}/common_hlsl_cpp_consts.h"
			"${SHADER_NVFX_PS20_DIR}/common_ps_fxc.h"
			"${SHADER_NVFX_PS20_DIR}/common_vs_fxc.h"
		#}
	)
END_SRC( SHADER_NVFX_PS20_HEADER_FILES "Header Files" )

add_object(
	TARGET shader_nvfx_ps20
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${SHADER_NVFX_PS20_SOURCE_FILES} ${SHADER_NVFX_PS20_HEADER_FILES}
)

parse_shaders( TARGET shader_nvfx_ps20 WORKING_DIRECTORY "${SHADER_NVFX_PS20_DIR}" CUSTOM_OUTDIR "fxctmp9_nv3x_ps20" FILE "shader_nvfx_ps20.txt" )

target_compile_definitions(
	shader_nvfx_ps20 PRIVATE

	NV3X

	fopen=dont_use_fopen

	SHADER_NVFX_PS20_DLL_EXPORT
)

target_link_libraries(
	shader_nvfx_ps20 PRIVATE

	shaderlib
)

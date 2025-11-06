set( STDSHADER_HDR_DX9_DIR ${CMAKE_CURRENT_LIST_DIR} )

include( "${SRCDIR}/cmake/misc/parse_shaders.cmake" )

set( STDSHADER_HDR_DX9_SOURCE_FILES )
BEGIN_SRC( STDSHADER_HDR_DX9_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/bumpvects.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/vmatrix.cpp"

			"${STDSHADER_HDR_DX9_DIR}/BaseVSShader.cpp"
			"${STDSHADER_HDR_DX9_DIR}/Bloom.cpp"
			"${STDSHADER_HDR_DX9_DIR}/BlurFilterX.cpp"
			"${STDSHADER_HDR_DX9_DIR}/BlurFilterY.cpp"
			"${STDSHADER_HDR_DX9_DIR}/Downsample.cpp"
			"${STDSHADER_HDR_DX9_DIR}/hsv.cpp"
			"${STDSHADER_HDR_DX9_DIR}/lightmappedgeneric_HDR.cpp"
			"${STDSHADER_HDR_DX9_DIR}/refract.cpp"
			"${STDSHADER_HDR_DX9_DIR}/shadow.cpp"
			"${STDSHADER_HDR_DX9_DIR}/shatteredglass.cpp"
			"${STDSHADER_HDR_DX9_DIR}/ShowDestAlpha.cpp"
			"${STDSHADER_HDR_DX9_DIR}/sky.cpp"
			"${STDSHADER_HDR_DX9_DIR}/stdshader_hdr_dx9.cpp"
			"${STDSHADER_HDR_DX9_DIR}/teeth.cpp"
			"${STDSHADER_HDR_DX9_DIR}/unlitgeneric_hdr.cpp"
			"${STDSHADER_HDR_DX9_DIR}/vertexlitgeneric_hdr.cpp"
			"${STDSHADER_HDR_DX9_DIR}/water.cpp"
			"${STDSHADER_HDR_DX9_DIR}/worldvertexalpha.cpp"
		#}
	)
END_SRC( STDSHADER_HDR_DX9_SOURCE_FILES "Source Files" )

set( STDSHADER_HDR_DX9_HEADER_FILES )
BEGIN_SRC( STDSHADER_HDR_DX9_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${STDSHADER_HDR_DX9_DIR}/BaseVSShader.h"
			"${STDSHADER_HDR_DX9_DIR}/common_fxc.h"
			"${STDSHADER_HDR_DX9_DIR}/common_hlsl_cpp_consts.h"
			"${STDSHADER_HDR_DX9_DIR}/common_ps_fxc.h"
			"${STDSHADER_HDR_DX9_DIR}/common_vs_fxc.h"
		#}
	)
END_SRC( STDSHADER_HDR_DX9_HEADER_FILES "Header Files" )

add_object(
	TARGET stdshader_hdr_dx9
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${STDSHADER_HDR_DX9_SOURCE_FILES} ${STDSHADER_HDR_DX9_HEADER_FILES}
)

# @FIXME(SanyaSho): error C1128: number of sections exceeded object file format limit: compile with /bigobj
if( ${IS_WINDOWS} )
	set_source_files_properties(
		"${STDSHADER_HDR_DX9_DIR}/stdshader_hdr_dx9.cpp"
		PROPERTIES COMPILE_FLAGS /bigobj
	)
endif()

parse_shaders( TARGET stdshader_hdr_dx9 WORKING_DIRECTORY "${STDSHADER_HDR_DX9_DIR}" FILE "stdshader_hdr_dx9.txt" )

target_include_directories(
	stdshader_hdr_dx9 PRIVATE

	"${STDSHADER_HDR_DX9_DIR}/fxctmp9"
)

target_compile_definitions(
	stdshader_hdr_dx9 PRIVATE

	HDR

	fopen=dont_use_fopen

	stdshader_hdr_dx9_DLL_EXPORT
)

target_link_libraries(
	stdshader_hdr_dx9 PRIVATE

	shaderlib
)

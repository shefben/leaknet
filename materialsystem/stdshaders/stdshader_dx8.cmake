set( STDSHADER_DX8_DIR ${CMAKE_CURRENT_LIST_DIR} )

include( "${SRCDIR}/cmake/misc/parse_shaders.cmake" )

set( STDSHADER_DX8_SOURCE_FILES )
BEGIN_SRC( STDSHADER_DX8_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/bumpvects.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/vmatrix.cpp"

			"${STDSHADER_DX8_DIR}/alienscale_dx8.cpp"
			"${STDSHADER_DX8_DIR}/basetimeslightmapwet.cpp"
			"${STDSHADER_DX8_DIR}/BaseVSShader.cpp"
			"${STDSHADER_DX8_DIR}/cable.cpp"
			"${STDSHADER_DX8_DIR}/eyes.cpp"
			"${STDSHADER_DX8_DIR}/gooinglass.cpp"
			"${STDSHADER_DX8_DIR}/jellyfish.cpp"
			"${STDSHADER_DX8_DIR}/jojirium.cpp"
			"${STDSHADER_DX8_DIR}/lightmappedgeneric_dx8.cpp"
			"${STDSHADER_DX8_DIR}/modulate.cpp"
			"${STDSHADER_DX8_DIR}/overlay_fit.cpp"
			"${STDSHADER_DX8_DIR}/particlesphere.cpp"
			"${STDSHADER_DX8_DIR}/predator.cpp"
			"${STDSHADER_DX8_DIR}/refract_dx80.cpp"
			"${STDSHADER_DX8_DIR}/shadowbuild.cpp"
			"${STDSHADER_DX8_DIR}/shadowmodel.cpp"
			"${STDSHADER_DX8_DIR}/shadow_dx8.cpp"
			"${STDSHADER_DX8_DIR}/sprite.cpp"
			"${STDSHADER_DX8_DIR}/stdshader_dx8.cpp"
			"${STDSHADER_DX8_DIR}/teeth_dx8.cpp"
			"${STDSHADER_DX8_DIR}/unlitgeneric_dx8.cpp"
			"${STDSHADER_DX8_DIR}/unlittwotexture.cpp"
			"${STDSHADER_DX8_DIR}/vertexlitgeneric_dx8.cpp"
			"${STDSHADER_DX8_DIR}/water_dx80.cpp"
			"${STDSHADER_DX8_DIR}/water_dx81.cpp"
			"${STDSHADER_DX8_DIR}/worldvertexalpha_dx8.cpp"
			"${STDSHADER_DX8_DIR}/WorldVertexTransition_dx8.cpp"
			"${STDSHADER_DX8_DIR}/writez.cpp"
			"${STDSHADER_DX8_DIR}/yuv.cpp"
		#}
	)
END_SRC( STDSHADER_DX8_SOURCE_FILES "Source Files" )

set( STDSHADER_DX8_HEADER_FILES )
BEGIN_SRC( STDSHADER_DX8_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${STDSHADER_DX8_DIR}/BaseVSShader.h"
		#}
	)
END_SRC( STDSHADER_DX8_HEADER_FILES "Header Files" )

add_object(
	TARGET stdshader_dx8
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${STDSHADER_DX8_SOURCE_FILES} ${STDSHADER_DX8_HEADER_FILES}
)

parse_shaders( TARGET stdshader_dx8 WORKING_DIRECTORY "${STDSHADER_DX8_DIR}" FILE "stdshader_dx8.txt" )

target_compile_definitions(
	stdshader_dx8 PRIVATE

	fopen=dont_use_fopen

	STDSHADER_DX8_DLL_EXPORT
)

target_link_libraries(
	stdshader_dx8 PRIVATE

	shaderlib
)

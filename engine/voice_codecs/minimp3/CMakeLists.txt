cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VAUDIO_MINIMP3_SOURCE_FILES )
BEGIN_SRC( VAUDIO_MINIMP3_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#$Folder "Source Files"
		#{
			"${SRCDIR}/public/interface.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"

			"${SRCDIR}/engine/voice_codecs/frame_encoder/voice_codec_frame.cpp"

			"vaudio_minimp3.cpp"
		#}
	)
END_SRC( VAUDIO_MINIMP3_SOURCE_FILES "Source Files" )

set( VAUDIO_MINIMP3_HEADER_FILES )
BEGIN_SRC( VAUDIO_MINIMP3_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#$Folder "Header Files"
		#{
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/vaudio/ivaudio.h"

			"minimp3.h"
		#}
	)
END_SRC( VAUDIO_MINIMP3_HEADER_FILES "Header Files" )

add_object(
	TARGET vaudio_minimp3
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${VAUDIO_MINIMP3_SOURCE_FILES} ${VAUDIO_MINIMP3_HEADER_FILES}
)

target_include_directories(
	vaudio_minimp3 PRIVATE

	"${SRCDIR}/engine"
)

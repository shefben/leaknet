cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VAUDIO_MILES_SOURCE_FILES )
BEGIN_SRC( VAUDIO_MILES_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#$Folder "Source Files"
		#{
			"${SRCDIR}/public/interface.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"

			"${SRCDIR}/engine/voice_codecs/frame_encoder/voice_codec_frame.cpp"

			"milesbase.cpp"
			"mp3_miles.cpp"
			"voice_codec_miles.cpp"
		#}
	)
END_SRC( VAUDIO_MILES_SOURCE_FILES "Source Files" )

set( VAUDIO_MILES_HEADER_FILES )
BEGIN_SRC( VAUDIO_MILES_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#$Folder "Header Files"
		#{
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/vaudio/ivaudio.h"

			"milesbase.h"
		#}
	)
END_SRC( VAUDIO_MILES_HEADER_FILES "Header Files" )

add_object(
	TARGET vaudio_miles
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${VAUDIO_MILES_SOURCE_FILES} ${VAUDIO_MILES_HEADER_FILES}
)

target_include_directories(
	vaudio_miles PRIVATE

	"${SRCDIR}/engine"
	"${SRCDIR}/engine/voice_codecs/frame_encoder"
)

target_compile_definitions(
	vaudio_miles PRIVATE

	MILES_EXPORTS
	WINDOWS_IGNORE_PACKING_MISMATCH
)

target_link_libraries(
	vaudio_miles PRIVATE

	"$<${IS_WINDOWS}:${LIBCOMMON}/mss32.lib>"
)

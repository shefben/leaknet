include_guard( GLOBAL )

set( PHONEMEEXTRACTOR_IMS_SOURCE_FILES )
BEGIN_SRC( PHONEMEEXTRACTOR_IMS_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/filesystem_tools.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/sentence.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"

			"${SRCDIR}/utils/common/cmdlib.cpp"

			"${SRCDIR}/utils/hlfaceposer/phonemeconverter.cpp"

			"extractor_utils.cpp"
			#"phonemeextractor.cpp" # !IMS
			"phonemeextractor_ims.cpp" # IMS
		#}
	)
END_SRC( PHONEMEEXTRACTOR_IMS_SOURCE_FILES "Source Files" )

set( PHONEMEEXTRACTOR_IMS_HEADER_FILES )
BEGIN_SRC( PHONEMEEXTRACTOR_IMS_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/commonmacros.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/filesystem_tools.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/mathlib.h"
			"${SRCDIR}/public/protected_things.h"
			"${SRCDIR}/public/sentence.h"
			"${SRCDIR}/public/string_t.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utllinkedlist.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vector.h"
			"${SRCDIR}/public/vector2d.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"

			"${SRCDIR}/utils/common/cmdlib.h"

			"${SRCDIR}/utils/hlfaceposer/PhonemeConverter.h"

			"${SRCDIR}/utils/sapi51/include/sapi.h"
			"${SRCDIR}/utils/sapi51/include/sapiddk.h"
			"${SRCDIR}/utils/sapi51/include/spddkhlp.h"
			"${SRCDIR}/utils/sapi51/include/spdebug.h"
			"${SRCDIR}/utils/sapi51/include/sperror.h"
			"${SRCDIR}/utils/sapi51/include/sphelper.h"

			"phonemeextractor.h"
			"talkback.h"
		#}
	)
END_SRC( PHONEMEEXTRACTOR_IMS_HEADER_FILES "Header Files" )

add_object(
	TARGET phonemeextractor_ims
	MODULE
	PROPERTY_FOLDER "Utils//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/bin/phonemeextractors"
	SOURCES ${PHONEMEEXTRACTOR_IMS_SOURCE_FILES} ${PHONEMEEXTRACTOR_IMS_HEADER_FILES}
)

target_include_directories(
	phonemeextractor_ims PRIVATE

	"${SRCDIR}/utils/common"
	"${SRCDIR}/utils/hlfaceposer"
	"${SRCDIR}/utils/sapi51/include"
)

target_compile_definitions(
	phonemeextractor_ims PRIVATE

	PHONEMEEXTRACTOR_EXPORTS
)

target_link_libraries(
	phonemeextractor_ims PRIVATE

	"${SRCDIR}/utils/sapi51/lib/i386/sapi.lib"
)


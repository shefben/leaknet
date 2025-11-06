cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( CLASSCHECK_SOURCE_FILES )
BEGIN_SRC( CLASSCHECK_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/utlsymbol.cpp" # Noprecomp?
			"class.cpp"
			"classcheck.cpp"
			"classcheck_util.cpp"
			"processmodule.cpp"
			"StdAfx.cpp" # Create precomp: StdAfx.h
		#}
	)
END_SRC( CLASSCHECK_SOURCE_FILES "Source Files" )

set( CLASSCHECK_HEADER_FILES )
BEGIN_SRC( CLASSCHECK_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/utldict.h"
			"class.h"
			"classcheck_util.h"
			"codeprocessor.h"
			"icodeprocessor.h"
			"StdAfx.h"
		#}
	)
END_SRC( CLASSCHECK_HEADER_FILES "Header Files" )

add_object(
	TARGET classcheck
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${CLASSCHECK_SOURCE_FILES} ${CLASSCHECK_HEADER_FILES}
)

target_precompile_headers(
	classcheck
	PRIVATE "StdAfx.h"
)

target_include_directories(
	classcheck PRIVATE

	"${SRCDIR}/utils/common"
)

target_link_libraries(
	classcheck PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
)
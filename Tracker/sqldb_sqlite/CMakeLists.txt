cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( SQLDB_SQLITE_SOURCE_FILES )
BEGIN_SRC( SQLDB_SQLITE_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/MemPool.cpp"
			"${SRCDIR}/public/interface.cpp"

			"${SRCDIR}/utils/sqlite/shell.c"
			"${SRCDIR}/utils/sqlite/sqlite3.c"

			"${SRCDIR}/Tracker/common/CompletionEvent.cpp"

			"SQLiteDatabase.cpp"
			"SQLiteDistroDatabase.cpp"
			"SQLiteUserDatabase.cpp"
		#}
	)
END_SRC( SQLDB_SQLITE_SOURCE_FILES "Source Files" )

set( SQLDB_SQLITE_HEADER_FILES )
BEGIN_SRC( SQLDB_SQLITE_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.h"

			"SQLiteDatabase.h"
		#}
	)
END_SRC( SQLDB_SQLITE_HEADER_FILES "Header Files" )

add_object(
	TARGET sqldb_sqlite
	MODULE
	PROPERTY_FOLDER "Tracker//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/TrackerServer"
	SOURCES ${SQLDB_SQLITE_SOURCE_FILES} ${SQLDB_SQLITE_HEADER_FILES}
)

target_include_directories(
	sqldb_sqlite PRIVATE

	"${SRCDIR}/Tracker/common"
	"${SRCDIR}/Tracker/Public"

	"${SRCDIR}/utils/sqlite"
)

target_compile_definitions(
	sqldb_sqlite PRIVATE

	sqldb_sqlite_EXPORTS
)

target_link_libraries(
	sqldb_sqlite PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"

	"$<${IS_WINDOWS}:${SRCDIR}/lib/common/sqlite3.lib>"
)

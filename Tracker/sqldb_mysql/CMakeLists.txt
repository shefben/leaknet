cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( SQLDB_MYSQL_SOURCE_FILES )
BEGIN_SRC( SQLDB_MYSQL_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/MemPool.cpp"
			"${SRCDIR}/public/interface.cpp"

			"${SRCDIR}/Tracker/common/CompletionEvent.cpp"

			"MySqlDatabase.cpp"
			"MySqlDistroDatabase.cpp"
			"MySqlUserDatabase.cpp"
		#}
	)
END_SRC( SQLDB_MYSQL_SOURCE_FILES "Source Files" )

set( SQLDB_MYSQL_HEADER_FILES )
BEGIN_SRC( SQLDB_MYSQL_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.h"

			"MySqlDatabase.h"
		#}
	)
END_SRC( SQLDB_MYSQL_HEADER_FILES "Header Files" )

add_object(
	TARGET sqldb_mysql
	MODULE
	PROPERTY_FOLDER "Tracker//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/TrackerServer"
	SOURCES ${SQLDB_MYSQL_SOURCE_FILES} ${SQLDB_MYSQL_HEADER_FILES}
)

target_include_directories(
	sqldb_mysql PRIVATE

	"${SRCDIR}/Tracker/common"
	"${SRCDIR}/Tracker/Public"

	"${SRCDIR}/utils/mysqlpp/mysql/include"
	"${SRCDIR}/utils/mysqlpp/lib"
)

target_compile_definitions(
	sqldb_mysql PRIVATE

	SQLDB_MYSQL_EXPORTS
)

target_link_libraries(
	sqldb_mysql PRIVATE

	"$<${IS_WINDOWS}:wsock32>"
	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"

	#"$<${IS_WINDOWS}:${SRCDIR}/lib/common/libmysql.lib>"
	#"$<${IS_WINDOWS}:${SRCDIR}/lib/common/mysqlpp.lib>"

	"$<${IS_WINDOWS}:${SRCDIR}/utils/mysqlpp/mysql/lib/libmysql.lib>"
	mysqlpp
)

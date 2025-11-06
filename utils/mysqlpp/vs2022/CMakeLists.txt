cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( MYSQLPP_SOURCE_FILES )
BEGIN_SRC( MYSQLPP_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/utils/mysqlpp/lib/beemutex.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/cmdline.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/connection.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/cpool.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/datetime.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/dbdriver.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/field_names.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/field_types.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/manip.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/myset.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/mysql++.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/mystring.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/null.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/options.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/qparms.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/query.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/result.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/row.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/scopedconnection.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/sqlstream.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/sql_buffer.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/ssqls2.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/stadapter.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/tcp_connection.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/transaction.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/type_info.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/uds_connection.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/utility.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/vallist.cpp"
			"${SRCDIR}/utils/mysqlpp/lib/wnp_connection.cpp"
		#}
	)
END_SRC( MYSQLPP_SOURCE_FILES "Source Files" )

add_object(
	TARGET mysqlpp
	MODULE
	PROPERTY_FOLDER "Utils//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/bin"
	EXCLUDE_LIB tier0 vstdlib
	SOURCES ${MYSQLPP_SOURCE_FILES}
)

target_include_directories(
	mysqlpp PRIVATE

	"${SRCDIR}/utils/mysqlpp/lib"
	"${SRCDIR}/utils/mysqlpp/mysql/include"
)

target_compile_definitions(
	mysqlpp PRIVATE

	UNICODE
	_UNICODE
	MYSQLPP_MAKING_DLL
	"$<$<CONFIG:Release>:HAVE_MYSQL_SSL_SET>"
	"$<$<CONFIG:Debug>:nnnHAVE_MYSQL_SSL_SETnnn>"
)

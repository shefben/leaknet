cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( HK_BASE_SOURCE_FILES )
BEGIN_SRC( HK_BASE_SOURCE_FILES "" )
	SRC_GRP(
		SUBGROUP "memory"
		SOURCES
		#{
			"memory/memory.cpp"
			"memory/memory_util.cpp"
			"memory/memory.h"
			"memory/memory_managed_virtual_class.h"
			"memory/memory_util.h"
			"memory/memory.inl"
		#}
	)

	SRC_GRP(
		SUBGROUP "array"
		SOURCES
		#{
			"array/array.cpp"
			"array/array.h"
			"array/pre_allocated_array.h"
			"array/array.inl"
		#}
	)

	SRC_GRP(
		SUBGROUP "hash"
		SOURCES
		#{
			"hash/hash.cpp"
			"hash/hash.h"
			"hash/hash.inl"
		#}
	)

	SRC_GRP(
		SUBGROUP "stopwatch"
		SOURCES
		#{
			"stopwatch/stopwatch.cpp"
			"stopwatch/stopwatch.h"
			"stopwatch/stopwatch_qpt.inl"
		#}
	)

	SRC_GRP(
		SUBGROUP "string"
		SOURCES
		#{
			"string/string.cpp"
			"string/string.h"
		#}
	)

	SRC_GRP(
		SUBGROUP "id_server"
		SOURCES
		#{
			"id_server/id_server.cpp"
			"id_server/id_server.h"
			"id_server/id_server.inl"
		#}
	)
END_SRC( HK_BASE_SOURCE_FILES "" )

BEGIN_SRC( HK_BASE_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"base_types.cpp"
			"console.cpp"
		#}
	)
END_SRC( HK_BASE_SOURCE_FILES "Source Files" )

set( HK_BASE_HEADER_FILES )
BEGIN_SRC( HK_BASE_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"base.h"
			"base_types.h"
			"console.h"
		#}
	)
END_SRC( HK_BASE_HEADER_FILES "Header Files" )

add_object(
	TARGET hk_base
	STATIC
	PROPERTY_FOLDER "VPhysics//Static Libs"
	SOURCES ${HK_BASE_SOURCE_FILES} ${HK_BASE_HEADER_FILES}
)

target_precompile_headers(
	hk_base
	PRIVATE "base.h"
)

target_include_directories(
	hk_base PRIVATE

	"${SRCDIR}/ivp/havana/havok"
)

cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( HK_MATH_SOURCE_FILES )
BEGIN_SRC( HK_MATH_SOURCE_FILES "" )
	SRC_GRP(
		SUBGROUP "lcp"
		SOURCES
		#{
			"lcp/lcp_solver.cpp"
			"lcp/lcp_solver.h"
		#}
	)

	SRC_GRP(
		SUBGROUP "incr_lu"
		SOURCES
		#{
			"incr_lu/incr_lu.cpp"
			"incr_lu/incr_lu.h"
		#}
	)

	SRC_GRP(
		SUBGROUP "gauss_elimination"
		SOURCES
		#{
			"gauss_elimination/gauss_elimination.cpp"
			"gauss_elimination/gauss_elimination.h"
		#}
	)

	SRC_GRP(
		SUBGROUP "quaternion"
		SOURCES
		#{
			"quaternion/quaternion.cpp"
			"quaternion/quaternion_util.cpp"
			"quaternion/quaternion.h"
			"quaternion/quaternion_util.h"
			"quaternion/quaternion.inl"
		#}
	)

	SRC_GRP(
		SUBGROUP "vector3"
		SOURCES
		#{
			"vector3/vector3.cpp"
			"vector3/vector3_util.cpp"
			"vector3/vector3.h"
			"vector3/vector3_util.h"
			"vector3/vector3.inl"
			"vector3/vector3_util.inl"
		#}
	)

	SRC_GRP(
		SUBGROUP "matrix"
		SOURCES
		#{
			"matrix/matrix_inverter.h"
		#}
	)

	SRC_GRP(
		SUBGROUP "vector_fpu"
		SOURCES
		#{
			"vector_fpu/vector_fpu.h"
			"vector_fpu/vector_fpu.inl"
		#}
	)
END_SRC( HK_MATH_SOURCE_FILES "" )

BEGIN_SRC( HK_MATH_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"densematrix.cpp"
			"densematrix_util.cpp"
			"eulerangles.cpp"
			"math.cpp"
			"matrix3.cpp"
			"odesolve.cpp"
			"plane.cpp"
			"rotation.cpp"
			"spatial_matrix.cpp"
			"spatial_vector.cpp"
			"transform.cpp"
		#}
	)
END_SRC( HK_MATH_SOURCE_FILES "Source Files" )

set( HK_MATH_HEADER_FILES )
BEGIN_SRC( HK_MATH_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"dense_vector.h"
			"densematrix.h"
			"densematrix_util.h"
			"eulerangles.h"
			"interval.h"
			"math.h"
			"matrix3.h"
			"odesolve.h"
			"plane.h"
			"qtransform.h"
			"ray.h"
			"rotation.h"
			"spatial_matrix.h"
			"spatial_vector.h"
			"transform.h"
			"types.h"
			"vecmath.h"
			"vector4.h"

			"dense_vector.inl"
			"densematrix.inl"
			"densematrix_util.inl"
			"math.inl"
			"matrix3.inl"
			"plane.inl"
			"qtransform.inl"
			"ray.inl"
			"rotation.inl"
			"spatial_matrix.inl"
			"spatial_vector.inl"
			"transform.inl"
			"vector4.inl"
		#}
	)
END_SRC( HK_MATH_HEADER_FILES "Header Files" )

add_object(
	TARGET hk_math
	STATIC
	PROPERTY_FOLDER "VPhysics//Static Libs"
	SOURCES ${HK_MATH_SOURCE_FILES} ${HK_MATH_HEADER_FILES}
)

target_precompile_headers(
	hk_math
	PRIVATE "vecmath.h"
)

target_include_directories(
	hk_math PRIVATE

	"${SRCDIR}/ivp/havana/havok"
)

target_compile_definitions(
	hk_math PRIVATE

	"$<$<CONFIG:Debug>:HK_DEBUG>"
)

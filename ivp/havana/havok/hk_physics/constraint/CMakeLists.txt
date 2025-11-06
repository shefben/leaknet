cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( HAVANA_CONSTRAINTS_SOURCE_FILES )
BEGIN_SRC( HAVANA_CONSTRAINTS_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"ball_socket/ball_socket_constraint.cpp"
			"breakable_constraint/breakable_constraint.cpp"
			"constraint.cpp"
			"fixed/fixed_constraint.cpp"
			"hinge/hinge_bp_builder.cpp"
			"hinge/hinge_constraint.cpp"
			"limited_ball_socket/limited_ball_socket_constraint.cpp"
			"prismatic/prismatic_constraint.cpp"
			"pulley/pulley_constraint.cpp"
			"ragdoll/ragdoll_constraint.cpp"
			"ragdoll/ragdoll_constraint_bp_builder.cpp"
			"stiff_spring/stiff_spring_constraint.cpp"
			"${SRCDIR}/ivp/ivp_physics/hk_physics/constraint/local_constraint_system/local_constraint_system.cpp"
			"${SRCDIR}/ivp/ivp_physics/hk_physics/core/rigid_body_core.cpp"
			"${SRCDIR}/ivp/ivp_physics/hk_physics/effector/rigid_body_binary_effector.cpp"
		#}
	)
END_SRC( HAVANA_CONSTRAINTS_SOURCE_FILES "Source Files" )

set( HAVANA_CONSTRAINTS_HEADER_FILES )
BEGIN_SRC( HAVANA_CONSTRAINTS_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"ball_socket/ball_socket_bp.h"
			"ball_socket/ball_socket_constraint.h"
			"breakable_constraint/breakable_constraint.h"
			"breakable_constraint/breakable_constraint_bp.h"
			"constraint.h"
			"constraint_bp.h"
			"constraint_limit.h"
			"fixed/fixed_bp.h"
			"fixed/fixed_constraint.h"
			"hinge/hinge_bp.h"
			"hinge/hinge_bp_builder.h"
			"hinge/hinge_constraint.h"
			"limited_ball_socket/limited_ball_socket_bp.h"
			"limited_ball_socket/limited_ball_socket_constraint.h"
			"local_constraint_system/local_constraint_system.h"
			"local_constraint_system/local_constraint_system_bp.h"
			"prismatic/prismatic_bp.h"
			"prismatic/prismatic_constraint.h"
			"pulley/pulley_bp.h"
			"pulley/pulley_constraint.h"
			"ragdoll/ragdoll_constraint.h"
			"ragdoll/ragdoll_constraint_bp.h"
			"ragdoll/ragdoll_constraint_bp_builder.h"
			"stiff_spring/stiff_spring_bp.h"
			"stiff_spring/stiff_spring_constraint.h"
			"util/constraint_limit_util.h"
			"../core/vm_query_builder/mass_relative_vector3.h"
			"../core/vm_query_builder/vm_query_builder.h"
			"${SRCDIR}/ivp/ivp_physics/hk_physics/constraint/local_constraint_system/local_constraint_system.h"
			"${SRCDIR}/ivp/ivp_physics/hk_physics/physics.h"
			"${SRCDIR}/ivp/ivp_physics/hk_physics/simunit/psi_info.h"

			#"local_constraint_system/local_constraint_system.inl"
		#}
	)
END_SRC( HAVANA_CONSTRAINTS_HEADER_FILES "Header Files" )

add_object(
	TARGET havana_constraints
	STATIC
	PROPERTY_FOLDER "VPhysics//Static Libs"
	SOURCES ${HAVANA_CONSTRAINTS_SOURCE_FILES} ${HAVANA_CONSTRAINTS_HEADER_FILES}
)

target_include_directories(
	havana_constraints PRIVATE

	"${SRCDIR}/ivp/ivp_collision"
	"${SRCDIR}/ivp/ivp_controller"
	"${SRCDIR}/ivp/ivp_intern"
	"${SRCDIR}/ivp/ivp_physics"
	"${SRCDIR}/ivp/ivp_surface_manager"
	"${SRCDIR}/ivp/ivp_utility"
	"${SRCDIR}/ivp/havana/havok"
)

target_compile_definitions(
	havana_constraints PRIVATE

	IVP_VERSION_SDK
	HAVANA_CONSTRAINTS
)

cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VPHYSICS_SOURCE_FILES )
BEGIN_SRC( VPHYSICS_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"convert.cpp"
			"main.cpp"
			"physics_airboat.cpp"
			"physics_collide.cpp"
			"physics_constraint.cpp"
			"physics_controller_raycast_vehicle.cpp"
			"physics_environment.cpp"
			"physics_fluid.cpp"
			"physics_material.cpp"
			"physics_motioncontroller.cpp"
			"physics_object.cpp"
			"physics_shadow.cpp"
			"physics_spring.cpp"
			"physics_vehicle.cpp"
			"trace.cpp"
			"vcollide_parse.cpp"
			"vphysics_saverestore.cpp"
		#}
	)
END_SRC( VPHYSICS_SOURCE_FILES "Source Files" )

set( VPHYSICS_HEADER_FILES )
BEGIN_SRC( VPHYSICS_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/datamap.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/vcollide.h"
			"${SRCDIR}/public/vcollide_parse.h"
			"${SRCDIR}/public/vphysics/constraints.h"
			"${SRCDIR}/public/vphysics/vehicles.h"
			"${SRCDIR}/public/vphysics_interface.h"

			"convert.h"
			"linear_solver.h"
			"physics_airboat.h"
			"physics_constraint.h"
			"physics_controller_raycast_vehicle.h"
			"physics_environment.h"
			"physics_fluid.h"
			"physics_material.h"
			"physics_motioncontroller.h"
			"physics_object.h"
			"physics_shadow.h"
			"physics_spring.h"
			"physics_trace.h"
			"physics_vehicle.h"
			"vcollide_parse_private.h"
			"vphysics_internal.h"
			"vphysics_saverestore.h"
		#}
	)
END_SRC( VPHYSICS_HEADER_FILES "Header Files" )

add_object(
	TARGET vphysics
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	PROPERTY_FOLDER "VPhysics//Shared Libs"
	SOURCES ${VPHYSICS_SOURCE_FILES} ${VPHYSICS_HEADER_FILES}
)

target_include_directories(
	vphysics PRIVATE

	"${SRCDIR}/ivp/ivp_intern"
	"${SRCDIR}/ivp/ivp_collision"
	"${SRCDIR}/ivp/ivp_physics"
	"${SRCDIR}/ivp/ivp_surface_manager"
	"${SRCDIR}/ivp/ivp_utility"
	"${SRCDIR}/ivp/ivp_controller"
	"${SRCDIR}/ivp/ivp_compact_builder"
	"${SRCDIR}/ivp/havana/havok"
	"${SRCDIR}/ivp/havana"
)

target_compile_definitions(
	vphysics PRIVATE

	VPHYSICS_EXPORTS
	"$<${IS_WINDOWS}:ENABLE_IVP_MOPP=1>"
	strncpy=use_Q_strncpy_instead
	_snprintf=use_Q_snprintf_instead
	HAVANA_CONSTRAINTS
	HAVOK_MOPP
)

target_link_libraries(
	vphysics PRIVATE

	havana_constraints
	"$<${IS_WINDOWS}:${LIBCOMMON}/havok_mopp.lib>"
	hk_math
	hk_base
	ivp_compact_builder
	ivp_physics
)


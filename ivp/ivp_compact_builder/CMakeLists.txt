cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( IVP_COMPACT_BUILDER_SOURCE_FILES )
BEGIN_SRC( IVP_COMPACT_BUILDER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/ivp/ivp_compact_builder/3dsimport_co.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack_cutfac.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack_cvdec3.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack_drdec3.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack_dsphdc.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack_edght.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack_initcb.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack_insed3.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack_insfac.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack_insvr3.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack_prime.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack_ptpolg.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack_resedg.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_compact_ledge_gen.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_compact_modify.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_compact_recursive.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_convex_decompositor.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_halfspacesoup.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_i_fpoint_vhash.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_i_point_vhash.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_object_polygon_tetra.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_rot_inertia_solver.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_halfspacesoup.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_ledge_soup.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_pointsoup.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_polygon_convex.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_polyhdrn_cncv.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_q12.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_templates_intern.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_tetra_intrude.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivv_cluster_min_hash.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_geom.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_geom2.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_global.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_io.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_mem.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_merge.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_poly.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_poly2.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_qset.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_stat.cxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_user.cxx"
		#}
	)
END_SRC( IVP_COMPACT_BUILDER_SOURCE_FILES "Source Files" )

set( IVP_COMPACT_BUILDER_HEADER_FILES )
BEGIN_SRC( IVP_COMPACT_BUILDER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/ivp/ivp_compact_builder/3dsimport_load.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/geompack.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_compact_ledge_gen.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_compact_modify.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_compact_recursive.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_convex_decompositor.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_halfspacesoup.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_i_fpoint_vhash.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_i_point_vhash.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_object_polygon_tetra.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_rot_inertia_solver.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_3ds.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_halfspacesoup.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_ledge_soup.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_pointsoup.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_polygon_convex.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_polyhdrn_cncv.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_surbuild_q12.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_templates_intern.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_tetra_intrude.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivp_triangle_gen.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/ivv_cluster_min_hash.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_a.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_geom.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_io.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_mem.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_merge.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_poly.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_qset.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_stat.hxx"
			"${SRCDIR}/ivp/ivp_compact_builder/qhull_user.hxx"
		#}
	)
END_SRC( IVP_COMPACT_BUILDER_HEADER_FILES "Header Files" )

add_object(
	TARGET ivp_compact_builder
	STATIC
	PROPERTY_FOLDER "VPhysics//Static Libs"
	SOURCES ${IVP_COMPACT_BUILDER_SOURCE_FILES} ${IVP_COMPACT_BUILDER_HEADER_FILES}
)

#target_precompile_headers(
#	ivp_compact_builder
#	PRIVATE "ivp_physics.hxx"
#)

target_include_directories(
	ivp_compact_builder PRIVATE

	"${SRCDIR}/ivp/ivp_surface_manager"
	"${SRCDIR}/ivp/ivp_physics"
	"${SRCDIR}/ivp/ivp_compact_builder"
	"${SRCDIR}/ivp/ivp_utility"
	"${SRCDIR}/ivp/ivp_collision"
)

target_compile_definitions(
	ivp_compact_builder PRIVATE

	IVP_VERSION_SDK
	HAVOK_MOPP
)

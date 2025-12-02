//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "sysexternal.h"
#include "cmodel_engine.h"
#include "modelloader.h"		// For GetCurrentBSPVersion()

// UNDONE: Abstract the texture/material lookup stuff and all of this goes away
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
extern IMaterialSystem *materialSystemInterface;

#include "vphysics_interface.h"
#include "sys_dll.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IPhysicsSurfaceProps *physprop = NULL;
IPhysicsCollision	 *physcollision = NULL;

// local forward declarations
void CollisionBSPData_LoadTextures( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadTexinfo( CCollisionBSPData *pBSPData, CUtlVector<unsigned short> &map_texinfo );
void CollisionBSPData_LoadLeafs( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadLeafs( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadLeafBrushes( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadPlanes( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadBrushes( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadBrushSides( CCollisionBSPData *pBSPData, CUtlVector<unsigned short> &map_texinfo );
void CollisionBSPData_LoadSubmodels( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadNodes( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadAreas( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadAreaPortals( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadVisibility( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadEntityString( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadPhysics( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadDispInfo( CCollisionBSPData *pBSPData );
void CollisionBSPData_LoadMapFlags( CCollisionBSPData *pBSPData );


//=============================================================================
//
// Initialization/Destruction Functions
//


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CollisionBSPData_Init( CCollisionBSPData *pBSPData )
{
	pBSPData->numleafs = 1;
	pBSPData->map_vis = NULL;
	pBSPData->numareas = 1;
	pBSPData->numclusters = 1;
	pBSPData->map_nullname = "**empty**";
	pBSPData->numtextures = 0;

	// Initialize map flags
	pBSPData->map_flags_loaded = false;
	memset( &pBSPData->map_flags, 0, sizeof( dflagslump_t ) );

	return true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_Destroy( CCollisionBSPData *pBSPData )
{
	for ( int i = 0; i < pBSPData->numcmodels; i++ )
	{
		physcollision->VCollideUnload( &pBSPData->map_cmodels[i].vcollisionData );
	}

	// free displacement data
	DispCollTrees_FreeLeafList( pBSPData );
	DispCollTrees_Free( g_pDispCollTrees );
	g_pDispCollTrees = NULL;

	// Free displacement bounds array
	if ( g_pDispBounds )
	{
		free( g_pDispBounds );
		g_pDispBounds = NULL;
	}

	g_DispCollTreeCount = 0;

	if ( pBSPData->map_planes.Base() )
	{
		free( pBSPData->map_planes.Base() );
		pBSPData->map_planes.Detach();
	}

	if ( pBSPData->map_texturenames )
	{
		free( pBSPData->map_texturenames );
		pBSPData->map_texturenames = NULL;
	}

	if ( pBSPData->map_surfaces.Base() )
	{
		free( pBSPData->map_surfaces.Base() );
		pBSPData->map_surfaces.Detach();
	}

	if ( pBSPData->map_areaportals.Base() )
	{
		free( pBSPData->map_areaportals.Base() );
		pBSPData->map_areaportals.Detach();
	}

	if ( pBSPData->portalopen.Base() )
	{
		free( pBSPData->portalopen.Base() );
		pBSPData->portalopen.Detach();
	}

	if ( pBSPData->map_areas.Base() )
	{
		free( pBSPData->map_areas.Base() );
		pBSPData->map_areas.Detach();
	}

	if ( pBSPData->map_entitystring.Base() )
	{
		free( pBSPData->map_entitystring.Base() );
		pBSPData->map_entitystring.Detach();
	}

	if ( pBSPData->map_brushes.Base() )
	{
		free( pBSPData->map_brushes.Base() );
		pBSPData->map_brushes.Detach();
	}

	if ( pBSPData->map_cmodels.Base() )
	{
		free( pBSPData->map_cmodels.Base() );
		pBSPData->map_cmodels.Detach();
	}

	if ( pBSPData->map_leafbrushes.Base() )
	{
		free( pBSPData->map_leafbrushes.Base() );
		pBSPData->map_leafbrushes.Detach();
	}

	if ( pBSPData->map_leafs.Base() )
	{
		free( pBSPData->map_leafs.Base() );
		pBSPData->map_leafs.Detach();
	}

	if ( pBSPData->map_nodes.Base() )
	{
		free( pBSPData->map_nodes.Base() );
		pBSPData->map_nodes.Detach();
	}

	if ( pBSPData->map_brushsides.Base() )
	{
		free( pBSPData->map_brushsides.Base() );
		pBSPData->map_brushsides.Detach();
	}

	if ( pBSPData->map_vis )
	{
		free( pBSPData->map_vis );
		pBSPData->map_vis = NULL;
	}

	pBSPData->numplanes = 0;
	pBSPData->numbrushsides = 0;
	pBSPData->emptyleaf = pBSPData->solidleaf =0;
	pBSPData->numnodes = 0;
	pBSPData->numleafs = 0;
	pBSPData->numbrushes = 0;
	pBSPData->numleafbrushes = 0;
	pBSPData->numareas = 0;
	pBSPData->numtextures = 0;
	pBSPData->floodvalid = 0;
	pBSPData->numareaportals = 0;
	pBSPData->numclusters = 0;
	pBSPData->numcmodels = 0;
	pBSPData->numvisibility = 0;
	pBSPData->numentitychars = 0;
	pBSPData->numportalopen = 0;
	pBSPData->map_name[0] = 0;
	pBSPData->map_rootnode = NULL;
}

//-----------------------------------------------------------------------------
// Returns the collision tree associated with the ith displacement
//-----------------------------------------------------------------------------

CDispCollTree* CollisionBSPData_GetCollisionTree( int i )
{
	if ((i < 0) || (i >= g_DispCollTreeCount))
		return 0;

	return &g_pDispCollTrees[i];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LinkPhysics( void )
{
	//
	// initialize the physics surface properties -- if necessary!
	//
	if( !physprop )
	{
		physprop = ( IPhysicsSurfaceProps* )physicsFactory( VPHYSICS_SURFACEPROPS_INTERFACE_VERSION, NULL );
		physcollision = ( IPhysicsCollision* )physicsFactory( VPHYSICS_COLLISION_INTERFACE_VERSION, NULL );

		if ( !physprop || !physcollision )
		{
			Sys_Error( "CollisionBSPData_PreLoad: Can't link physics" );
		}
	}
}


//=============================================================================
//
// Loading Functions
//

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_PreLoad( CCollisionBSPData *pBSPData )
{
	// initialize the collision bsp data
	CollisionBSPData_Init( pBSPData ); 
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CollisionBSPData_Load( const char *pName, CCollisionBSPData *pBSPData )
{
	// This is a table that maps texinfo references to csurface_t
	// It is freed after the map has been loaded
	CUtlVector<unsigned short> 	map_texinfo;

	// copy map name
	strcpy( pBSPData->map_name, pName );

	//
	// load bsp file data
	//
	CollisionBSPData_LoadTextures( pBSPData );
	CollisionBSPData_LoadTexinfo( pBSPData, map_texinfo );
	CollisionBSPData_LoadLeafs( pBSPData );
	CollisionBSPData_LoadLeafBrushes( pBSPData );
	CollisionBSPData_LoadPlanes( pBSPData );
	CollisionBSPData_LoadBrushes( pBSPData );
	CollisionBSPData_LoadBrushSides( pBSPData, map_texinfo );
	CollisionBSPData_LoadSubmodels( pBSPData );
	CollisionBSPData_LoadNodes( pBSPData );
	CollisionBSPData_LoadAreas( pBSPData );
	CollisionBSPData_LoadAreaPortals( pBSPData );
	CollisionBSPData_LoadVisibility( pBSPData );
	CollisionBSPData_LoadEntityString( pBSPData );
	CollisionBSPData_LoadPhysics( pBSPData );
    CollisionBSPData_LoadDispInfo( pBSPData );
	CollisionBSPData_LoadMapFlags( pBSPData );

	return true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadTextures( CCollisionBSPData *pBSPData )
{
	CMapLoadHelper lh( LUMP_TEXDATA );

	CMapLoadHelper lhStringData( LUMP_TEXDATA_STRING_DATA );
	const char *pStringData = ( const char * )lhStringData.LumpBase();

	CMapLoadHelper lhStringTable( LUMP_TEXDATA_STRING_TABLE );
	if( lhStringTable.LumpSize() % sizeof( int ) )
		Sys_Error( "CMod_LoadTextures: funny lump size");
	int *pStringTable = ( int * )lhStringTable.LumpBase();

	dtexdata_t	*in;
	int			i, count;
	bool		found;
	IMaterial	*material;

	in = (dtexdata_t *)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error( "CMod_LoadTextures: funny lump size");
	}
	count = lh.LumpSize() / sizeof(*in);
	if (count < 1)
	{
		Sys_Error( "Map with no textures");
	}
	if (count > MAX_MAP_TEXDATA)
	{
		Sys_Error( "Map has too many textures");
	}

	int nSize = count * sizeof(csurface_t);
	pBSPData->map_surfaces.Attach( count, (csurface_t*)malloc( nSize ) );
	memset( pBSPData->map_surfaces.Base(), 0, nSize );

	pBSPData->numtextures = count;

	pBSPData->map_texturenames = (char *)malloc( lhStringData.LumpSize() * sizeof(char) );
	memcpy( pBSPData->map_texturenames, pStringData, lhStringData.LumpSize() );
 
	for ( i=0 ; i<count ; i++, in++ )
	{
		const char *pInName = &pStringData[pStringTable[in->nameStringTableID]];
		int index = pInName - pStringData;
		
		csurface_t *out = &pBSPData->map_surfaces[i];
		out->name = &pBSPData->map_texturenames[index];
		out->surfaceProps = 0;
		out->flags = 0;

		material = materialSystemInterface->FindMaterial( pBSPData->map_surfaces[i].name, &found, true );
		if ( found )
		{
			IMaterialVar *var;
			bool varFound;
			var = material->FindVar( "$surfaceprop", &varFound, false );
			if ( varFound )
			{
				const char *pProps = var->GetStringValue();
				pBSPData->map_surfaces[i].surfaceProps = physprop->GetSurfaceIndex( pProps );
			}
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadTexinfo( CCollisionBSPData *pBSPData, 
									CUtlVector<unsigned short> &map_texinfo )
{
	CMapLoadHelper lh( LUMP_TEXINFO );

	texinfo_t	*in;
	unsigned short	out;
	int			i, count;

	in = (texinfo_t *)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
		Sys_Error( "CollisionBSPData_LoadTexinfo: funny lump size");
	count = lh.LumpSize() / sizeof(*in);
	if (count < 1)
		Sys_Error( "Map with no texinfo");
	if (count > MAX_MAP_TEXINFO)
		Sys_Error( "Map has too many surfaces");

	map_texinfo.RemoveAll();
	map_texinfo.EnsureCapacity( count );

	for ( i=0 ; i<count ; i++, in++ )
	{
		out = in->texdata;
		
		if ( out >= pBSPData->numtextures )
			out = 0;

		// HACKHACK: Copy this over for the whole material!!!
		pBSPData->map_surfaces[out].flags |= in->flags;
		map_texinfo.AddToTail(out);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Load leafs - Lump version 0 (v19 BSP with embedded ambient lighting, 56 bytes)
//-----------------------------------------------------------------------------
static void CollisionBSPData_LoadLeafs_Version_0( CCollisionBSPData *pBSPData, CMapLoadHelper &lh )
{
	int			i;
	dleaf_v0_t 	*in;
	int			count;

	in = (dleaf_v0_t *)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error( "CollisionBSPData_LoadLeafs: funny lump size (v0)");
	}

	count = lh.LumpSize() / sizeof(*in);

	if (count < 1)
	{
		Sys_Error( "Map with no leafs");
	}

	// need to save space for box planes
	if (count > MAX_MAP_PLANES)
	{
		Sys_Error( "Map has too many planes");
	}

	// Need an extra one for the emptyleaf below, another extra one for CM_InitBoxHull
	int nSize = (count + 2) * sizeof(cleaf_t);
	pBSPData->map_leafs.Attach( count + 2, (cleaf_t*)malloc( nSize ) );
	memset( pBSPData->map_leafs.Base(), 0, nSize );

	pBSPData->numleafs = count;
	pBSPData->numclusters = 0;

	Con_DPrintf( "Loading %d leaves (lump v0, 56 bytes each)\n", count );

	for ( i=0 ; i<count ; i++, in++ )
	{
		cleaf_t	*out = &pBSPData->map_leafs[i];
		out->contents = in->contents;
		out->cluster = in->cluster;
		out->area = in->area;		// Bitfield extraction works directly
		out->firstleafbrush = in->firstleafbrush;
		out->numleafbrushes = in->numleafbrushes;
		out->m_pDisplacements = NULL;

		if (out->cluster >= pBSPData->numclusters)
		{
			pBSPData->numclusters = out->cluster + 1;
		}
	}

	if (pBSPData->map_leafs[0].contents != CONTENTS_SOLID)
	{
		Sys_Error( "Map leaf 0 is not CONTENTS_SOLID");
	}

	pBSPData->solidleaf = 0;
	pBSPData->emptyleaf = pBSPData->numleafs;
	memset( &pBSPData->map_leafs[pBSPData->emptyleaf], 0, sizeof(pBSPData->map_leafs[pBSPData->emptyleaf]) );
	pBSPData->numleafs++;
}

//-----------------------------------------------------------------------------
// Purpose: Load leafs - Lump version 1 (v20+ BSP, 32 bytes, ambient in separate lump)
//-----------------------------------------------------------------------------
static void CollisionBSPData_LoadLeafs_Version_1( CCollisionBSPData *pBSPData, CMapLoadHelper &lh )
{
	int			i;
	dleaf_v1_t 	*in;
	int			count;

	in = (dleaf_v1_t *)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error( "CollisionBSPData_LoadLeafs: funny lump size (v1)");
	}

	count = lh.LumpSize() / sizeof(*in);

	if (count < 1)
	{
		Sys_Error( "Map with no leafs");
	}

	// need to save space for box planes
	if (count > MAX_MAP_PLANES)
	{
		Sys_Error( "Map has too many planes");
	}

	// Need an extra one for the emptyleaf below, another extra one for CM_InitBoxHull
	int nSize = (count + 2) * sizeof(cleaf_t);
	pBSPData->map_leafs.Attach( count + 2, (cleaf_t*)malloc( nSize ) );
	memset( pBSPData->map_leafs.Base(), 0, nSize );

	pBSPData->numleafs = count;
	pBSPData->numclusters = 0;

	Con_DPrintf( "Loading %d leaves (lump v1, 32 bytes each)\n", count );

	for ( i=0 ; i<count ; i++, in++ )
	{
		cleaf_t	*out = &pBSPData->map_leafs[i];
		out->contents = in->contents;
		out->cluster = in->cluster;
		out->area = in->area;		// Bitfield extraction works directly
		out->firstleafbrush = in->firstleafbrush;
		out->numleafbrushes = in->numleafbrushes;
		out->m_pDisplacements = NULL;

		if (out->cluster >= pBSPData->numclusters)
		{
			pBSPData->numclusters = out->cluster + 1;
		}
	}

	if (pBSPData->map_leafs[0].contents != CONTENTS_SOLID)
	{
		Sys_Error( "Map leaf 0 is not CONTENTS_SOLID");
	}

	pBSPData->solidleaf = 0;
	pBSPData->emptyleaf = pBSPData->numleafs;
	memset( &pBSPData->map_leafs[pBSPData->emptyleaf], 0, sizeof(pBSPData->map_leafs[pBSPData->emptyleaf]) );
	pBSPData->numleafs++;
}

//-----------------------------------------------------------------------------
// Purpose: Load leafs - v18 native (LeakNet original, 32 bytes, no bitfields)
//-----------------------------------------------------------------------------
static void CollisionBSPData_LoadLeafs_Native( CCollisionBSPData *pBSPData, CMapLoadHelper &lh )
{
	int			i;
	dleaf_t 	*in;
	int			count;

	in = (dleaf_t *)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error( "CollisionBSPData_LoadLeafs: funny lump size (native)");
	}

	count = lh.LumpSize() / sizeof(*in);

	if (count < 1)
	{
		Sys_Error( "Map with no leafs");
	}

	// need to save space for box planes
	if (count > MAX_MAP_PLANES)
	{
		Sys_Error( "Map has too many planes");
	}

	// Need an extra one for the emptyleaf below, another extra one for CM_InitBoxHull
	int nSize = (count + 2) * sizeof(cleaf_t);
	pBSPData->map_leafs.Attach( count + 2, (cleaf_t*)malloc( nSize ) );
	memset( pBSPData->map_leafs.Base(), 0, nSize );

	pBSPData->numleafs = count;
	pBSPData->numclusters = 0;

	Con_DPrintf( "Loading %d leaves (v18 native, 32 bytes each)\n", count );

	for ( i=0 ; i<count ; i++, in++ )
	{
		cleaf_t	*out = &pBSPData->map_leafs[i];
		out->contents = LittleLong(in->contents);
		out->cluster = LittleShort(in->cluster);
		out->area = LittleShort(in->area);
		out->firstleafbrush = (unsigned short)LittleShort(in->firstleafbrush);
		out->numleafbrushes = (unsigned short)LittleShort(in->numleafbrushes);
		out->m_pDisplacements = NULL;

		if (out->cluster >= pBSPData->numclusters)
		{
			pBSPData->numclusters = out->cluster + 1;
		}
	}

	if (pBSPData->map_leafs[0].contents != CONTENTS_SOLID)
	{
		Sys_Error( "Map leaf 0 is not CONTENTS_SOLID");
	}

	pBSPData->solidleaf = 0;
	pBSPData->emptyleaf = pBSPData->numleafs;
	memset( &pBSPData->map_leafs[pBSPData->emptyleaf], 0, sizeof(pBSPData->map_leafs[pBSPData->emptyleaf]) );
	pBSPData->numleafs++;
}

//-----------------------------------------------------------------------------
// Purpose: Load leafs with multi-version support
//          Dispatches based on LUMP VERSION (not BSP version!) like 2007 engine
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadLeafs( CCollisionBSPData *pBSPData )
{
	CMapLoadHelper lh( LUMP_LEAFS );
	int nBSPVersion = GetCurrentBSPVersion();

	// v18 BSP has no lump versioning, use native loader
	if ( BSP_VERSION_HAS_AVGCOLOR( nBSPVersion ) )
	{
		CollisionBSPData_LoadLeafs_Native( pBSPData, lh );
		return;
	}

	// v19+ BSP: Dispatch based on LUMP VERSION (critical for v20 support!)
	switch( lh.LumpVersion() )
	{
	case 0:
		// Lump version 0: v19 format with embedded ambient lighting (56 bytes)
		CollisionBSPData_LoadLeafs_Version_0( pBSPData, lh );
		break;
	case 1:
		// Lump version 1: v20+ format with ambient in separate lump (32 bytes)
		CollisionBSPData_LoadLeafs_Version_1( pBSPData, lh );
		break;
	default:
		Sys_Error( "Unknown LUMP_LEAFS version %d\n", lh.LumpVersion() );
		break;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadLeafBrushes( CCollisionBSPData *pBSPData )
{
	CMapLoadHelper lh( LUMP_LEAFBRUSHES );

	int			i;
	unsigned short 	*in;
	int			count;
	
	in = (unsigned short *)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error( "CMod_LoadLeafBrushes: funny lump size");
	}

	count = lh.LumpSize() / sizeof(*in);
	if (count < 1)
	{
		Sys_Error( "Map with no planes");
	}

	// need to save space for box planes
	if (count > MAX_MAP_LEAFBRUSHES)
	{
		Sys_Error( "Map has too many leafbrushes");
	}

	// Extra one added for CM_InitBoxHull
	pBSPData->map_leafbrushes.Attach( count + 1, (unsigned short*)malloc( ( count + 1 ) * sizeof(unsigned short) ) );
	pBSPData->numleafbrushes = count;

	for ( i=0 ; i<count ; i++, in++)
	{
		pBSPData->map_leafbrushes[i] = (unsigned short)LittleShort (*in);
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadPlanes( CCollisionBSPData *pBSPData )
{
	CMapLoadHelper lh( LUMP_PLANES );

	int			i, j;
	dplane_t 	*in;
	int			count;
	int			bits;
	
	in = (dplane_t *)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error( "CollisionBSPData_LoadPlanes: funny lump size");
	}

	count = lh.LumpSize() / sizeof(*in);

	if (count < 1)
	{
		Sys_Error( "Map with no planes");
	}

	// need to save space for box planes
	if (count > MAX_MAP_PLANES)
	{
		Sys_Error( "Map has too many planes");
	}

	// Add extra room for CM_InitBoxHull
	int nSize = ( count + 12 ) * sizeof(cplane_t);
	pBSPData->map_planes.Attach( count + 12, (cplane_t*)malloc( nSize ) );
	memset( pBSPData->map_planes.Base(), 0, nSize );

	pBSPData->numplanes = count;

	for ( i=0 ; i<count ; i++, in++)
	{
		cplane_t *out = &pBSPData->map_planes[i];	
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0)
			{
				bits |= 1<<j;
			}
		}

		out->dist = LittleFloat (in->dist);
		out->type = LittleLong (in->type);
		out->signbits = bits;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadBrushes( CCollisionBSPData *pBSPData )
{
	CMapLoadHelper lh( LUMP_BRUSHES );

	dbrush_t	*in;
	int			i, count;
	
	in = (dbrush_t *)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error( "CMod_LoadBrushes: funny lump size");
	}

	count = lh.LumpSize() / sizeof(*in);
	if (count > MAX_MAP_BRUSHES)
	{
		Sys_Error( "Map has too many brushes");
	}

	// Extra one for CM_InitBoxHull
	int nSize = ( count + 1 ) * sizeof(cbrush_t);
	pBSPData->map_brushes.Attach( count+1, (cbrush_t*)malloc( nSize ) );
	memset( pBSPData->map_brushes.Base(), 0, nSize );

	pBSPData->numbrushes = count;

	for (i=0 ; i<count ; i++, in++)
	{
		cbrush_t *out = &pBSPData->map_brushes[i];
		out->firstbrushside = LittleLong(in->firstside);
		out->numsides = LittleLong(in->numsides);
		out->contents = LittleLong(in->contents);
		out->next = NULL;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Multi-version brushside loading
// v18-v20: 8-byte structure with short bevel field
// v21+: 8-byte structure with byte bevel and byte thin fields
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadBrushSides( CCollisionBSPData *pBSPData, CUtlVector<unsigned short> &map_texinfo )
{
	CMapLoadHelper lh( LUMP_BRUSHSIDES );

	int				i, j;
	int				count;
	int				num;

	// Determine brushside structure size based on BSP version
	int nBSPVersion = GetCurrentBSPVersion();
	bool bV21Format = BSP_VERSION_HAS_THIN_BRUSHSIDE( nBSPVersion );

	// Both v18-v20 and v21 brushsides are 8 bytes, just with different field layouts
	int nBrushSideSize = bV21Format ? sizeof(dbrushside_v21_t) : sizeof(dbrushside_t);

	byte *pBrushSideData = (byte *)lh.LumpBase();
	if (lh.LumpSize() % nBrushSideSize)
	{
		Sys_Error( "CMod_LoadBrushSides: funny lump size (size=%d, elem=%d)", lh.LumpSize(), nBrushSideSize);
	}

	count = lh.LumpSize() / nBrushSideSize;

	// need to save space for box planes
	if (count > MAX_MAP_BRUSHSIDES)
	{
		Sys_Error( "Map has too many planes");
	}

	// Extra 6 for CM_InitBoxHull
	int nSize = ( count + 6 ) * sizeof(cbrushside_t);
	pBSPData->map_brushsides.Attach( count + 6, (cbrushside_t*)malloc( nSize ) );
	memset( pBSPData->map_brushsides.Base(), 0, nSize );

	pBSPData->numbrushsides = count;

	Con_DPrintf( "Loading %d brushsides (BSP v%d, %s format)\n", count, nBSPVersion, bV21Format ? "v21" : "v18-v20" );

	for ( i=0 ; i<count ; i++ )
	{
		cbrushside_t *out = &pBSPData->map_brushsides[i];
		unsigned short planenum;
		short texinfo;
		bool bevel;

		if ( bV21Format )
		{
			// v21+ format with byte bevel and thin fields
			dbrushside_v21_t *in = (dbrushside_v21_t *)(pBrushSideData + i * nBrushSideSize);
			planenum = (unsigned short)LittleShort(in->planenum);
			texinfo = LittleShort(in->texinfo);
			bevel = (in->bevel != 0);
		}
		else
		{
			// v18-v20 format with short bevel field
			dbrushside_t *in = (dbrushside_t *)(pBrushSideData + i * nBrushSideSize);
			planenum = (unsigned short)LittleShort(in->planenum);
			texinfo = LittleShort(in->texinfo);
			bevel = (LittleShort(in->bevel) != 0);
		}

		num = planenum;
		out->plane = &pBSPData->map_planes[num];
		j = texinfo;
		if (j >= map_texinfo.Size())
		{
			Sys_Error( "Bad brushside texinfo");
		}

		// BUGBUG: Why is vbsp writing out -1 as the texinfo id?  (TEXINFO_NODE ?)
		if ( j < 0 )
		{
			out->surface = &nullsurface;
		}
		else
		{
			out->surface = &pBSPData->map_surfaces[map_texinfo[j]];
		}

		out->bBevel = bevel ? TRUE : FALSE;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadSubmodels( CCollisionBSPData *pBSPData )
{
	CMapLoadHelper lh( LUMP_MODELS );

	dmodel_t	*in;
	int			i, j, count;

	in = (dmodel_t *)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
		Sys_Error("CMod_LoadSubmodels: funny lump size");
	count = lh.LumpSize() / sizeof(*in);

	if (count < 1)
		Sys_Error( "Map with no models" );
	if (count > MAX_MAP_MODELS)
		Sys_Error( "Map has too many models" );

	int nSize = count * sizeof(cmodel_t);
	pBSPData->map_cmodels.Attach( count, (cmodel_t*)malloc( nSize ) );
	memset( pBSPData->map_cmodels.Base(), 0, nSize );
	pBSPData->numcmodels = count;

	for ( i=0 ; i<count ; i++, in++ )
	{
		cmodel_t *out = &pBSPData->map_cmodels[i];

		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
			out->origin[j] = LittleFloat (in->origin[j]);
		}
		out->headnode = LittleLong (in->headnode);
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadNodes( CCollisionBSPData *pBSPData )
{
	CMapLoadHelper lh( LUMP_NODES );

	dnode_t		*in;
	int			child;
	int			i, j, count;
	
	in = (dnode_t *)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
		Sys_Error( "CollisionBSPData_LoadNodes: funny lump size");
	count = lh.LumpSize() / sizeof(*in);

	if (count < 1)
		Sys_Error( "Map has no nodes");
	if (count > MAX_MAP_NODES)
		Sys_Error( "Map has too many nodes");

	// 6 extra for box hull
	int nSize = ( count + 6 ) * sizeof(cnode_t);
	pBSPData->map_nodes.Attach( count + 6, (cnode_t*)malloc( nSize ) );
	memset( pBSPData->map_nodes.Base(), 0, nSize );

	pBSPData->numnodes = count;
	pBSPData->map_rootnode = pBSPData->map_nodes.Base();

	for (i=0 ; i<count ; i++, in++)
	{
		cnode_t	*out = &pBSPData->map_nodes[i];
		out->plane = &pBSPData->map_planes[ LittleLong(in->planenum) ];
		for (j=0 ; j<2 ; j++)
		{
			child = LittleLong (in->children[j]);
			out->children[j] = child;
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadAreas( CCollisionBSPData *pBSPData )
{
	CMapLoadHelper lh( LUMP_AREAS );

	int			i;
	darea_t 	*in;
	int			count;

	in = (darea_t *)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error( "CMod_LoadAreas: funny lump size");
	}

	count = lh.LumpSize() / sizeof(*in);
	if (count > MAX_MAP_AREAS)
	{
		Sys_Error( "Map has too many areas");
	}

	int nSize = count * sizeof(carea_t);
	pBSPData->map_areas.Attach( count, (carea_t*)malloc( nSize ) );
	memset( pBSPData->map_areas.Base(), 0, nSize );

	pBSPData->numareas = count;

	for ( i=0 ; i<count ; i++, in++)
	{
		carea_t	*out = &pBSPData->map_areas[i];
		out->numareaportals = LittleLong (in->numareaportals);
		out->firstareaportal = LittleLong (in->firstareaportal);
		out->floodvalid = 0;
		out->floodnum = 0;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadAreaPortals( CCollisionBSPData *pBSPData )
{
	CMapLoadHelper lh( LUMP_AREAPORTALS );

	dareaportal_t 	*in;
	int				count;

	in = (dareaportal_t *)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error( "CMod_LoadAreaPortals: funny lump size");
	}
		   
	count = lh.LumpSize() / sizeof(*in);
	if (count > MAX_MAP_AREAPORTALS)
	{
		Sys_Error( "Map has too many area portals");
	}

	// Need to add one more in owing to 1-based instead of 0-based data!
	++count;

	pBSPData->numportalopen = count;
	pBSPData->portalopen.Attach( count, (qboolean*)malloc( pBSPData->numportalopen * sizeof(qboolean) ) );
	for ( int i=0; i < pBSPData->numportalopen; i++ )
	{
		pBSPData->portalopen[i] = false;
	}

	pBSPData->numareaportals = count;
	int nSize = count * sizeof(dareaportal_t);
	pBSPData->map_areaportals.Attach( count, (dareaportal_t*)malloc( nSize ) );
	memset( pBSPData->map_areaportals.Base(), 0, nSize );

	Assert( nSize >= lh.LumpSize() ); 
	memcpy( pBSPData->map_areaportals.Base(), in, lh.LumpSize() );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadVisibility( CCollisionBSPData *pBSPData )
{
	CMapLoadHelper lh( LUMP_VISIBILITY );

	int		i;

	pBSPData->numvisibility = lh.LumpSize();
	if (lh.LumpSize() > MAX_MAP_VISIBILITY)
		Sys_Error( "Map has too large visibility lump");

	int visDataSize = lh.LumpSize();
	if ( visDataSize == 0 )
	{
		pBSPData->map_vis = NULL;
	}
	else
	{
		pBSPData->map_vis = (dvis_t *) malloc( visDataSize );
		memcpy( pBSPData->map_vis, lh.LumpBase(), visDataSize );

		pBSPData->map_vis->numclusters = LittleLong (pBSPData->map_vis->numclusters);
		for (i=0 ; i<pBSPData->map_vis->numclusters ; i++)
		{
			pBSPData->map_vis->bitofs[i][0] = LittleLong (pBSPData->map_vis->bitofs[i][0]);
			pBSPData->map_vis->bitofs[i][1] = LittleLong (pBSPData->map_vis->bitofs[i][1]);
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadEntityString( CCollisionBSPData *pBSPData )
{
	CMapLoadHelper lh( LUMP_ENTITIES );

	pBSPData->numentitychars = lh.LumpSize();
	if (lh.LumpSize() > MAX_MAP_ENTSTRING)
		Sys_Error( "Map has too large entity lump");

	pBSPData->map_entitystring.Attach( pBSPData->numentitychars, (char*)malloc( lh.LumpSize() ) );
	memcpy( pBSPData->map_entitystring.Base(), lh.LumpBase(), lh.LumpSize() );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadPhysics( CCollisionBSPData *pBSPData )
{
	CMapLoadHelper lh( LUMP_PHYSCOLLIDE );

	if ( !lh.LumpSize() )
		return;

	byte *ptr = lh.LumpBase();
	byte *basePtr = ptr;

	dphysmodel_t physModel;

	// physics data is variable length.  The last physmodel is a NULL pointer
	// with modelIndex -1, dataSize -1
	do
	{
		memcpy( &physModel, ptr, sizeof(physModel) );
		ptr += sizeof(physModel);

		if ( physModel.dataSize > 0 )
		{
			cmodel_t *pModel = &pBSPData->map_cmodels[ physModel.modelIndex ];
			physcollision->VCollideLoad( &pModel->vcollisionData, physModel.solidCount, (const char *)ptr, physModel.dataSize + physModel.keydataSize );
			ptr += physModel.dataSize;
			ptr += physModel.keydataSize;
		}
		
		// avoid infinite loop on badly formed file
		if ( (int)(ptr - basePtr) > lh.LumpSize() )
			break;

	} while ( physModel.dataSize > 0 );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadDispInfo( CCollisionBSPData *pBSPData )
{
	// How many displacements in the map?
	int coreDispCount = CMapLoadHelper::LumpSize( LUMP_DISPINFO ) / sizeof( ddispinfo_t );
	if( coreDispCount == 0 )
		return;	

    //
    // get the vertex data
    //
 	CMapLoadHelper lhv( LUMP_VERTEXES );
	dvertex_t *pVerts = ( dvertex_t* )lhv.LumpBase();
	if( lhv.LumpSize() % sizeof( dvertex_t ) )
		Sys_Error( "CMod_LoadDispInfo: bad vertex lump size!" );

    //
    // get the edge data
    //
 	CMapLoadHelper lhe( LUMP_EDGES );
    dedge_t *pEdges = ( dedge_t* )lhe.LumpBase();
    if( lhe.LumpSize() % sizeof( dedge_t ) )
        Sys_Error( "CMod_LoadDispInfo: bad edge lump size!" );

    //
    // get surf edges data
    //
 	CMapLoadHelper lhs( LUMP_SURFEDGES );
    int *pSurfEdges = ( int* )lhs.LumpBase();
    if( lhs.LumpSize() % sizeof( int ) )
        Sys_Error( "CMod_LoadDispInfo: bad surf edge lump size!" );

    //
    // get face data - handle multi-version BSP formats
    //
 	CMapLoadHelper lhf( LUMP_FACES );
	int nBSPVersion = GetCurrentBSPVersion();

	dface_t *pFaces = NULL;
	dface_t *pFaceBuffer = NULL;  // For v19+ conversion
	int faceCount = 0;

	if ( BSP_VERSION_HAS_AVGCOLOR( nBSPVersion ) )
	{
		// v18: Native format with m_AvgLightColor (72 bytes)
		pFaces = ( dface_t* )lhf.LumpBase();
		if( lhf.LumpSize() % sizeof( dface_t ) )
			Sys_Error( "CMod_LoadDispInfo: bad face lump size (v18)!" );
		faceCount = lhf.LumpSize() / sizeof( dface_t );
	}
	else
	{
		// v19+: Smaller format without m_AvgLightColor (56 bytes)
		dface_v19_t *pV19Faces = ( dface_v19_t* )lhf.LumpBase();
		if( lhf.LumpSize() % sizeof( dface_v19_t ) )
			Sys_Error( "CMod_LoadDispInfo: bad face lump size (v19+)!" );
		faceCount = lhf.LumpSize() / sizeof( dface_v19_t );

		// Allocate temporary buffer and convert to internal v18 format
		// We only need the dispinfo field, but convert fully for consistency
		pFaceBuffer = new dface_t[faceCount];
		for ( int i = 0; i < faceCount; i++ )
		{
			// Clear the v18-only m_AvgLightColor field
			memset( pFaceBuffer[i].m_AvgLightColor, 0, sizeof( pFaceBuffer[i].m_AvgLightColor ) );

			// Copy all common fields from v19 to v18 structure
			pFaceBuffer[i].planenum = pV19Faces[i].planenum;
			pFaceBuffer[i].side = pV19Faces[i].side;
			pFaceBuffer[i].onNode = pV19Faces[i].onNode;
			pFaceBuffer[i].firstedge = pV19Faces[i].firstedge;
			pFaceBuffer[i].numedges = pV19Faces[i].numedges;
			pFaceBuffer[i].texinfo = pV19Faces[i].texinfo;
			pFaceBuffer[i].dispinfo = pV19Faces[i].dispinfo;
			pFaceBuffer[i].surfaceFogVolumeID = pV19Faces[i].surfaceFogVolumeID;
			memcpy( pFaceBuffer[i].styles, pV19Faces[i].styles, sizeof( pFaceBuffer[i].styles ) );
			pFaceBuffer[i].lightofs = pV19Faces[i].lightofs;
			pFaceBuffer[i].area = pV19Faces[i].area;
			pFaceBuffer[i].m_LightmapTextureMinsInLuxels[0] = pV19Faces[i].m_LightmapTextureMinsInLuxels[0];
			pFaceBuffer[i].m_LightmapTextureMinsInLuxels[1] = pV19Faces[i].m_LightmapTextureMinsInLuxels[1];
			pFaceBuffer[i].m_LightmapTextureSizeInLuxels[0] = pV19Faces[i].m_LightmapTextureSizeInLuxels[0];
			pFaceBuffer[i].m_LightmapTextureSizeInLuxels[1] = pV19Faces[i].m_LightmapTextureSizeInLuxels[1];
			pFaceBuffer[i].origFace = pV19Faces[i].origFace;
			pFaceBuffer[i].numPrims = pV19Faces[i].numPrims;
			pFaceBuffer[i].firstPrimID = pV19Faces[i].firstPrimID;
			pFaceBuffer[i].smoothingGroups = pV19Faces[i].smoothingGroups;
		}
		pFaces = pFaceBuffer;
	}

	dface_t *pFaceList = pFaces;
	if (!pFaceList)
	{
		delete[] pFaceBuffer;
		return;
	}

    //
    // get texinfo data
    //
 	CMapLoadHelper lhti( LUMP_TEXINFO );
    texinfo_t *pTexinfoList = ( texinfo_t* )lhti.LumpBase();
    if( lhti.LumpSize() % sizeof( texinfo_t ) )
        Sys_Error( "CMod_LoadDispInfo: bad texinfo lump size!" );

	// allocate displacement collision trees
	g_DispCollTreeCount = coreDispCount;
	g_pDispCollTrees = DispCollTrees_Alloc( g_DispCollTreeCount );
	g_pDispBounds = (alignedbbox_t *)malloc( g_DispCollTreeCount * sizeof(alignedbbox_t) );

	// Build the inverse mapping from disp index to face
	int nMemSize = coreDispCount * sizeof(unsigned short);
	unsigned short *pDispIndexToFaceIndex = (unsigned short*)stackalloc( nMemSize );
	memset( pDispIndexToFaceIndex, 0xFF, nMemSize );
	
	int i;
    for( i = 0; i < faceCount; ++i, ++pFaces )
    {
        // check face for displacement data
        if( pFaces->dispinfo == -1 )
            continue;

        // get the current displacement build surface
		if( pFaces->dispinfo >= coreDispCount )
			continue;

		pDispIndexToFaceIndex[pFaces->dispinfo] = (unsigned short)i;
    }

	// Load one dispinfo from disk at a time and set it up.
	int iCurVert = 0;
	int iCurTri = 0;
	CDispVert tempVerts[MAX_DISPVERTS];
	CDispTri  tempTris[MAX_DISPTRIS];

	for ( i = 0; i < coreDispCount; ++i )
	{
		// Find the face associated with this dispinfo
		unsigned short nFaceIndex = pDispIndexToFaceIndex[i];
		if (nFaceIndex == 0xFFFF)
			continue;

		// Load up the dispinfo and create the CCoreDispInfo from it.
		ddispinfo_t dispInfo;
		CMapLoadHelper::LoadLumpElement( LUMP_DISPINFO, i, sizeof(ddispinfo_t), &dispInfo );

		// Read in the vertices.
		int nVerts = NUM_DISP_POWER_VERTS( dispInfo.power );
		CMapLoadHelper::LoadLumpData( LUMP_DISP_VERTS, iCurVert * sizeof(CDispVert), nVerts*sizeof(CDispVert), tempVerts );
		iCurVert += nVerts;
		
		// Read in the tris.
		int nTris = NUM_DISP_POWER_TRIS( dispInfo.power );
		CMapLoadHelper::LoadLumpData( LUMP_DISP_TRIS, iCurTri * sizeof( CDispTri ), nTris*sizeof( CDispTri), tempTris );
		iCurTri += nTris;

		CCoreDispInfo coreDisp;
		CCoreDispSurface *pDispSurf = coreDisp.GetSurface();
		pDispSurf->SetPointStart( dispInfo.startPosition );
		pDispSurf->SetContents( dispInfo.contents );
	
		coreDisp.InitDispInfo( dispInfo.power, dispInfo.minTess, dispInfo.smoothingAngle, tempVerts, tempTris );

		// Hook the disp surface to the face
		pFaces = &pFaceList[ nFaceIndex ];
		pDispSurf->SetHandle( nFaceIndex );

		// get points
		if( pFaces->numedges > 4 )
			continue;

		Vector surfPoints[4];
		pDispSurf->SetPointCount( pFaces->numedges );
		int j;
		for( j = 0; j < pFaces->numedges; j++ )
		{
			int eIndex = pSurfEdges[pFaces->firstedge+j];
			if( eIndex < 0 )
			{
				VectorCopy( pVerts[pEdges[-eIndex].v[1]].point, surfPoints[j] );
			}
			else
			{
				VectorCopy( pVerts[pEdges[eIndex].v[0]].point, surfPoints[j] );
			}
		}

		for( j = 0; j < 4; j++ )
		{
			pDispSurf->SetPoint( j, surfPoints[j] );
		}

		pDispSurf->FindSurfPointStartIndex();
		pDispSurf->AdjustSurfPointData();

		//
		// generate the collision displacement surfaces
		//
		CDispCollTree *pDispTree = &g_pDispCollTrees[i];
		pDispTree->SetPower( 0 );

		//
		// check for null faces, should have been taken care of in vbsp!!!
		//
		int pointCount = pDispSurf->GetPointCount();
		if( pointCount != 4 )
			continue;

		coreDisp.Create();

		// new collision
		pDispTree->Create( &coreDisp );

		// Initialize displacement bounds for collision optimization
		Vector mins, maxs;
		pDispTree->GetBounds( mins, maxs );
		g_pDispBounds[i].Init( mins, maxs, i, pDispTree->GetContents() );

		// Surface props.
		texinfo_t *pTex = &pTexinfoList[pFaces->texinfo];
		bool bFound = false;
		if( pTex->texdata >= 0 )
		{
			IMaterial *pMaterial = materialSystemInterface->FindMaterial( pBSPData->map_surfaces[pTex->texdata].name, &bFound, true );
			if ( bFound )
			{
				IMaterialVar *pVar;
				bool bVarFound;
				pVar = pMaterial->FindVar( "$surfaceprop", &bVarFound, false );
				if ( bVarFound )
				{
					const char *pProps = pVar->GetStringValue();
					pDispTree->SetSurfaceProps( physprop->GetSurfaceIndex( pProps ) );
				}

				pVar = pMaterial->FindVar( "$surfaceprop2", &bVarFound, false );
				if ( bVarFound )
				{
					const char *pProps = pVar->GetStringValue();
					pDispTree->SetSurfaceProps2( physprop->GetSurfaceIndex( pProps ) );
				}
			}
		}
	}

	// Clean up temporary face buffer if we allocated one for v19+ conversion
	delete[] pFaceBuffer;
}

//-----------------------------------------------------------------------------
// Purpose: Load map flags (v21+ BSP format)
//          Provides information about map compilation features like HDR lighting
//-----------------------------------------------------------------------------
void CollisionBSPData_LoadMapFlags( CCollisionBSPData *pBSPData )
{
	// Initialize flags to default values
	pBSPData->map_flags_loaded = false;
	memset( &pBSPData->map_flags, 0, sizeof( dflagslump_t ) );

	// Check BSP version - map flags are only available in v21+
	int nBSPVersion = GetCurrentBSPVersion();
	if ( nBSPVersion < 21 )
	{
		Con_DPrintf( "Map flags not available in BSP v%d (requires v21+)\n", nBSPVersion );
		return;
	}

	CMapLoadHelper lh( LUMP_MAP_FLAGS );

	// Check if the lump exists and has data
	if ( lh.LumpSize() == 0 )
	{
		Con_DPrintf( "Map flags lump is empty (BSP v%d)\n", nBSPVersion );
		return;
	}

	// Validate lump size
	if ( lh.LumpSize() != sizeof( dflagslump_t ) )
	{
		Warning( "CollisionBSPData_LoadMapFlags: Invalid map flags lump size. Expected %d, got %d\n",
				 sizeof( dflagslump_t ), lh.LumpSize() );
		return;
	}

	// Load map flags data
	dflagslump_t *pMapFlags = (dflagslump_t *)lh.LumpBase();
	pBSPData->map_flags = *pMapFlags;
	pBSPData->map_flags_loaded = true;

	Con_DPrintf( "Loaded map flags: 0x%08X\n", pBSPData->map_flags.m_LevelFlags );

	// Log specific flag information
	if ( pBSPData->map_flags.m_LevelFlags & LVLFLAGS_BAKED_STATIC_PROP_LIGHTING_HDR )
	{
		Con_DPrintf( "  - HDR static prop lighting enabled\n" );
	}
	if ( pBSPData->map_flags.m_LevelFlags & LVLFLAGS_BAKED_STATIC_PROP_LIGHTING_NONHDR )
	{
		Con_DPrintf( "  - Non-HDR static prop lighting enabled\n" );
	}
}


//=============================================================================
//
// Collision Count Functions
//

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionCounts_Init( CCollisionCounts *pCounts )
{
	pCounts->m_PointContents = 0;
	pCounts->m_Traces = 0;
	pCounts->m_BrushTraces = 0;
	pCounts->m_DispTraces = 0;
	pCounts->m_Stabs = 0;
}

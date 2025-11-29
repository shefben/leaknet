//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef ISTUDIORENDER_H
#define ISTUDIORENDER_H
#ifdef _WIN32
#pragma once
#endif

#include "interface.h"
#include "vector.h"
#include "utlbuffer.h"
#include "studio.h"

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
struct studiohdr_t;
struct studiomeshdata_t;
class Vector;
struct LightDesc_t;
class IMaterial;
struct studiohwdata_t;
struct Ray_t;
class Vector4D;
class IMaterialSystem;
struct matrix3x4_t;
class IMesh;


// undone: what's the standard for function type naming?
typedef void (*StudioRender_Printf_t)( const char *fmt, ... );

struct StudioRenderConfig_t
{
	StudioRender_Printf_t pConPrintf;
	StudioRender_Printf_t pConDPrintf;
	float fEyeShiftX;	// eye X position
	float fEyeShiftY;	// eye Y position
	float fEyeShiftZ;	// eye Z position
	float fEyeSize;		// adjustment to iris textures
	float gamma;
	float texGamma;
	float brightness;
	float overbrightFactor;
	float modelLightBias;
	int eyeGloss;		// wet eyes
	int drawEntities;
	int skin;
	int fullbright;
	bool bEyeMove;		// look around
	bool bSoftwareSkin;
	bool bNoHardware;
	bool bNoSoftware;
	bool bTeeth;
	bool bEyes;
	bool bFlex;
	bool bWireframe;
	bool bNormals;
	bool bUseAmbientCube;
	bool bSoftwareLighting;
	bool bShowEnvCubemapOnly;
	int maxDecalsPerModel;
	bool bWireframeDecals;
	bool bStaticLighting;
	int r_speeds;
};


//-----------------------------------------------------------------------------
// Studio render interface
//-----------------------------------------------------------------------------

#define STUDIO_RENDER_INTERFACE_VERSION "VStudioRender011"

typedef unsigned short StudioDecalHandle_t;

enum
{
	STUDIORENDER_DECAL_INVALID = (StudioDecalHandle_t)~0
};

enum
{
	ADDDECAL_TO_ALL_LODS = -1
};

//-----------------------------------------------------------------------------
// DrawModel flags
//-----------------------------------------------------------------------------
enum
{
	STUDIORENDER_DRAW_ENTIRE_MODEL		= 0,
	STUDIORENDER_DRAW_OPAQUE_ONLY		= 0x1,
	STUDIORENDER_DRAW_TRANSLUCENT_ONLY	= 0x2,
	STUDIORENDER_DRAW_GROUP_MASK		= 0x3,

	STUDIORENDER_DRAW_NO_FLEXES			= 0x4,
	STUDIORENDER_DRAW_STATIC_LIGHTING	= 0x8,


	STUDIORENDER_DRAW_ACCURATETIME		= 0x10,		// Use accurate timing when drawing the model.
};


//-----------------------------------------------------------------------------
// What kind of material override is it?
//-----------------------------------------------------------------------------
enum OverrideType_t
{
	OVERRIDE_NORMAL = 0,
	OVERRIDE_BUILD_SHADOWS,
};


//-----------------------------------------------------------------------------
// DrawModel info
//-----------------------------------------------------------------------------

// Special flag for studio models that have a compiled in shadow lod version
// It's negative 2 since positive numbers == use a regular slot and -1 means
//  have studiorender compute a value instead
enum
{
	USESHADOWLOD = -2,
};

struct DrawModelInfo_t
{
	studiohdr_t *m_pStudioHdr;
	studiohwdata_t *m_pHardwareData;
	StudioDecalHandle_t m_Decals;
	int m_Skin;
	int m_Body;
	int m_HitboxSet;
	void *m_pClientEntity;
	int m_Lod;
	IMesh **m_ppColorMeshes;
	int m_ActualTriCount;
	int m_TextureMemoryBytes;
	CFastTimer m_RenderTime;
};

//-----------------------------------------------------------------------------
// GetTriangles support structures
//-----------------------------------------------------------------------------
struct GetTriangles_Vertex_t
{
	Vector m_Position;
	Vector m_Normal;
	Vector4D m_TangentS;
	Vector2D m_TexCoord;
	Vector4D m_BoneWeight;
	int m_BoneIndex[4];
	int m_NumBones;

	// Constructors and assignment operators to handle Vector4D members
	GetTriangles_Vertex_t()
	{
		m_NumBones = 0;
		memset( m_BoneIndex, 0, sizeof(m_BoneIndex) );
	}

	GetTriangles_Vertex_t( const GetTriangles_Vertex_t& src )
	{
		VectorCopy( src.m_Position, m_Position );
		VectorCopy( src.m_Normal, m_Normal );
		Vector4DCopy( src.m_TangentS, m_TangentS );
		Vector2DCopy( src.m_TexCoord, m_TexCoord );
		Vector4DCopy( src.m_BoneWeight, m_BoneWeight );
		memcpy( m_BoneIndex, src.m_BoneIndex, sizeof(m_BoneIndex) );
		m_NumBones = src.m_NumBones;
	}

	GetTriangles_Vertex_t& operator=( const GetTriangles_Vertex_t& src )
	{
		if ( this != &src )
		{
			VectorCopy( src.m_Position, m_Position );
			VectorCopy( src.m_Normal, m_Normal );
			Vector4DCopy( src.m_TangentS, m_TangentS );
			Vector2DCopy( src.m_TexCoord, m_TexCoord );
			Vector4DCopy( src.m_BoneWeight, m_BoneWeight );
			memcpy( m_BoneIndex, src.m_BoneIndex, sizeof(m_BoneIndex) );
			m_NumBones = src.m_NumBones;
		}
		return *this;
	}
};

struct GetTriangles_MaterialBatch_t
{
	IMaterial *m_pMaterial;
	CUtlVector<GetTriangles_Vertex_t> m_Verts;
	CUtlVector<int> m_TriListIndices;
};

struct GetTriangles_Output_t
{
	CUtlVector<GetTriangles_MaterialBatch_t> m_MaterialBatches;
	matrix3x4_t m_PoseToWorld[MAXSTUDIOBONES];
};


//-----------------------------------------------------------------------------
// Studio render interface
//-----------------------------------------------------------------------------
class IStudioRender
{
public:
	// Initializes, shutdowns the studio render library
	virtual bool Init( CreateInterfaceFn materialSystemFactory, CreateInterfaceFn materialSystemHWConfigFactory ) = 0;
	virtual void Shutdown( void ) = 0;

	virtual void BeginFrame( void ) = 0;
	virtual void EndFrame( void ) = 0;

	// Updates the rendering configuration 
	virtual void UpdateConfig( const StudioRenderConfig_t& config ) = 0;

	// HACK HACK - don't allocate memory in here.
	// Legacy LoadModel for v37 models with embedded vertex data
	virtual bool LoadModel(
		studiohdr_t		*pStudioHdr, 	// read from the mdl file.
		void			*pVtxHdr, 		// read from the vtx file.(format OptimizedModel::FileHeader_t)
		studiohwdata_t	*pHardwareData
		) = 0;

	//-----------------------------------------------------------------------------
	// v48 LoadModel with external VVD vertex data
	// For v44+ models, vertex data is stored in external .vvd files
	// pVvdHdr: Pointer to vertexFileHeader_t from VVD file (NULL for v37 models)
	//-----------------------------------------------------------------------------
	virtual bool LoadModelWithVertexData(
		studiohdr_t		*pStudioHdr,	// read from the mdl file
		void			*pVtxHdr,		// read from the vtx file (OptimizedModel::FileHeader_t)
		void			*pVvdHdr,		// read from the vvd file (vertexFileHeader_t), NULL for v37
		studiohwdata_t	*pHardwareData
		)
	{
		// Default implementation: fall back to legacy LoadModel (for v37 compatibility)
		// Derived implementations should override this for v48 support
		return LoadModel( pStudioHdr, pVtxHdr, pHardwareData );
	}

	// since studiomeshes are allocated inside of the lib, they need to be freed there as well.
	virtual void UnloadModel( studiohwdata_t *pHardwareData ) = 0;

	// This is needed to do eyeglint and calculate the correct texcoords for the eyes.
	virtual void SetEyeViewTarget( const Vector& worldPosition ) = 0;
		
	virtual int GetNumAmbientLightSamples() = 0;
	
	virtual const Vector *GetAmbientLightDirections() = 0;

	// assumes that the arraysize is the same as returned from GetNumAmbientLightSamples
	virtual void SetAmbientLightColors( const Vector *pAmbientOnlyColors, const Vector *pTotalColors ) = 0;
	
	virtual void SetLocalLights( int numLights, const LightDesc_t *pLights ) = 0;

	virtual void SetViewState( 	
		const Vector& viewOrigin,
		const Vector& viewRight,
		const Vector& viewUp,
		const Vector& viewPlaneNormal ) = 0;
	
	virtual void SetFlexWeights( int numWeights, const float *pWeights ) = 0;

	// fixme: these interfaces sucks. . use 'em to get this stuff working with the client dll
	// and then interate
	virtual matrix3x4_t* GetPoseToWorld(int i) = 0; // this will be hidden enntually (computed internally)
	virtual matrix3x4_t* GetBoneToWorld(int i) = 0;

	// NOTE: this array must have space for MAXSTUDIOBONES.
	virtual matrix3x4_t* GetBoneToWorldArray() = 0;
	
	// LOD stuff
	virtual int GetNumLODs( const studiohwdata_t &hardwareData ) const = 0;
	virtual float GetLODSwitchValue( const studiohwdata_t &hardwareData, int lod ) const = 0;
	virtual void SetLODSwitchValue( studiohwdata_t &hardwareData, int lod, float switchValue ) = 0;

	// Sets the color modulation
	virtual void SetColorModulation( float const* pColor ) = 0;
	virtual void SetAlphaModulation( float alpha ) = 0;
	
	// returns the number of triangles rendered.
	virtual int DrawModel( DrawModelInfo_t& info, const Vector &modelOrigin,
		int *pLodUsed, float *pMetric, int flags = STUDIORENDER_DRAW_ENTIRE_MODEL ) = 0;

	// Causes a material to be used instead of the materials the model was compiled with
	virtual void ForcedMaterialOverride( IMaterial *newMaterial, OverrideType_t nOverrideType = OVERRIDE_NORMAL ) = 0;

	// Create, destroy list of decals for a particular model
	virtual StudioDecalHandle_t CreateDecalList( studiohwdata_t *pHardwareData ) = 0;
	virtual void DestroyDecalList( StudioDecalHandle_t handle ) = 0;

	// Add decals to a decal list by doing a planar projection along the ray
	// The BoneToWorld matrices must be set before this is called
	virtual void AddDecal( StudioDecalHandle_t handle, studiohdr_t *pStudioHdr, const Ray_t & ray, 
		const Vector& decalUp, IMaterial* pDecalMaterial, float radius, int body, bool noPokethru = false, int maxLODToDecal = ADDDECAL_TO_ALL_LODS ) = 0;

	// Remove all the decals on a model
	virtual void RemoveAllDecals( StudioDecalHandle_t handle, studiohdr_t *pStudioHdr ) = 0;

	// Compute the lighting at a point and normal
	virtual void ComputeLighting( const Vector* pAmbient, int lightCount,
		LightDesc_t* pLights, const Vector& pt, const Vector& normal, Vector& lighting ) = 0;

	// Refresh the studiohdr since it was lost...
	virtual void RefreshStudioHdr( studiohdr_t* pStudioHdr, studiohwdata_t* pHardwareData ) = 0;

	// Used for the mat_stub console command.
	virtual void Mat_Stub( IMaterialSystem *pMatSys ) = 0;

	// Shadow state (affects the models as they are rendered)
	virtual void AddShadow( IMaterial* pMaterial, void* pProxyData ) = 0;
	virtual void ClearAllShadows() = 0;

	// Gets the model LOD; pass in the screen size in pixels of a sphere 
	// of radius 1 that has the same origin as the model to get the LOD out...
	virtual int ComputeModelLod( studiohwdata_t* pHardwareData, float unitSphereSize ) = 0;

	// Return a number that is usable for budgets, etc.
	// Things that we care about:
	// 1) effective triangle count (factors in batch sizes, state changes, etc)
	// 2) texture memory usage
	virtual void GetPerfStats( DrawModelInfo_t &info, CUtlBuffer *pSpewBuf = NULL ) const = 0;
	virtual void GetTriangles( const DrawModelInfo_t& info, matrix3x4_t *pBoneToWorld, GetTriangles_Output_t &out ) = 0;
};

extern IStudioRender *g_pStudioRender;

#endif // ISTUDIORENDER_H

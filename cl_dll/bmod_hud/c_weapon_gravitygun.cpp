//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "keydefs.h"
#include "hud.h"
#include "in_buttons.h"
#include "beamdraw.h"
#include "c_weapon__stubs.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterial.h"
#include "bmod_material_extensions.h"
#include "vmatrix.h"
// Note: IMaterialRenderContext didn't exist in 2003, using direct IMaterialSystem calls
#include "model_types.h"
#include "tier0/vprof.h"
#include "c_te_effect_dispatch.h"
#include "studio.h"
#include "engine/ivmodelrender.h"
#include "convar.h"

// External interfaces
extern IVModelRender *modelrender;
extern ConVar physgun_r;
extern ConVar physgun_g;
extern ConVar physgun_b;

//-----------------------------------------------------------------------------
// Physics Gun Effect States (matching Garry's Mod)
//-----------------------------------------------------------------------------
enum PhysGunEffectState_t
{
	EFFECT_NONE = 0,		// Inactive
	EFFECT_READY,			// Targeting valid object (blue theme)
	EFFECT_HOLDING,			// Holding object (orange theme)
	EFFECT_LAUNCH			// Launch animation
};

class C_BeamQuadratic : public CDefaultClientRenderable
{
public:
	C_BeamQuadratic();
	void			Update( C_BaseEntity *pOwner );
	void			UpdateHeldObjectGlow( void );
	void			UpdateEffectState( void );
	void			RenderPrimaryBeam( void );

	// Recomputes the beam end point from the client-side (interpolated) held
	// object every frame instead of using the stale networked world position.
	// This is what removes the visible beam lag - m_worldPosition only changes
	// at snapshot rate, the held prop's render transform changes every frame.
	void			CacheHeldObjectOffset( void );
	const Vector	&ComputeBeamEndPoint( void );

	// IClientRenderable
	virtual const Vector&			GetRenderOrigin( void ) { return m_vecBeamEnd; }
	virtual const QAngle&			GetRenderAngles( void ) { return vec3_angle; }
	virtual bool					ShouldDraw( void ) { return true; }
	virtual bool					IsTransparent( void ) { return true; }
	virtual bool					ShouldCacheRenderInfo() { return false;}
	virtual bool					ShouldReceiveShadows() { return false; }
	virtual int						DrawModel( int flags );

	// Returns the bounds relative to the origin (render bounds)
	virtual void	GetRenderBounds( Vector& mins, Vector& maxs )
	{
		// bogus.  But it should draw if you can see the end point
		mins.Init(-32,-32,-32);
		maxs.Init(32,32,32);
	}

	C_BaseEntity			*m_pOwner;
	Vector					m_targetPosition;
	Vector					m_worldPosition;
	ClientRenderHandle_t	m_handle;
	int						m_active;
	int						m_glueTouching;
	int						m_viewModelIndex;
	EHANDLE					m_heldObject;

	// Enhanced physics gun state (matching GMod)
	int						m_effectState;
	bool					m_bIsCurrentlyRotating;
	bool					m_bIsCurrentlyHolding;
	int						m_serversidebeams;

	// Client-only: grab point in the held object's local space, plus the entity
	// the offset was cached against. Not networked, not saved.
	Vector					m_vecHeldLocalOffset;
	EHANDLE					m_hOffsetEntity;
	Vector					m_vecBeamEnd;
};


class C_WeaponGravityGun : public C_BaseCombatWeapon
{
	DECLARE_CLASS( C_WeaponGravityGun, C_BaseCombatWeapon );
public:
	C_WeaponGravityGun() {}

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	int KeyInput( int down, int keynum, const char *pszCurrentBinding )
	{
		if ( gHUD.m_iKeyBits & IN_ATTACK )
		{
			switch ( keynum )
			{
			case K_MWHEELUP:
				gHUD.m_iKeyBits |= IN_WEAPON1;
				return 0;

			case K_MWHEELDOWN:
				gHUD.m_iKeyBits |= IN_WEAPON2;
				return 0;
			}
		}

		// Allow engine to process
		return BaseClass::KeyInput( down, keynum, pszCurrentBinding );
	}

	void OnDataChanged( DataUpdateType_t updateType )
	{
		BaseClass::OnDataChanged( updateType );
		m_beam.Update( this );
		m_beam.UpdateHeldObjectGlow();
	}

private:
	C_WeaponGravityGun( const C_WeaponGravityGun & );

	C_BeamQuadratic	m_beam;
};

STUB_WEAPON_CLASS_IMPLEMENT( weapon_physgun, C_WeaponGravityGun );

IMPLEMENT_CLIENTCLASS_DT( C_WeaponGravityGun, DT_WeaponGravityGun, CWeaponGravityGun )
	RecvPropVector( RECVINFO_NAME(m_beam.m_targetPosition,m_targetPosition) ),
	RecvPropVector( RECVINFO_NAME(m_beam.m_worldPosition, m_worldPosition) ),
	RecvPropInt( RECVINFO_NAME(m_beam.m_active, m_active) ),
	RecvPropInt( RECVINFO_NAME(m_beam.m_glueTouching, m_glueTouching) ),
	RecvPropInt( RECVINFO_NAME(m_beam.m_viewModelIndex, m_viewModelIndex) ),
	RecvPropEHandle( RECVINFO_NAME(m_beam.m_heldObject, m_heldObject) ),
	// Enhanced physics gun state (matching GMod)
	RecvPropInt( RECVINFO_NAME(m_beam.m_effectState, m_effectState) ),
	RecvPropBool( RECVINFO_NAME(m_beam.m_bIsCurrentlyRotating, m_bIsCurrentlyRotating) ),
	RecvPropBool( RECVINFO_NAME(m_beam.m_bIsCurrentlyHolding, m_bIsCurrentlyHolding) ),
	RecvPropInt( RECVINFO_NAME(m_beam.m_serversidebeams, m_serversidebeams) ),
END_RECV_TABLE()


C_BeamQuadratic::C_BeamQuadratic()
{
	m_pOwner = NULL;
	m_handle = INVALID_CLIENT_RENDER_HANDLE;

	// Initialize enhanced physics gun state
	m_effectState = EFFECT_NONE;
	m_bIsCurrentlyRotating = false;
	m_bIsCurrentlyHolding = false;
	m_serversidebeams = 0;

	m_vecHeldLocalOffset.Init();
	m_hOffsetEntity = NULL;
	m_vecBeamEnd.Init();
}

//-----------------------------------------------------------------------------
// Store the networked grab point in the held object's local space. Done once per
// network update; the per-frame path then just transforms it by the object's
// current (interpolated) matrix, so the beam end tracks the prop at framerate.
//-----------------------------------------------------------------------------
void C_BeamQuadratic::CacheHeldObjectOffset( void )
{
	C_BaseEntity *pHeld = m_heldObject.Get();
	if ( !pHeld )
	{
		m_hOffsetEntity = NULL;
		return;
	}

	pHeld->WorldToEntitySpace( m_worldPosition, &m_vecHeldLocalOffset );
	m_hOffsetEntity = pHeld;
}

//-----------------------------------------------------------------------------
// Per-frame beam end point.
//-----------------------------------------------------------------------------
const Vector &C_BeamQuadratic::ComputeBeamEndPoint( void )
{
	C_BaseEntity *pHeld = m_heldObject.Get();
	if ( pHeld && m_hOffsetEntity.Get() == pHeld )
	{
		pHeld->EntityToWorldSpace( m_vecHeldLocalOffset, &m_vecBeamEnd );
	}
	else
	{
		// Nothing held (or the offset is stale) - the networked point is the
		// player's aim point, which is already what we want.
		m_vecBeamEnd = m_worldPosition;
	}

	return m_vecBeamEnd;
}

void C_BeamQuadratic::Update( C_BaseEntity *pOwner )
{
	m_pOwner = pOwner;

	// Update effect state based on network variables
	UpdateEffectState();

	CacheHeldObjectOffset();
	ComputeBeamEndPoint();

	if ( m_active )
	{
		if ( m_handle == INVALID_CLIENT_RENDER_HANDLE )
		{
			m_handle = ClientLeafSystem()->AddRenderable( this, RENDER_GROUP_TRANSLUCENT_ENTITY );
		}
		else
		{
			ClientLeafSystem()->RenderableMoved( m_handle );
		}
	}
	else if ( !m_active && m_handle != INVALID_CLIENT_RENDER_HANDLE )
	{
		ClientLeafSystem()->RemoveRenderable( m_handle );
		m_handle = INVALID_CLIENT_RENDER_HANDLE;
	}
}


int	C_BeamQuadratic::DrawModel( int )
{
	if ( !m_active )
		return 0;

	// Retail Garry's Mod draws exactly one quadratic beam with sprites/physbeam.
	// No overlay pass, no second material.
	RenderPrimaryBeam();

	return 1;
}

void C_BeamQuadratic::UpdateEffectState( void )
{
	// Determine effect state based on network variables
	if ( !m_active )
	{
		m_effectState = EFFECT_NONE;
	}
	else if ( m_bIsCurrentlyHolding && m_heldObject.Get() )
	{
		m_effectState = EFFECT_HOLDING;
	}
	else if ( m_active && !m_heldObject.Get() )
	{
		m_effectState = EFFECT_READY;
	}
	else
	{
		m_effectState = EFFECT_NONE;
	}
}

void C_BeamQuadratic::RenderPrimaryBeam( void )
{
	Vector points[3];
	QAngle tmpAngle;

	C_BaseEntity *pEnt = cl_entitylist->GetEnt( m_viewModelIndex );
	if ( !pEnt )
		return;
	pEnt->GetAttachment( 1, points[0], tmpAngle );

	// End point is recomputed every frame from the held object's interpolated
	// transform, so the beam does not trail behind the prop.
	points[2] = ComputeBeamEndPoint();

	// Control point: midpoint between the muzzle and the *aim* point, exactly as
	// retail GMod does it. Deriving the bend from (world - target) instead made
	// the beam whip around whenever the prop lagged behind the cursor, which is
	// what read as input lag.
	points[1] = 0.5f * ( m_targetPosition + points[0] );

	// Retail GMod beam material.
	const char *materialName = "sprites/physbeam";

	// ConVar-driven beam color (GMod physgun color)
	float c_r = clamp( physgun_r.GetFloat() / 255.0f, 0.0f, 1.0f );
	float c_g = clamp( physgun_g.GetFloat() / 255.0f, 0.0f, 1.0f );
	float c_b = clamp( physgun_b.GetFloat() / 255.0f, 0.0f, 1.0f );
	Vector beamColor( c_r, c_g, c_b );

	// GMod's width is a flat 13 regardless of state.
	float beamWidth = 13.0f;

	if ( m_glueTouching )
	{
		beamColor.Init( 1, 0, 0 );
	}

	int subdivisions = 16;
	IMaterial *pMat = materials->FindMaterial( materialName, 0, 0 );

	CBeamSegDraw beamDraw;
	// The loop below emits subdivisions+1 segments - Start() must be told the
	// real count or the mesh builder overruns its reserved vertices.
	beamDraw.Start( subdivisions + 1, pMat );

	CBeamSeg seg;
	seg.m_flAlpha = 1.0;
	seg.m_flWidth = beamWidth;
	seg.m_vColor = beamColor;

	float t = 0;
	float u = gpGlobals->curtime - (int)gpGlobals->curtime;
	float dt = 1.0 / (float)subdivisions;
	for( int i = 0; i <= subdivisions; i++, t += dt )
	{
		float omt = (1-t);
		float p0 = omt*omt;
		float p1 = 2*t*omt;
		float p2 = t*t;

		seg.m_vPos = p0 * points[0] + p1 * points[1] + p2 * points[2];
		seg.m_flTexCoord = u - t;

		beamDraw.NextSeg( &seg );
	}

	beamDraw.End();
}

void C_BeamQuadratic::UpdateHeldObjectGlow( void )
{
	if ( !m_heldObject )
		return;

	C_BaseEntity *pHeldEntity = m_heldObject;
	if ( !pHeldEntity )
		return;

	// Only show glow when actively holding or in ready state
	if ( m_effectState != EFFECT_HOLDING && m_effectState != EFFECT_READY )
		return;

	// Get the model and check if it's valid
	const model_t *pModel = pHeldEntity->GetModel();
	if ( !pModel )
		return;

	// Select glow properties based on effect state
	const char *glowMaterialName = "sprites/glow01";
	float glowColor[4];
	float glowScale = 1.02f;
	float glowIntensity = 0.8f;

	switch ( m_effectState )
	{
		case EFFECT_READY:
		{
			// Blue glow for targeting
			glowColor[0] = 0.3f; glowColor[1] = 0.7f; glowColor[2] = 1.0f; glowColor[3] = 0.6f;
			glowScale = 1.015f; // Subtle glow
			glowMaterialName = "sprites/blueglow1";
			break;
		}

		case EFFECT_HOLDING:
		{
			// Orange glow for holding objects
			glowColor[0] = 1.0f; glowColor[1] = 0.6f; glowColor[2] = 0.2f; glowColor[3] = 0.9f;
			glowScale = 1.03f; // More prominent glow
			glowMaterialName = "sprites/orangelight1";

			// Add pulsing effect for held objects
			float pulseTime = gpGlobals->curtime * 2.0f;
			glowIntensity = 0.7f + 0.3f * sin( pulseTime );
			glowColor[3] *= glowIntensity;
			break;
		}

		default:
			return; // No glow for other states
	}

	// Create outline glow effect using the material system
	IMaterial *pGlowMaterial = materials->FindMaterial( glowMaterialName, 0, 0 );
	if ( !pGlowMaterial )
		pGlowMaterial = materials->FindMaterial( "sprites/glow01", 0, 0 ); // Fallback

	if ( !pGlowMaterial )
		return;

	// Set up rendering state for glow effect (2003 Material System)
	materials->Bind( pGlowMaterial );

	// Enable stencil testing to create silhouette effect (2003 version using BMod extensions)
	BMod::SetStencilEnable( materials, true );
	BMod::SetStencilFunc( materials, BMod::STENCILFUNC_ALWAYS );
	BMod::SetStencilPassOp( materials, BMod::STENCILOP_REPLACE );
	BMod::SetStencilFailOp( materials, BMod::STENCILOP_KEEP );
	BMod::SetStencilZFailOp( materials, BMod::STENCILOP_KEEP );
	BMod::SetStencilRef( materials, 1 );

	// First pass: Render object to stencil buffer (no color output)
	BMod::SetColorWritesEnabled( materials, false );

	// Get object's render origin and angles
	Vector origin = pHeldEntity->GetRenderOrigin();
	QAngle angles = pHeldEntity->GetRenderAngles();

	// Render the object slightly larger to create outline
	matrix3x4_t matrix;
	AngleMatrix( angles, origin, matrix );

	// Scale the matrix for outline effect
	matrix[0][0] *= glowScale;  // X scale
	matrix[1][1] *= glowScale;  // Y scale
	matrix[2][2] *= glowScale;  // Z scale

	materials->MatrixMode( MATERIAL_MODEL );
	// Convert matrix3x4_t to VMatrix using 2003 engine constructor
	VMatrix vmatrix( matrix );
	materials->LoadMatrix( vmatrix );

	// Draw the model scaled up - 2003 IVModelRender interface
	modelrender->DrawModel(
		STUDIO_RENDER,          // flags
		pHeldEntity,            // cliententity
		MODEL_INSTANCE_INVALID, // instance handle
		pHeldEntity->index,     // entity_index
		pModel,                 // model
		origin,                 // origin
		angles,                 // angles
		0,                      // sequence
		0,                      // skin
		0,                      // body
		0                       // hitboxset
	);

	// Second pass: Render glow where stencil is 0 (outside the object)
	BMod::SetColorWritesEnabled( materials, true );
	BMod::SetStencilFunc( materials, BMod::STENCILFUNC_NOTEQUAL );
	BMod::SetStencilRef( materials, 1 );

	// Set glow color - Color4fv not available in 2003 engine
	// Color would be set via material properties or vertex colors in 2003
	// For now, the glow effect will use the default material color

	// Reset matrix for normal rendering
	AngleMatrix( angles, origin, matrix );
	VMatrix normalMatrix( matrix );
	materials->LoadMatrix( normalMatrix );

	// Draw the model normally with glow material - 2003 IVModelRender interface
	modelrender->DrawModel(
		STUDIO_RENDER,          // flags
		pHeldEntity,            // cliententity
		MODEL_INSTANCE_INVALID, // instance handle
		pHeldEntity->index,     // entity_index
		pModel,                 // model
		origin,                 // origin
		angles,                 // angles
		0,                      // sequence
		0,                      // skin
		0,                      // body
		0                       // hitboxset
	);

	// Third pass: Add rotating indicator if in rotation mode
	if ( m_bIsCurrentlyRotating && m_effectState == EFFECT_HOLDING )
	{
		// Add subtle rotation indicator effect
		float rotationTime = gpGlobals->curtime * 4.0f;
		float rotationAlpha = 0.3f + 0.2f * sin( rotationTime );

		// Yellow tint for rotation indicator - Color4fv not available in 2003 engine
		// Color would be set via material properties or vertex colors in 2003
		// For now, the rotation indicator will use the default material color

		// Slightly larger scale for rotation indicator
		matrix[0][0] *= 1.01f;
		matrix[1][1] *= 1.01f;
		matrix[2][2] *= 1.01f;
		VMatrix rotationMatrix( matrix );
		materials->LoadMatrix( rotationMatrix );

		// Draw rotation indicator - 2003 IVModelRender interface
		modelrender->DrawModel(
			STUDIO_RENDER,          // flags
			pHeldEntity,            // cliententity
			MODEL_INSTANCE_INVALID, // instance handle
			pHeldEntity->index,     // entity_index
			pModel,                 // model
			origin,                 // origin
			angles,                 // angles
			0,                      // sequence
			0,                      // skin
			0,                      // body
			0                       // hitboxset
		);
	}

	// Cleanup: Disable stencil testing (2003 version using BMod extensions)
	BMod::SetStencilEnable( materials, false );
	materials->MatrixMode( MATERIAL_MODEL );
	materials->LoadIdentity();
}

/*
P0 = start
P1 = control
P2 = end
P(t) = (1-t)^2 * P0 + 2t(1-t)*P1 + t^2 * P2
*/

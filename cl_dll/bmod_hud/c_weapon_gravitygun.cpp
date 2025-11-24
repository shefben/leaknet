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

// External interfaces
extern IVModelRender *modelrender;

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
	void			RenderOverlayBeam( void );

	// IClientRenderable
	virtual const Vector&			GetRenderOrigin( void ) { return m_worldPosition; }
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
}

void C_BeamQuadratic::Update( C_BaseEntity *pOwner )
{
	m_pOwner = pOwner;

	// Update effect state based on network variables
	UpdateEffectState();

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

	// Render primary beam
	RenderPrimaryBeam();

	// Render overlay beam based on effect state
	if ( m_effectState == EFFECT_HOLDING || m_effectState == EFFECT_READY )
	{
		RenderOverlayBeam();
	}

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

	// Calculate beam path
	Vector beamDir = m_worldPosition - points[0];
	float beamLength = beamDir.Length();
	VectorNormalize( beamDir );

	// Add dynamic bending based on movement speed
	Vector velocity = m_worldPosition - m_targetPosition;
	float bendAmount = min( velocity.Length() * 0.01f, 8.0f ); // Max 8 units of bend
	Vector rightVector = CrossProduct( beamDir, Vector(0,0,1) );
	VectorNormalize( rightVector );

	points[1] = points[0] + beamDir * (beamLength * 0.5f) + rightVector * bendAmount;
	points[2] = m_worldPosition;

	// Select material and color based on effect state
	const char *materialName = "sprites/physbeam";
	Vector beamColor = Vector(1,1,1);
	float beamWidth = 13.0f;

	switch ( m_effectState )
	{
		case EFFECT_READY:
			beamColor = Vector(0.3f, 0.7f, 1.0f);  // Blue for ready state
			beamWidth = 11.0f;
			break;
		case EFFECT_HOLDING:
			beamColor = Vector(1.0f, 0.6f, 0.2f);  // Orange for holding
			beamWidth = 15.0f;
			break;
		case EFFECT_LAUNCH:
			beamColor = Vector(1.0f, 1.0f, 0.5f);  // Yellow for launch
			beamWidth = 18.0f;
			break;
		default:
			if ( m_glueTouching )
			{
				beamColor = Vector(1,0,0);  // Red when touching
			}
			break;
	}

	int subdivisions = 16;
	IMaterial *pMat = materials->FindMaterial( materialName, 0, 0 );

	CBeamSegDraw beamDraw;
	beamDraw.Start( subdivisions, pMat );

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

		if ( i == subdivisions )
		{
			// Fade out the end
			seg.m_vColor = beamColor * 0.1f;
		}
		beamDraw.NextSeg( &seg );
	}

	beamDraw.End();
}

void C_BeamQuadratic::RenderOverlayBeam( void )
{
	Vector points[3];
	QAngle tmpAngle;

	C_BaseEntity *pEnt = cl_entitylist->GetEnt( m_viewModelIndex );
	if ( !pEnt )
		return;
	pEnt->GetAttachment( 1, points[0], tmpAngle );

	// Calculate beam path (same as primary)
	Vector beamDir = m_worldPosition - points[0];
	float beamLength = beamDir.Length();
	VectorNormalize( beamDir );

	Vector velocity = m_worldPosition - m_targetPosition;
	float bendAmount = min( velocity.Length() * 0.01f, 8.0f );
	Vector rightVector = CrossProduct( beamDir, Vector(0,0,1) );
	VectorNormalize( rightVector );

	points[1] = points[0] + beamDir * (beamLength * 0.5f) + rightVector * bendAmount;
	points[2] = m_worldPosition;

	// Overlay beam properties - pulsing effect
	float pulseTime = gpGlobals->curtime * 3.0f;
	float pulseAlpha = 0.3f + 0.4f * sin( pulseTime );

	Vector overlayColor;
	float overlayWidth;

	if ( m_effectState == EFFECT_HOLDING )
	{
		overlayColor = Vector(1.0f, 0.8f, 0.4f);  // Bright orange overlay
		overlayWidth = 8.0f;
	}
	else // EFFECT_READY
	{
		overlayColor = Vector(0.5f, 0.9f, 1.0f);  // Bright blue overlay
		overlayWidth = 6.0f;
	}

	int subdivisions = 12; // Fewer subdivisions for overlay
	IMaterial *pMat = materials->FindMaterial( "sprites/physgbeam", 0, 0 ); // Different material for overlay

	CBeamSegDraw beamDraw;
	beamDraw.Start( subdivisions, pMat );

	CBeamSeg seg;
	seg.m_flAlpha = pulseAlpha;
	seg.m_flWidth = overlayWidth;
	seg.m_vColor = overlayColor;

	float t = 0;
	float u = gpGlobals->curtime * 2.0f - (int)(gpGlobals->curtime * 2.0f); // Faster scrolling for overlay
	float dt = 1.0 / (float)subdivisions;
	for( int i = 0; i <= subdivisions; i++, t += dt )
	{
		float omt = (1-t);
		float p0 = omt*omt;
		float p1 = 2*t*omt;
		float p2 = t*t;

		seg.m_vPos = p0 * points[0] + p1 * points[1] + p2 * points[2];
		seg.m_flTexCoord = u - t;

		// Alpha falloff toward the end
		if ( i > subdivisions * 0.8f )
		{
			float falloff = 1.0f - ((float)(i - subdivisions * 0.8f) / (subdivisions * 0.2f));
			seg.m_flAlpha = pulseAlpha * falloff;
		}

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

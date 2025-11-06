//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Physics cannon
//
//=============================================================================

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "soundenvelope.h"
#include "engine/IEngineSound.h"
#include "physics.h"
#include "in_buttons.h"
#include "soundent.h"
#include "IEffects.h"
#include "ndebugoverlay.h"
#include "shake.h"
#include "hl2_player.h"
#include "beam_shared.h"
#include "sprite.h"
#include "util.h"
#include "weapon_physcannon.h"
#include "physobj.h"
#include "physics_saverestore.h"
#include "mathlib/mathlib.h"
#include "tier1/strtools.h"

ConVar	g_debug_physcannon( "g_debug_physcannon", "0" );

#define	HOLDSOUND	"weapons/physcannon/hold_loop.wav"

// Enhanced sound definitions (matching Garry's Mod)
#define PHYSCANNON_PICKUP_SOUND		"weapons/physcannon/physcannon_pickup.wav"
#define PHYSCANNON_DROP_SOUND		"weapons/physcannon/physcannon_drop.wav"
#define PHYSCANNON_LAUNCH_SOUND		"weapons/physcannon/physcannon_launch.wav"
#define PHYSCANNON_CHARGE_SOUND		"weapons/physcannon/physcannon_charge.wav"
#define PHYSCANNON_READY_SOUND		"weapons/physcannon/physcannon_ready.wav"

// Physics Gun Effect States (matching client-side)
enum PhysGunEffectState_t
{
	EFFECT_NONE = 0,		// Inactive
	EFFECT_READY,			// Targeting valid object (blue theme)
	EFFECT_HOLDING,			// Holding object (orange theme)
	EFFECT_LAUNCH			// Launch animation
};

ConVar physcannon_minforce( "physcannon_minforce", "1000" );
ConVar physcannon_maxforce( "physcannon_maxforce", "1500" );
ConVar physcannon_maxmass( "physcannon_maxmass", "250" );
ConVar physcannon_tracelength( "physcannon_tracelength", "200" );
ConVar physcannon_chargetime("physcannon_chargetime", "2" );
ConVar physcannon_pullforce( "physcannon_pullforce", "4000" );

// BMOD: New console variables
ConVar bm_snapangles( "bm_snapangles", "15", 0, "Snap angle increment for physcannon rotation in degrees" );
ConVar bm_physcannon_distance( "bm_physcannon_distance", "80", 0, "Default distance for holding objects" );
ConVar bm_physcannon_mindist( "bm_physcannon_mindist", "24", 0, "Minimum distance for holding objects" );
ConVar bm_physcannon_maxdist( "bm_physcannon_maxdist", "300", 0, "Maximum distance for holding objects" );

extern ConVar hl2_normspeed;
extern ConVar hl2_walkspeed;

static void MatrixOrthogonalize( matrix3x4_t &matrix, int column )
{
	Vector columns[3];
	int i;

	for ( i = 0; i < 3; i++ )
	{
		MatrixGetColumn( matrix, i, columns[i] );
	}

	int index0 = column;
	int index1 = (column+1)%3;
	int index2 = (column+2)%3;

	columns[index2] = CrossProduct( columns[index0], columns[index1] );
	columns[index1] = CrossProduct( columns[index2], columns[index0] );
	VectorNormalize( columns[index2] );
	VectorNormalize( columns[index1] );
	MatrixSetColumn( columns[index1], index1, matrix );
	MatrixSetColumn( columns[index2], index2, matrix );
}

#define SIGN(x) ( (x) < 0 ? -1 : 1 )

static QAngle AlignAngles( const QAngle &angles, float cosineAlignAngle )
{
	matrix3x4_t alignMatrix;
	AngleMatrix( angles, alignMatrix );

	for ( int j = 0; j < 3; j++ )
	{
		Vector vec;
		MatrixGetColumn( alignMatrix, j, vec );
		for ( int i = 0; i < 3; i++ )
		{
			if ( fabs(vec[i]) > cosineAlignAngle )
			{
				vec[i] = SIGN(vec[i]);
				vec[(i+1)%3] = 0;
				vec[(i+2)%3] = 0;
				MatrixSetColumn( vec, j, alignMatrix );
				MatrixOrthogonalize( alignMatrix, j );
				break;
			}
		}
	}

	QAngle out;
	MatrixAngles( alignMatrix, out );
	return out;
}


static void TraceCollideAgainstBBox( const CPhysCollide *pCollide, const Vector &start, const Vector &end, const QAngle &angles, const Vector &boxOrigin, const Vector &mins, const Vector &maxs, trace_t *ptr )
{
	physcollision->TraceBox( boxOrigin, boxOrigin + (start-end), mins, maxs, pCollide, start, angles, ptr );

	if ( ptr->DidHit() )
	{
		ptr->endpos = start * (1-ptr->fraction) + end * ptr->fraction;
		ptr->startpos = start;
		ptr->plane.dist = -ptr->plane.dist;
		ptr->plane.normal *= -1;
	}
}

//-----------------------------------------------------------------------------

BEGIN_SIMPLE_DATADESC( CGrabController )

// Silence, classcheck!
//	DEFINE_EMBEDDED( CGrabController, m_shadow ),

	DEFINE_FIELD( CGrabController, m_shadow.targetPosition,		FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CGrabController, m_shadow.targetRotation,		FIELD_VECTOR ),
	DEFINE_FIELD( CGrabController, m_shadow.maxSpeed,			FIELD_VECTOR ),
	DEFINE_FIELD( CGrabController, m_shadow.maxAngular,			FIELD_VECTOR ),
	DEFINE_FIELD( CGrabController, m_shadow.dampFactor,			FIELD_FLOAT ),
	DEFINE_FIELD( CGrabController, m_shadow.teleportDistance,	FIELD_FLOAT ),

	DEFINE_FIELD( CGrabController, m_timeToArrive,		FIELD_FLOAT ),
	DEFINE_FIELD( CGrabController, m_errorTime,			FIELD_FLOAT ),
	DEFINE_FIELD( CGrabController, m_error,				FIELD_FLOAT ),
	DEFINE_FIELD( CGrabController, m_saveRotDamping,	FIELD_FLOAT ),
	DEFINE_FIELD( CGrabController, m_flLoadWeight,		FIELD_FLOAT ),
	DEFINE_FIELD( CGrabController, m_attachedEntity,	FIELD_EHANDLE ),

	// Physptrs can't be inside embedded classes
	// DEFINE_PHYSPTR( CGrabController, m_controller ),

END_DATADESC()


CGrabController::CGrabController( void )
{
	m_shadow.dampFactor = 1;
	m_shadow.teleportDistance = 0;
	m_errorTime = 0;
	m_error = 0;
	// make this controller really stiff!
	m_shadow.maxSpeed.Init( 1e4, 1e4, 1e4 );
	m_shadow.maxAngular.Init( 1e4, 1e4, 1e4 );

	m_attachedEntity = NULL;
}

void CGrabController::SetMaxImpulse( const Vector &linear, const AngularImpulse &angular )
{
	m_shadow.maxSpeed = linear;
	m_shadow.maxAngular = angular;
}

CGrabController::~CGrabController( void )
{
	DetachEntity();
}

void CGrabController::OnRestore()
{
	if ( m_controller )
	{
		m_controller->SetEventHandler( this );
	}
}

void CGrabController::SetTargetPosition( const Vector &target, const QAngle &targetOrientation )
{
	m_shadow.targetPosition = target;
	m_shadow.targetRotation = targetOrientation;
	
	m_timeToArrive = gpGlobals->frametime;

	CBaseEntity *pAttached = GetAttached();
	if ( pAttached )
	{
		IPhysicsObject *pObj = pAttached->VPhysicsGetObject();
		
		if ( pObj != NULL )
		{
			pObj->Wake();
		}
		else
		{
			DetachEntity();
		}
	}
}

float CGrabController::ComputeError()
{
	if ( m_errorTime <= 0 )
		return 0;

	CBaseEntity *pAttached = GetAttached();
	if ( pAttached )
	{
		Vector pos;
		IPhysicsObject *pObj = pAttached->VPhysicsGetObject();
		pObj->GetShadowPosition( &pos, NULL );
		float error = (m_shadow.targetPosition - pos).Length();
		m_errorTime = clamp(m_errorTime, 0, 1);
		m_error = (1-m_errorTime) * m_error + m_errorTime * error;
	}
	
	m_errorTime = 0;

	return m_error;
}


void CGrabController::AttachEntity( CBaseEntity *pEntity, IPhysicsObject *pPhys, const Vector &position, const QAngle &rotation )
{
	pPhys->GetDamping( NULL, &m_saveRotDamping );
	float damping = 10;
	pPhys->SetDamping( NULL, &damping );

	m_controller = physenv->CreateMotionController( this );
	m_controller->AttachObject( pPhys );
	m_controller->SetPriority( IPhysicsMotionController::HIGH_PRIORITY );

	pPhys->Wake();
	PhysSetGameFlags( pPhys, FVPHYSICS_PLAYER_HELD );
	SetTargetPosition( position, rotation );
	m_attachedEntity = pEntity;
	m_flLoadWeight = pPhys->GetMass();
}

void CGrabController::DetachEntity( void )
{
	CBaseEntity *pEntity = GetAttached();
	if ( pEntity )
	{
		IPhysicsObject *pPhys = pEntity->VPhysicsGetObject();
		if ( pPhys )
		{
			// on the odd chance that it's gone to sleep while under anti-gravity
			pPhys->Wake();
			pPhys->SetDamping( NULL, &m_saveRotDamping );
			PhysClearGameFlags( pPhys, FVPHYSICS_PLAYER_HELD );
		}
	}
	m_attachedEntity = NULL;
	physenv->DestroyMotionController( m_controller );
	m_controller = NULL;
}

IMotionEvent::simresult_e CGrabController::Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular )
{
	m_timeToArrive = pObject->ComputeShadowControl( m_shadow, m_timeToArrive, deltaTime );
	linear.Init();
	angular.Init();
	m_errorTime += deltaTime;
	return SIM_LOCAL_ACCELERATION;
}

// when looking level, hold bottom of object 8 inches below eye level
#define PLAYER_HOLD_LEVEL_EYES	-8

// when looking down, hold bottom of object 0 inches from feet
#define PLAYER_HOLD_DOWN_FEET	2

// when looking up, hold bottom of object 24 inches above eye level
#define PLAYER_HOLD_UP_EYES		24

// use a +/-30 degree range for the entire range of motion of pitch
#define PLAYER_LOOK_PITCH_RANGE	30

// player can reach down 2ft below his feet (otherwise he'll hold the object above the bottom)
#define PLAYER_REACH_DOWN_DISTANCE	24

class CPlayerPickupController : public CBaseEntity
{
	DECLARE_DATADESC();
	DECLARE_CLASS( CPlayerPickupController, CBaseEntity );
public:
	void Init( CBasePlayer *pPlayer, CBaseEntity *pObject );
	void Shutdown();
	bool OnControls( CBaseEntity *pControls ) { return true; }
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void ComputePlayerMatrix( matrix3x4_t &out, bool init );
	void CheckObjectPosition( Vector &position, const QAngle &angles, const Vector &oldPosition );
	void OnRestore()
	{
		m_grabController.OnRestore();
	}

private:
	CGrabController		m_grabController;
	CBasePlayer			*m_pPlayer;
	Vector				m_positionPlayerSpace;
	QAngle				m_anglesPlayerSpace;
};

LINK_ENTITY_TO_CLASS( player_pickup, CPlayerPickupController );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CPlayerPickupController )

	DEFINE_EMBEDDED( CPlayerPickupController, m_grabController ),

	// Physptrs can't be inside embedded classes
	DEFINE_PHYSPTR( CPlayerPickupController, m_grabController.m_controller ),

	DEFINE_FIELD( CPlayerPickupController, m_pPlayer,		FIELD_CLASSPTR ),
	DEFINE_FIELD( CPlayerPickupController, m_positionPlayerSpace, FIELD_VECTOR ),
	DEFINE_FIELD( CPlayerPickupController, m_anglesPlayerSpace, FIELD_VECTOR ),

END_DATADESC()


void CPlayerPickupController::Init( CBasePlayer *pPlayer, CBaseEntity *pObject )
{
	m_pPlayer = pPlayer;

	IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();
	Vector position;
	QAngle angles;
	pPhysics->GetPosition( &position, &angles );

	matrix3x4_t tmp;
	ComputePlayerMatrix( tmp, false /*true*/ );

	// If we've picked up a physbox, and it has a preferred orientation, use that instead.
	CPhysBox *pBox = dynamic_cast<CPhysBox*>(pObject);
	if ( pBox && pBox->HasPreferredCarryAngles() )
	{
		angles = TransformAnglesToWorldSpace( pBox->PreferredCarryAngles(), tmp );
	}

	m_grabController.AttachEntity( pObject, pPhysics, position, angles );
	float maxSpeed = pPhysics->GetInvMass() * 20 * 200;
	AngularImpulse maxAngular = pPhysics->GetInvInertia() * 20 * 360;
	m_grabController.SetMaxImpulse( maxSpeed * Vector(1,1,1), maxAngular );
	// Holster player's weapon
	if ( m_pPlayer->GetActiveWeapon() )
	{
		if ( !m_pPlayer->GetActiveWeapon()->Holster() )
		{
			Shutdown();
			return;
		}
	}

	m_pPlayer->m_Local.m_iHideHUD |= HIDEHUD_WEAPONS;
	m_pPlayer->SetUseEntity( this );
	VectorITransform( position, tmp, m_positionPlayerSpace );

	// UNDONE: This algorithm needs a bit more thought.  REVISIT.
	// put the bottom of the object arms' length below eye level
	// get bottommost point of object
	Vector bottom = physcollision->CollideGetExtent( pPhysics->GetCollide(), vec3_origin, angles, Vector(0,0,-1) );

	// get the real eye origin
	Vector playerEye = pPlayer->EyePosition();

	// move target up so that bottom of object is at PLAYER_HOLD_LEVEL z in local space
	float delta = PLAYER_HOLD_LEVEL_EYES - bottom.z - m_positionPlayerSpace.z;

	// player can reach down 2ft below his feet
	float maxPickup = (playerEye.z + PLAYER_HOLD_LEVEL_EYES) - (pPlayer->GetAbsMins().z - PLAYER_REACH_DOWN_DISTANCE);

	delta = clamp( delta, pPlayer->WorldAlignMins().z, maxPickup );
	m_positionPlayerSpace.z += delta;
	m_anglesPlayerSpace = TransformAnglesToLocalSpace( angles, tmp );

	m_anglesPlayerSpace = AlignAngles( m_anglesPlayerSpace, DOT_30DEGREE );
	
	// re-transform and check
	angles = TransformAnglesToWorldSpace( m_anglesPlayerSpace, tmp );
	VectorTransform( m_positionPlayerSpace, tmp, position );
	// hackhack: Move up to eye position for the check
	float saveZ = position.z;
	position.z = playerEye.z;
	CheckObjectPosition( position, angles, position );
	
	// move back to original position
	position.z = saveZ;

	VectorITransform( position, tmp, m_positionPlayerSpace );

	pObject->OnPhysGunPickup( pPlayer );
}

void CPlayerPickupController::Shutdown()
{
	CBaseEntity *pObject = m_grabController.GetAttached();
	if ( pObject != NULL )
	{
		pObject->OnPhysGunDrop( m_pPlayer, false );
	}

	m_pPlayer->SetUseEntity( NULL );
	m_grabController.DetachEntity();
	if ( m_pPlayer->GetActiveWeapon() )
	{
		m_pPlayer->GetActiveWeapon()->Deploy();
	}

	m_pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_WEAPONS;
	Remove();
}

void CPlayerPickupController::ComputePlayerMatrix( matrix3x4_t &out, bool init )
{
	QAngle angles = m_pPlayer->EyeAngles();
	Vector origin = m_pPlayer->EyePosition();
	
	// 0-360 / -180-180
	angles.x = init ? 0 : AngleDistance( angles.x, 0 );

	angles.x = clamp( angles.x, -PLAYER_LOOK_PITCH_RANGE, PLAYER_LOOK_PITCH_RANGE );

	float feet = m_pPlayer->GetAbsMins().z;
	float eyes = origin.z;
	float zoffset = 0;
	// moving up (negative pitch is up)
	if ( angles.x < 0 )
	{
		zoffset = RemapVal( angles.x, 0, -PLAYER_LOOK_PITCH_RANGE, PLAYER_HOLD_LEVEL_EYES, PLAYER_HOLD_UP_EYES );
	}
	else
	{
		zoffset = RemapVal( angles.x, 0, PLAYER_LOOK_PITCH_RANGE, PLAYER_HOLD_LEVEL_EYES, PLAYER_HOLD_DOWN_FEET + (feet - eyes) );
	}
	origin.z += zoffset;
	angles.x = 0;
	AngleMatrix( angles, origin, out );
}

void CPlayerPickupController::CheckObjectPosition( Vector &position, const QAngle &angles, const Vector &oldPosition )
{
	CBaseEntity *pAttached = m_grabController.GetAttached();
	trace_t tr;
	// move radially away from the player and check for space
	Vector offsetDir = position - m_pPlayer->GetAbsOrigin();
	offsetDir.z = 0;
	VectorNormalize(offsetDir);
	Vector startSweep = position + offsetDir * pAttached->EntitySpaceSize().Length();

	TraceCollideAgainstBBox( pAttached->VPhysicsGetObject()->GetCollide(), startSweep, position, angles, 
			m_pPlayer->GetAbsOrigin(), m_pPlayer->WorldAlignMins(), m_pPlayer->WorldAlignMaxs(), &tr );

	if ( tr.fraction != 1.0 )
	{
		// if you hit, back off 4 inches and set that as the target
		// otherwise, you made it all the way and the position is fine
		if ( tr.startsolid )
		{
			position = oldPosition;
		}
		else
		{
			position = tr.endpos + offsetDir * 2;
		}
	}
}


void CPlayerPickupController::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( ToBasePlayer(pActivator) == m_pPlayer )
	{
		CBaseEntity *pAttached = m_grabController.GetAttached();

		if ( !pAttached || useType == USE_OFF || m_grabController.ComputeError() > 12 || (m_pPlayer->m_nButtons & (IN_ATTACK|IN_ATTACK2)) )
		{
			Shutdown();
			return;
		}
		else if ( useType == USE_SET )
		{
			// update position
			matrix3x4_t tmp;
			ComputePlayerMatrix( tmp, false );
			Vector position;
			QAngle angles;
			VectorTransform( m_positionPlayerSpace, tmp, position );
			angles = TransformAnglesToWorldSpace( m_anglesPlayerSpace, tmp );
			CheckObjectPosition( position, angles, pAttached->GetAbsOrigin() );

			m_grabController.SetTargetPosition( position, angles );
		}
	}
}


void PlayerPickupObject( CBasePlayer *pPlayer, CBaseEntity *pObject )
{
	CPlayerPickupController *pController = (CPlayerPickupController *)CBaseEntity::Create( "player_pickup", pObject->GetAbsOrigin(), vec3_angle, pPlayer );
	if ( !pController )
		return;

	pController->Init( pPlayer, pObject );
}

struct launchtable_t
{
	IPhysicsObject *pLaunch;
	float			launchTime;
	CHandle<CBasePlayer> hPlayer;
};
static launchtable_t g_RememberLaunch;


CBasePlayer *WasLaunchedByPlayer( IPhysicsObject *pPhysicsObject, float currentTime, float dt )
{
	if ( pPhysicsObject ==  g_RememberLaunch.pLaunch )
	{
		CBasePlayer *pPlayer = g_RememberLaunch.hPlayer;
		float delta = currentTime - g_RememberLaunch.launchTime;
		if ( delta >= 0 && delta < dt )
			return pPlayer;
	}

	return NULL;
}

static void RememberLaunch( CBasePlayer *pPlayer, IPhysicsObject *pPhysicsObject, float launchTime )
{
	g_RememberLaunch.hPlayer = pPlayer;
	g_RememberLaunch.pLaunch = pPhysicsObject;
	g_RememberLaunch.launchTime = launchTime;
}

static void ClearLaunch()
{
	memset( &g_RememberLaunch, 0, sizeof(g_RememberLaunch) );
}


//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST(CWeaponPhysCannon, DT_WeaponPhysCannon)
	SendPropVector( SENDINFO(m_targetPosition), -1, SPROP_COORD ),
	SendPropVector( SENDINFO(m_worldPosition), -1, SPROP_COORD ),
	SendPropInt( SENDINFO(m_active) ),
	SendPropInt( SENDINFO(m_glueTouching) ),
	SendPropInt( SENDINFO(m_viewModelIndex) ),
	SendPropEHandle( SENDINFO(m_heldObject) ),
	// Enhanced physics gun state (matching GMod)
	SendPropInt( SENDINFO(m_effectState), 3, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO(m_bIsCurrentlyRotating) ),
	SendPropBool( SENDINFO(m_bIsCurrentlyHolding) ),
	SendPropInt( SENDINFO(m_serversidebeams), 1, SPROP_UNSIGNED ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_physcannon, CWeaponPhysCannon );
PRECACHE_WEAPON_REGISTER( weapon_physcannon );

BEGIN_DATADESC( CWeaponPhysCannon )

	DEFINE_FIELD( CWeaponPhysCannon, m_bOpen, FIELD_BOOLEAN ),
	DEFINE_FIELD( CWeaponPhysCannon, m_bActive, FIELD_BOOLEAN ),

	DEFINE_FIELD( CWeaponPhysCannon, m_nChangeState, FIELD_INTEGER ),
	DEFINE_FIELD( CWeaponPhysCannon, m_flCheckSuppressTime, FIELD_TIME ),
	DEFINE_FIELD( CWeaponPhysCannon, m_flElementDebounce, FIELD_TIME ),
	DEFINE_FIELD( CWeaponPhysCannon, m_flElementPosition, FIELD_FLOAT ),
	DEFINE_FIELD( CWeaponPhysCannon, m_flElementDestination, FIELD_FLOAT ),

	DEFINE_FIELD( CWeaponPhysCannon, m_flObjectRadius, FIELD_FLOAT ),
	DEFINE_AUTO_ARRAY( CWeaponPhysCannon, m_pBeams, FIELD_CLASSPTR ),
	DEFINE_AUTO_ARRAY( CWeaponPhysCannon, m_pGlowSprites, FIELD_CLASSPTR ),
	DEFINE_AUTO_ARRAY( CWeaponPhysCannon, m_pEndSprites, FIELD_CLASSPTR ),
	DEFINE_FIELD( CWeaponPhysCannon, m_pCenterSprite, FIELD_CLASSPTR ),
	DEFINE_FIELD( CWeaponPhysCannon, m_pBlastSprite, FIELD_CLASSPTR ),
	DEFINE_SOUNDPATCH( CWeaponPhysCannon, m_sndMotor ),

	// BUGBUG: SAVE THIS PLEASE!
	//DEFINE_FIELD( CWeaponPhysCannon, m_grabController, FIELD_EMBEDDED ),
	DEFINE_FIELD( CWeaponPhysCannon, m_attachedAnglesPlayerSpace, FIELD_VECTOR ),
	DEFINE_FIELD( CWeaponPhysCannon, m_attachedPositionObjectSpace, FIELD_VECTOR ),

END_DATADESC()


enum
{
	ELEMENT_STATE_NONE = -1,
	ELEMENT_STATE_OPEN,
	ELEMENT_STATE_CLOSED,
};

enum
{
	EFFECT_NONE,
	EFFECT_CLOSED,
	EFFECT_READY,
	EFFECT_HOLDING,
	EFFECT_LAUNCH,
};

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponPhysCannon::CWeaponPhysCannon( void )
{
	m_flElementPosition		= 0.0f;
	m_flElementDestination	= 0.0f;
	m_bOpen					= false;
	m_nChangeState			= ELEMENT_STATE_NONE;
	m_flCheckSuppressTime	= 0.0f;
	ClearLaunch();

	// BMOD: Initialize new member variables
	m_flHoldDistance		= bm_physcannon_distance.GetFloat();
	m_flLastRKeyTime		= 0.0f;
	m_angRotationOffset.Init();
	m_bRotationMode			= false;
	m_vecLastMousePos.Init();
	m_frozenObjects.Purge();

	// Initialize network variables
	m_targetPosition.Init();
	m_worldPosition.Init();
	m_active = 0;
	m_glueTouching = 0;
	m_viewModelIndex = 0;
	m_heldObject = NULL;

	// Initialize enhanced physics gun state
	m_effectState = EFFECT_NONE;
	m_bIsCurrentlyRotating = false;
	m_bIsCurrentlyHolding = false;
	m_serversidebeams = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::Precache( void )
{
	enginesound->PrecacheSound( HOLDSOUND );

	// Enhanced sound precaching (matching Garry's Mod)
	enginesound->PrecacheSound( PHYSCANNON_PICKUP_SOUND );
	enginesound->PrecacheSound( PHYSCANNON_DROP_SOUND );
	enginesound->PrecacheSound( PHYSCANNON_LAUNCH_SOUND );
	enginesound->PrecacheSound( PHYSCANNON_CHARGE_SOUND );
	enginesound->PrecacheSound( PHYSCANNON_READY_SOUND );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Restore
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::OnRestore()
{
	BaseClass::OnRestore();
	if ( m_grabController.m_controller )
	{
		m_grabController.m_controller->SetEventHandler( &m_grabController );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::Deploy( void )
{
	CloseElements();
	DoEffect( EFFECT_CLOSED );

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: Force the cannon to drop anything it's carrying
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::ForceDrop( void )
{
	CloseElements();
	DetachObject();
	StopEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::Drop( const Vector &vecVelocity )
{
	CloseElements();
	DetachObject();
	StopEffects();

	BaseClass::Drop( vecVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	//Don't holster this weapon if we're holding onto something
	if ( m_bActive )
		return false;

	CloseElements();
	DetachObject();
	StopEffects();

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DryFire( void )
{
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	WeaponSound( EMPTY );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::PrimaryFireEffect( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	pOwner->ViewPunch( QAngle(-6, random->RandomInt(-2,2) ,0) );
	
	color32 white = { 245, 245, 255, 32 };
	UTIL_ScreenFade( pOwner, white, 0.1f, 0.0f, FFADE_IN );

	WeaponSound( SINGLE );
}

#define	MAX_KNOCKBACK_FORCE	128

//-----------------------------------------------------------------------------
// Purpose: 
//
// This mode is a toggle. Primary fire one time to pick up a physics object.
// With an object held, click primary fire again to drop object.
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::PrimaryAttack( void )
{
	if( m_flNextPrimaryAttack > gpGlobals->curtime )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	if( m_bActive )
	{
		// Punch the object being held!!
		Vector forward;

		pOwner->EyeVectors( &forward );
		LaunchObject( forward, physcannon_maxforce.GetFloat() );

		PrimaryFireEffect();
		SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	}
	else
	{
		// If not active, just issue a physics punch in the world.
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

		Vector forward;
		pOwner->EyeVectors( &forward );

		Vector start = pOwner->Weapon_ShootPosition();
		Vector end = start + forward * physcannon_tracelength.GetFloat();

		trace_t tr;
		UTIL_TraceHull( start, end, -Vector(8,8,8), Vector(8,8,8), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr );

		//See if we hit something
		if ( tr.fraction != 1 && tr.m_pEnt )
		{
			CBaseEntity *pEntity = tr.m_pEnt;

			if( pEntity->GetMoveType() != MOVETYPE_VPHYSICS )
			{
				if ( pEntity->m_takedamage != DAMAGE_NO )
				{
					PrimaryFireEffect();
					SendWeaponAnim( ACT_VM_SECONDARYATTACK );

					CTakeDamageInfo	info;

					info.SetAttacker( GetOwner() );
					info.SetInflictor( this );
					info.SetDamage( 15.0f );
					info.SetDamageType( DMG_CRUSH | DMG_PHYSGUN );

					pEntity->DispatchTraceAttack( info, forward, &tr );

					ApplyMultiDamage();
					
					OpenElements();
					DoEffect( EFFECT_HOLDING );

					//Explosion effect
					DoEffect( EFFECT_LAUNCH, &tr.endpos );

					PrimaryFireEffect();
					SendWeaponAnim( ACT_VM_SECONDARYATTACK );

					m_nChangeState = ELEMENT_STATE_CLOSED;
					m_flElementDebounce = gpGlobals->curtime + 0.5f;
					m_flCheckSuppressTime = gpGlobals->curtime + 0.25f;

					return;
				}

				//Play dry-fire sequence
				DryFire();
			}
			else
			{
				CTakeDamageInfo	info;

				info.SetAttacker( GetOwner() );
				info.SetInflictor( this );
				info.SetDamage( 0.0f );
				info.SetDamageType( DMG_PHYSGUN );
				pEntity->DispatchTraceAttack( info, forward, &tr );
				ApplyMultiDamage();

				IPhysicsObject *pPhysicsObject = pEntity->VPhysicsGetObject();

				if ( pPhysicsObject == NULL )
				{
					DryFire();
					return;
				}
						
				if( forward.z < 0 )
				{
					//reflect, but flatten the trajectory out a bit so it's easier to hit standing targets
					forward.z *= -0.65f;
				}

				//Must be light enough
				pPhysicsObject->ApplyForceCenter( forward * 15000 );
				
				// Put some spin on the object
				pPhysicsObject->ApplyForceOffset( forward * pPhysicsObject->GetMass() * 600, tr.endpos );

				//Knock the player backwards depending on the mass
				float	knockPerc = ( pPhysicsObject->GetMass() / physcannon_maxmass.GetFloat() );
				float	knockForce = MAX_KNOCKBACK_FORCE * knockPerc;

				if ( knockPerc > 2.0f )
					knockPerc = 2.0f;

				if ( knockForce > 400.0f )
					knockForce = 400.0f;

				if ( knockForce < 0.0f )
					knockForce = 0.0f;

				Vector	knockDir = ( tr.endpos - tr.startpos );
				VectorNormalize( knockDir );

				knockDir *= -knockForce;
				knockDir[2] += 8;

				pOwner->ApplyAbsVelocityImpulse( knockDir );
				pOwner->RemoveFlag( FL_ONGROUND );

				QAngle	recoil = QAngle( -4*knockPerc, random->RandomFloat( -1.0f, 1.0f ), 0 );
				pOwner->ViewPunch( recoil );

				OpenElements();
				DoEffect( EFFECT_HOLDING );

				//Explosion effect
				DoEffect( EFFECT_LAUNCH, &tr.endpos );

				PrimaryFireEffect();
				SendWeaponAnim( ACT_VM_SECONDARYATTACK );

				m_nChangeState = ELEMENT_STATE_CLOSED;
				m_flElementDebounce = gpGlobals->curtime + 0.5f;
				m_flCheckSuppressTime = gpGlobals->curtime + 0.25f;

				return;
			}
		}

		//Play dry-fire sequence
		DryFire();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Click secondary attack whilst holding an object to hurl it.
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::SecondaryAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	// BMOD: Right-click now freezes objects instead of dropping/picking up
	if( m_bActive )
	{
		// Freeze the held object in place
		CBaseEntity *pObject = m_grabController.GetAttached();
		if ( pObject )
		{
			FreezeObject( pObject );
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			DetachObject();
			WeaponSound( SPECIAL2 );
		}
	}
	else
	{
		// Try to freeze object we're aiming at
		Vector forward;
		pOwner->EyeVectors( &forward );
		Vector start = pOwner->Weapon_ShootPosition();
		Vector end = start + forward * physcannon_tracelength.GetFloat();

		trace_t tr;
		UTIL_TraceLine( start, end, MASK_SHOT|CONTENTS_GRATE, pOwner, COLLISION_GROUP_NONE, &tr );

		if ( tr.DidHit() && tr.m_pEnt && CanPickupObject( tr.m_pEnt ) )
		{
			if ( IsObjectFrozen( tr.m_pEnt ) )
			{
				// Object is already frozen, unfreeze it
				UnfreezeObject( tr.m_pEnt );
				WeaponSound( MELEE_HIT );
			}
			else
			{
				// Freeze the object
				FreezeObject( tr.m_pEnt );
				WeaponSound( SPECIAL2 );
			}
		}
		else
		{
			WeaponSound( EMPTY );
		}

		m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	}
}	

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::WeaponIdle( void )
{
	if ( HasWeaponIdleTimeElapsed() )
	{
		if ( m_bActive )
		{
			//Shake when holding an item
			SendWeaponAnim( ACT_VM_RELOAD );
		}
		else
		{
			//Otherwise idle simply
			SendWeaponAnim( ACT_VM_IDLE );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pObject - 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::AttachObject( CBaseEntity *pObject, const Vector &attachPosition )
{
	if ( m_bActive )
		return;

	if ( CanPickupObject( pObject ) == false )
		return;

	IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();

	Vector position;
	QAngle angles;
	pPhysics->GetPosition( &position, &angles );
	m_grabController.AttachEntity( pObject, pPhysics, position, angles );
	m_bActive = true;
	m_attachedAnglesPlayerSpace = TransformAnglesToLocalSpace( angles, GetOwner()->EntityToWorldTransform() );
	VectorITransform( attachPosition, pObject->EntityToWorldTransform(), m_attachedPositionObjectSpace );

	// BMOD: Reset rotation offset when picking up new object
	m_angRotationOffset.Init();
	m_bRotationMode = false;

	CHL2_Player *pOwner = (CHL2_Player *)ToBasePlayer( GetOwner() );

	if( pOwner )
	{
		pOwner->StopSprinting();

		float	loadWeight = ( 1.0f - GetLoadPercentage() );
		float	maxSpeed = hl2_walkspeed.GetFloat() + ( ( hl2_normspeed.GetFloat() - hl2_walkspeed.GetFloat() ) * loadWeight );

		//Msg( "Load perc: %f -- Movement speed: %f/%f\n", loadWeight, maxSpeed, hl2_normspeed.GetFloat() );

		pOwner->SetMaxSpeed( maxSpeed );
		pObject->OnPhysGunPickup( pOwner );
	}

	//Find the largest extent of the object's bounding box
	m_flObjectRadius = 0.0f;

	for ( int i = 0; i < 3; i++ )
	{
		if ( pObject->EntitySpaceMaxs()[i] > m_flObjectRadius )
		{
			m_flObjectRadius = pObject->EntitySpaceMaxs()[i];
		}

		//Just in case
		if ( -pObject->EntitySpaceMins()[i] > m_flObjectRadius )
		{
			m_flObjectRadius = -pObject->EntitySpaceMins()[i];
		}
	}

	m_flObjectRadius *= 0.5f;

	DoEffect( EFFECT_HOLDING );

	// Enhanced state management and sound (matching Garry's Mod)
	m_effectState = EFFECT_HOLDING;
	m_bIsCurrentlyHolding = true;
	m_bIsCurrentlyRotating = false;

	// Play pickup sound
	EmitSound( PHYSCANNON_PICKUP_SOUND );

	//Disable collision between the player and the object
	physenv->DisableCollisions( pPhysics, pOwner->VPhysicsGetObject() );
	pPhysics->RecheckCollisionFilter();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::FindObject( void )
{
	Vector forward;
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	Assert( pPlayer );
	pPlayer->EyeVectors( &forward );

	Vector	start = pPlayer->Weapon_ShootPosition();
	float	testLength = physcannon_tracelength.GetFloat() * 4.0f;
	Vector	end = start + forward * testLength;

	trace_t tr;
	UTIL_TraceHull( start, end, -Vector(4,4,4), Vector(4,4,4), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction != 1 && tr.m_pEnt )
	{
		CBaseEntity *pEntity = tr.m_pEnt;

		if ( tr.fraction <= 0.25f )
		{
			AttachObject( pEntity, tr.endpos );
		}
		else
		{
			IPhysicsObject	*pObj = pEntity->VPhysicsGetObject();

			if ( pObj != NULL )
			{
				//If we're too far, simply start to pull the object towards us
				Vector	pullDir = pPlayer->Weapon_ShootPosition() - pEntity->WorldSpaceCenter();
				VectorNormalize( pullDir );

				//Nudge it towards us
				pObj->ApplyForceCenter( pullDir * physcannon_pullforce.GetFloat() );

				//FIXME: Need some other sort of "pulling" effect
				DoEffect( EFFECT_READY );
			}
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::UpdateObject( void )
{
	CBaseEntity *pEntity = m_grabController.GetAttached();
	if ( !pEntity )
	{
		DetachObject();
		return;
	}

	Vector forward, right, up;
	CHL2_Player *pPlayer = (CHL2_Player *) ToBasePlayer( GetOwner() );
	Assert( pPlayer );
	pPlayer->EyeVectors( &forward, &right, &up );

	// BMOD: Use adjustable hold distance
	float	distance = m_flHoldDistance;

	Vector	start = pPlayer->Weapon_ShootPosition() + right * 3.0f + forward * 4.0f + up * -2.0f;

	Vector	end = start + ( forward * distance );

	trace_t	tr;
	UTIL_TraceLine( start, end, MASK_SOLID_BRUSHONLY, pPlayer, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction < 0.5 )
	{
		end = start + forward * (m_flObjectRadius*0.5f);
	}
	else if ( tr.fraction < 1.0f )
	{
		end = start + forward * ( distance - m_flObjectRadius );
	}

	//Show overlays of radius
	if ( g_debug_physcannon.GetBool() )
	{
		NDebugOverlay::Box( m_grabController.GetAttached()->WorldSpaceCenter(), 
							-Vector( m_flObjectRadius, m_flObjectRadius, m_flObjectRadius), 
							Vector( m_flObjectRadius, m_flObjectRadius, m_flObjectRadius ),
							255, 0, 0,
							true,
							0.05f );
	}

	// BMOD: Apply rotation offset to angles
	QAngle modifiedAngles = m_attachedAnglesPlayerSpace;
	modifiedAngles += m_angRotationOffset;

	QAngle angles = TransformAnglesToWorldSpace( modifiedAngles, GetOwner()->EntityToWorldTransform() );

	matrix3x4_t attachedToWorld;
	Vector offset;
	AngleMatrix( angles, attachedToWorld );
	VectorRotate( m_attachedPositionObjectSpace, attachedToWorld, offset );
	m_grabController.SetTargetPosition( end - offset, angles );

	// BMOD: Update network variables for client-side beam and glow rendering
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		m_targetPosition = end - offset;
		m_worldPosition = pEntity->WorldSpaceCenter();
		m_active = 1;
		m_glueTouching = 0; // Could be used for visual feedback when near surfaces
		m_viewModelIndex = pOwner->GetViewModel() ? pOwner->GetViewModel()->entindex() : 0;
		m_heldObject = pEntity;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DetachObject( bool playSound, bool wasLaunched )
{
	if ( m_bActive == false )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if( pOwner != NULL )
	{
		pOwner->SetMaxSpeed( hl2_normspeed.GetFloat() );
	}

	CBaseEntity *pObject = m_grabController.GetAttached();
	if ( pObject != NULL )
	{
		pObject->OnPhysGunDrop( pOwner, wasLaunched );

		IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();

		//Enable collision with this object again
		if ( pPhysics != NULL )
		{
			physenv->EnableCollisions( pPhysics, pOwner->VPhysicsGetObject() );
			pPhysics->RecheckCollisionFilter();

		}
	}

	m_bActive = false;
	m_grabController.DetachEntity();

	// BMOD: Clear network variables when no object is held
	m_targetPosition.Init();
	m_worldPosition.Init();
	m_active = 0;
	m_glueTouching = 0;
	m_viewModelIndex = 0;
	m_heldObject = NULL;

	// Enhanced state management (matching Garry's Mod)
	m_effectState = EFFECT_NONE;
	m_bIsCurrentlyHolding = false;
	m_bIsCurrentlyRotating = false;

	if ( playSound )
	{
		// Enhanced sound system
		if ( wasLaunched )
		{
			// Play launch sound
			EmitSound( PHYSCANNON_LAUNCH_SOUND );
		}
		else
		{
			// Play drop sound
			EmitSound( PHYSCANNON_DROP_SOUND );
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::ItemPreFrame()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	m_flElementPosition = UTIL_Approach( m_flElementDestination, m_flElementPosition, 0.1f );

	CBaseViewModel *vm = pOwner->GetViewModel();
	
	if ( vm != NULL )
	{
		vm->SetPoseParameter( "active", m_flElementPosition );
	}

	// Update the object if the weapon is switched on.
	if( m_bActive )
	{
		UpdateObject();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::CheckForTarget( void )
{
	//See if we're suppressing this
	if ( m_flCheckSuppressTime > gpGlobals->curtime )
		return;

	// holstered
	if ( m_fEffects & EF_NODRAW )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if ( m_bActive == false )
	{
		Vector	aimDir;
		pOwner->EyeVectors( &aimDir );

		Vector	startPos	= pOwner->Weapon_ShootPosition();
		Vector	endPos		= startPos + ( aimDir * physcannon_tracelength.GetFloat() );

		trace_t	tr;
		UTIL_TraceHull( startPos, endPos, -Vector(4,4,4), Vector(4,4,4), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr );

		if ( ( tr.fraction != 1.0f ) && ( tr.m_pEnt != NULL ) )
		{
			//FIXME: Try just having the elements always open when pointed at a physics object
			//if ( CanPickupObject( tr.m_pEnt ) )
			if ( tr.m_pEnt->VPhysicsGetObject() != NULL )
			{
				m_nChangeState = ELEMENT_STATE_NONE;

				// Enhanced state management (matching Garry's Mod)
				if ( CanPickupObject( tr.m_pEnt ) )
				{
					m_effectState = EFFECT_READY;
				}

				OpenElements();
				return;
			}
		}

		//Close the elements after a delay to prevent overact state switching
		if ( ( m_flElementDebounce < gpGlobals->curtime ) && ( m_nChangeState == ELEMENT_STATE_NONE ) )
		{
			m_nChangeState = ELEMENT_STATE_CLOSED;
			m_flElementDebounce = gpGlobals->curtime + 0.5f;

			// Enhanced state management - No target
			if ( m_effectState == EFFECT_READY )
			{
				m_effectState = EFFECT_NONE;
			}
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::ItemPostFrame()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	//Check for object in pickup range
	if ( m_bActive == false )
	{
		CheckForTarget();

		if ( ( m_flElementDebounce < gpGlobals->curtime ) && ( m_nChangeState != ELEMENT_STATE_NONE ) )
		{
			if ( m_nChangeState == ELEMENT_STATE_OPEN )
			{
				OpenElements();
			}
			else if ( m_nChangeState == ELEMENT_STATE_CLOSED )
			{
				CloseElements();
			}

			m_nChangeState = ELEMENT_STATE_NONE;
		}
	}

	// BMOD: Handle R key for unfreezing objects
	if ( pOwner->m_afButtonPressed & IN_RELOAD )
	{
		// Check for double-tap R within 0.5 seconds
		if ( gpGlobals->curtime - m_flLastRKeyTime < 0.5f )
		{
			// Double-tap: unfreeze all objects
			UnfreezeAllPlayerObjects();
		}
		else
		{
			// Single tap: try to unfreeze object we're aiming at
			Vector forward;
			pOwner->EyeVectors( &forward );
			Vector start = pOwner->Weapon_ShootPosition();
			Vector end = start + forward * physcannon_tracelength.GetFloat();

			trace_t tr;
			UTIL_TraceLine( start, end, MASK_SHOT|CONTENTS_GRATE, pOwner, COLLISION_GROUP_NONE, &tr );

			if ( tr.DidHit() && tr.m_pEnt && IsObjectFrozen( tr.m_pEnt ) )
			{
				UnfreezeObject( tr.m_pEnt );
				WeaponSound( MELEE_HIT );
			}
		}
		m_flLastRKeyTime = gpGlobals->curtime;
	}

	// BMOD: Handle distance adjustment and rotation
	if ( m_bActive )
	{
		HandleDistanceAdjustment();
		HandleRotation();
	}

	if ( pOwner->m_afButtonPressed & IN_ATTACK2 )
	{
		SecondaryAttack();
	}
	else if ( pOwner->m_nButtons & IN_ATTACK )
	{
		PrimaryAttack();
	}
	else
	{
		WeaponIdle();
	}

	//Make the end points flicker as fast as possible
	//FIXME: Make this a property of the CSprite class!
	for ( int i = 0; i < 2; i++ )
	{
		m_pEndSprites[i]->SetBrightness( random->RandomInt( 200, 255 ) );
		m_pEndSprites[i]->SetScale( random->RandomFloat( 0.15, 0.2 ) );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::LaunchObject( const Vector &vecDir, float flForce )
{
	// FIRE!!!
	if( m_grabController.GetAttached() )
	{
		CBaseEntity *pObject = m_grabController.GetAttached();

		DetachObject( false, true );

		// Trace ahead a bit and make a chain of danger sounds ahead of the phys object
		// to scare potential targets
		trace_t	tr;
		Vector	vecStart = pObject->GetAbsOrigin();
		Vector	vecSpot;
		int		iLength;
		int		i;

		UTIL_TraceLine( vecStart, vecStart + vecDir * flForce, MASK_SHOT, pObject, COLLISION_GROUP_NONE, &tr );

		iLength = ( tr.startpos - tr.endpos ).Length();
	
		vecSpot = vecStart + vecDir * 128;

		for( i = 128 ; i < iLength ; i += 128 )
		{
			CSoundEnt::InsertSound( SOUND_PHYSICS_DANGER, vecSpot, 128, 0.5, pObject );
			vecSpot = vecSpot + ( vecDir * 128 );
		}
		
		IPhysicsObject *pPhysObject = pObject->VPhysicsGetObject();
		
		//pPhysObject->ApplyForceCenter( vecDir * ( flForce * pPhysObject->GetMass() ) );
		
		Vector			vVel = vecDir * flForce;
		AngularImpulse	aVel = RandomAngularImpulse( -600, 600 );

		pPhysObject->AddVelocity( &vVel, &aVel );
		RememberLaunch( ToBasePlayer(GetOwner()), pPhysObject, gpGlobals->curtime );

		// Don't allow the gun to regrab a thrown object!!
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
		
		Vector	center = pObject->WorldSpaceCenter();

		//Do repulse effect
		DoEffect( EFFECT_LAUNCH, &center );
	}

	//Close the elements and suppress checking for a bit
	m_nChangeState = ELEMENT_STATE_CLOSED;
	m_flElementDebounce = gpGlobals->curtime + 0.1f;
	m_flCheckSuppressTime = gpGlobals->curtime + 0.25f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTarget - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::CanPickupObject( CBaseEntity *pTarget )
{
	return CBasePlayer::CanPickupObject( pTarget, physcannon_maxmass.GetFloat(), 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::OpenElements( void )
{
	if ( m_bOpen )
		return;

	WeaponSound( SPECIAL2 );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if ( m_flElementPosition < 0.0f )
		m_flElementPosition = 0.0f;

	m_flElementDestination = 1.0f;

	SendWeaponAnim( ACT_VM_IDLE );

	m_bOpen = true;

	if ( GetMotorSound() )
	{
		(CSoundEnvelopeController::GetController()).Play( GetMotorSound(), 0.0f, 50 );
		(CSoundEnvelopeController::GetController()).SoundChangePitch( GetMotorSound(), 100, 0.5f );
		(CSoundEnvelopeController::GetController()).SoundChangeVolume( GetMotorSound(), 1.0f, 0.5f );
	}

	DoEffect( EFFECT_READY );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::CloseElements( void )
{
	if ( m_bOpen == false )
		return;

	WeaponSound( MELEE_HIT );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if ( m_flElementPosition > 1.0f )
		m_flElementPosition = 1.0f;

	m_flElementDestination = 0.0f;

	SendWeaponAnim( ACT_VM_IDLE );

	m_bOpen = false;

	if ( GetMotorSound() )
	{
		(CSoundEnvelopeController::GetController()).SoundChangeVolume( GetMotorSound(), 0.1f, 1.0f );
		(CSoundEnvelopeController::GetController()).SoundChangePitch( GetMotorSound(), 50, 1.0f );
	}
	
	DoEffect( EFFECT_CLOSED );
}

#define	PHYSCANNON_MAX_MASS		500

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CWeaponPhysCannon::GetLoadPercentage( void )
{
	float loadWeight = m_grabController.GetLoadWeight();

	return ( loadWeight / physcannon_maxmass.GetFloat() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CSoundPatch
//-----------------------------------------------------------------------------
CSoundPatch *CWeaponPhysCannon::GetMotorSound( void )
{
	if ( m_sndMotor == NULL )
	{
		CPASAttenuationFilter filter( this );
		m_sndMotor = (CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), CHAN_STATIC, HOLDSOUND, ATTN_NORM );
	}

	return m_sndMotor;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::StopEffects( void )
{
	//Turn off main glow
	if ( m_pCenterSprite != NULL )
	{
		m_pCenterSprite->TurnOff();
	}

	if ( m_pBlastSprite != NULL )
	{
		m_pBlastSprite->TurnOff();
	}

	//Turn off beams
	for ( int i = 0; i < NUM_BEAMS; i++ )
	{
		if ( m_pBeams[i] != NULL )
		{
			m_pBeams[i]->SetBrightness( 0 );
		}
	}

	//Turn off sprites
	for ( int i = 0; i < NUM_SPRITES; i++ )
	{
		if ( m_pGlowSprites[i] != NULL )
		{
			m_pGlowSprites[i]->TurnOff();
		}
	}

	for ( int i = 0; i < 2; i++ )
	{
		if ( m_pEndSprites[i] != NULL )
		{
			m_pEndSprites[i]->TurnOff();
		}
	}

	//Shut off sounds
	if ( GetMotorSound() != NULL )
	{
		(CSoundEnvelopeController::GetController()).SoundFadeOut( GetMotorSound(), 0.1f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::StartEffects( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	CBaseEntity *pBeamEnt = pOwner->GetViewModel();
	
	// Create the beams
	for ( int i = 0; i < NUM_BEAMS; i++ )
	{
		if ( m_pBeams[i] == NULL )
		{
			const char *beamAttachNames[] = 
			{
				"fork1t",
				"fork2t",
				"fork1t",
				"fork2t",
				"fork1t",
				"fork2t",
			};

			m_pBeams[i] = CBeam::BeamCreate( PHYSCANNON_BEAM_SPRITE, 1.0f );
			m_pBeams[i]->EntsInit( pBeamEnt, pBeamEnt );

			int	startAttachment = LookupAttachment( beamAttachNames[i] );
			int endAttachment	= 1;

			m_pBeams[i]->SetStartAttachment( startAttachment );
			m_pBeams[i]->SetEndAttachment( endAttachment );
			m_pBeams[i]->SetNoise( 8.0f );
			m_pBeams[i]->SetColor( 255, 255, 255 );
			m_pBeams[i]->SetScrollRate( 25 );
			m_pBeams[i]->SetBrightness( 128 );
			m_pBeams[i]->SetWidth( (1+(i*4)) * 0.1 );
			m_pBeams[i]->SetEndWidth( (1+(i*32)) * 0.1 );
		}
	}

	//Create the glow sprites
	for ( int i = 0; i < NUM_SPRITES; i++ )
	{
		if ( m_pGlowSprites[i] == NULL )
		{
			const char *attachNames[] = 
			{
				"fork1b",
				"fork1m",
				"fork1t",
				"fork2b",
				"fork2m",
				"fork2t"
			};

			m_pGlowSprites[i] = CSprite::SpriteCreate( "sprites/glow04_noz.vmt", GetAbsOrigin(), false );

			m_pGlowSprites[i]->SetAttachment( pOwner->GetViewModel(), LookupAttachment( attachNames[i] ) );
			m_pGlowSprites[i]->SetTransparency( kRenderGlow, 255, 128, 0, 64, kRenderFxNoDissipation );
			m_pGlowSprites[i]->SetBrightness( 255, 0.2f );
			m_pGlowSprites[i]->SetScale( 0.25f, 0.2f );
		}
	}

	//Create the endcap sprites
	for ( int i = 0; i < 2; i++ )
	{
		if ( m_pEndSprites[i] == NULL )
		{
			const char *attachNames[] = 
			{
				"fork1t",
				"fork2t"
			};

			m_pEndSprites[i] = CSprite::SpriteCreate( "sprites/orangeflare1.vmt", GetAbsOrigin(), false );

			m_pEndSprites[i]->SetAttachment( pOwner->GetViewModel(), LookupAttachment( attachNames[i] ) );
			m_pEndSprites[i]->SetTransparency( kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation );
			m_pEndSprites[i]->SetBrightness( 255, 0.2f );
			m_pEndSprites[i]->SetScale( 0.25f, 0.2f );
			m_pEndSprites[i]->TurnOff();
		}
	}

	//Create the center glow
	if ( m_pCenterSprite == NULL )
	{
		m_pCenterSprite = CSprite::SpriteCreate( "sprites/orangecore1.vmt", GetAbsOrigin(), false );

		m_pCenterSprite->SetAttachment( pOwner->GetViewModel(), 1 );
		m_pCenterSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone );
		m_pCenterSprite->SetBrightness( 255, 0.2f );
		m_pCenterSprite->SetScale( 0.1f, 0.2f );
	}
	
	//Create the blast sprite
	if ( m_pBlastSprite == NULL )
	{
		m_pBlastSprite = CSprite::SpriteCreate( "sprites/orangecore2.vmt", GetAbsOrigin(), false );

		m_pBlastSprite->SetAttachment( pOwner->GetViewModel(), 1 );
		m_pBlastSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone );
		m_pBlastSprite->SetBrightness( 255, 0.2f );
		m_pBlastSprite->SetScale( 0.1f, 0.2f );
		m_pBlastSprite->TurnOff();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : effectType - 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DoEffect( int effectType, Vector *pos )
{
	StartEffects();

	switch( effectType )
	{
	case EFFECT_CLOSED:
		{
			//Turn off the center sprite
			if ( m_pCenterSprite != NULL )
			{
				m_pCenterSprite->SetBrightness( 0.0, 0.1f );
				m_pCenterSprite->SetScale( 0.0f, 0.1f );
				m_pCenterSprite->TurnOff();
			}

			//Turn off the end-caps
			for ( int i = 0; i < 2; i++ )
			{
				if ( m_pEndSprites[i] != NULL )
				{
					m_pEndSprites[i]->TurnOff();
				}
			}

			//Turn off the lightning
			for ( int i = 0; i < NUM_BEAMS; i++ )
			{
				if ( m_pBeams[i] != NULL )
				{
					m_pBeams[i]->SetBrightness( 0 );
				}
			}

			//Turn on the glow sprites
			for ( int i = 0; i < NUM_SPRITES; i++ )
			{
				if ( m_pGlowSprites[i] != NULL )
				{
					m_pGlowSprites[i]->TurnOn();
					m_pGlowSprites[i]->SetBrightness( 16.0f, 0.2f );
					m_pGlowSprites[i]->SetScale( 0.3f, 0.2f );
				}
			}
			
			//Prepare for scale down
			if ( m_pBlastSprite != NULL )
			{
				m_pBlastSprite->TurnOn();
				m_pBlastSprite->SetScale( 1.0f, 0.0f );
				m_pBlastSprite->SetBrightness( 0, 0.0f );
			}

		}
		break;

	case EFFECT_READY:
		{
			//Turn off the center sprite
			if ( m_pCenterSprite != NULL )
			{
				m_pCenterSprite->SetBrightness( 128, 0.2f );
				m_pCenterSprite->SetScale( 0.15f, 0.2f );
				m_pCenterSprite->TurnOn();
			}

			//Turn off the end-caps
			for ( int i = 0; i < 2; i++ )
			{
				if ( m_pEndSprites[i] != NULL )
				{
					m_pEndSprites[i]->TurnOff();
				}
			}

			//Turn off the lightning
			for ( int i = 0; i < NUM_BEAMS; i++ )
			{
				if ( m_pBeams[i] != NULL )
				{
					m_pBeams[i]->SetBrightness( 0 );
				}
			}

			//Turn on the glow sprites
			for ( int i = 0; i < NUM_SPRITES; i++ )
			{
				if ( m_pGlowSprites[i] != NULL )
				{
					m_pGlowSprites[i]->TurnOn();
					m_pGlowSprites[i]->SetBrightness( 32.0f, 0.2f );
					m_pGlowSprites[i]->SetScale( 0.4f, 0.2f );
				}
			}

			//Scale down
			if ( m_pBlastSprite != NULL )
			{
				m_pBlastSprite->TurnOn();
				m_pBlastSprite->SetScale( 0.1f, 0.2f );
				m_pBlastSprite->SetBrightness( 255, 0.1f );
			}
		}
		break;

	case EFFECT_HOLDING:
		{
			//Turn off the center sprite
			if ( m_pCenterSprite != NULL )
			{
				m_pCenterSprite->SetBrightness( 255, 0.1f );
				m_pCenterSprite->SetScale( 0.2f, 0.2f );
				m_pCenterSprite->TurnOn();
			}

			//Turn off the end-caps
			for ( int i = 0; i < 2; i++ )
			{
				if ( m_pEndSprites[i] != NULL )
				{
					m_pEndSprites[i]->TurnOn();
				}
			}

			//Turn off the lightning
			for ( int i = 0; i < NUM_BEAMS; i++ )
			{
				if ( m_pBeams[i] != NULL )
				{
					m_pBeams[i]->SetBrightness( 128 );
				}
			}

			//Turn on the glow sprites
			for ( int i = 0; i < NUM_SPRITES; i++ )
			{
				if ( m_pGlowSprites[i] != NULL )
				{
					m_pGlowSprites[i]->TurnOn();
					m_pGlowSprites[i]->SetBrightness( 64.0f, 0.2f );
					m_pGlowSprites[i]->SetScale( 0.5f, 0.2f );
				}
			}

			//Prepare for scale up
			if ( m_pBlastSprite != NULL )
			{
				m_pBlastSprite->TurnOff();
				m_pBlastSprite->SetScale( 0.1f, 0.0f );
				m_pBlastSprite->SetBrightness( 0, 0.0f );
			}
		}
		break;

	case EFFECT_LAUNCH:
		{
			assert(pos!=NULL);
			if ( pos == NULL )
				return;

			CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
			
			if ( pOwner == NULL )
				return;

			Vector	endpos = *pos;

			// Check to store off our view model index
			CBaseViewModel *vm = pOwner->GetViewModel();
			CBeam *pBeam = CBeam::BeamCreate( "sprites/orangelight1.vmt", 0.8 );

			if ( pBeam != NULL )
			{
				pBeam->PointEntInit( endpos, vm );
				pBeam->SetEndAttachment( 1 );
				pBeam->SetWidth( 6.4 );
				pBeam->SetEndWidth( 12.8 );					
				pBeam->SetBrightness( 255 );
				pBeam->SetColor( 255, 255, 255 );
				pBeam->LiveForTime( 0.1f );
				pBeam->RelinkBeam();
				pBeam->SetNoise( 2 );
			}

			Vector	shotDir = ( endpos - pOwner->Weapon_ShootPosition() );
			VectorNormalize( shotDir );

			//End hit
			//FIXME: Probably too big
			CPVSFilter filter( endpos );
			te->GaussExplosion( filter, 0.0f, endpos - ( shotDir * 4.0f ), RandomVector(-1.0f, 1.0f), 0 );
			
			
			m_pBlastSprite->TurnOn();
			m_pBlastSprite->SetScale( 2.0f, 0.1f );
			m_pBlastSprite->SetBrightness( 0.0f, 0.1f );
		}
		break;

	default:
	case EFFECT_NONE:
		break;

	}
}

//=============================================================================
// BMOD: New BarrysMod Physics Gun Features
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Freeze an object in place
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::FreezeObject( CBaseEntity *pObject )
{
	if ( !pObject )
		return;

	IPhysicsObject *pPhys = pObject->VPhysicsGetObject();
	if ( !pPhys )
		return;

	// Add to our frozen objects list
	EHANDLE hObject;
	hObject = pObject;

	// Check if already frozen
	for ( int i = 0; i < m_frozenObjects.Count(); i++ )
	{
		if ( m_frozenObjects[i].Get() == pObject )
			return; // Already frozen
	}

	m_frozenObjects.AddToTail( hObject );

	// Make the object motionless and disable physics
	pPhys->EnableMotion( false );
	pPhys->SetVelocity( &vec3_origin, &vec3_origin );

	// Visual effect for frozen objects
	DispatchParticleEffect( "freeze_effect", pObject->GetAbsOrigin(), pObject->GetAbsAngles() );
}

//-----------------------------------------------------------------------------
// Purpose: Unfreeze an object
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::UnfreezeObject( CBaseEntity *pObject )
{
	if ( !pObject )
		return;

	IPhysicsObject *pPhys = pObject->VPhysicsGetObject();
	if ( !pPhys )
		return;

	// Remove from frozen objects list
	for ( int i = 0; i < m_frozenObjects.Count(); i++ )
	{
		if ( m_frozenObjects[i].Get() == pObject )
		{
			m_frozenObjects.Remove( i );
			break;
		}
	}

	// Re-enable physics
	pPhys->EnableMotion( true );
	pPhys->Wake();

	// Visual effect for unfrozen objects
	DispatchParticleEffect( "unfreeze_effect", pObject->GetAbsOrigin(), pObject->GetAbsAngles() );
}

//-----------------------------------------------------------------------------
// Purpose: Unfreeze all objects frozen by this player
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::UnfreezeAllPlayerObjects( void )
{
	for ( int i = m_frozenObjects.Count() - 1; i >= 0; i-- )
	{
		CBaseEntity *pObject = m_frozenObjects[i].Get();
		if ( pObject )
		{
			UnfreezeObject( pObject );
		}
		else
		{
			// Remove invalid handles
			m_frozenObjects.Remove( i );
		}
	}

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "All frozen objects unfrozen!" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Check if an object is frozen
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::IsObjectFrozen( CBaseEntity *pObject )
{
	if ( !pObject )
		return false;

	for ( int i = 0; i < m_frozenObjects.Count(); i++ )
	{
		if ( m_frozenObjects[i].Get() == pObject )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Handle mouse wheel and E+W/S distance adjustment
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::HandleDistanceAdjustment( void )
{
	if ( !m_bActive )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	float distanceChange = 0.0f;

	// Mouse wheel adjustment (from client-side input)
	if ( pOwner->m_nButtons & IN_WEAPON1 ) // Mouse wheel up
	{
		distanceChange = 5.0f;
	}
	else if ( pOwner->m_nButtons & IN_WEAPON2 ) // Mouse wheel down
	{
		distanceChange = -5.0f;
	}

	// E + W/S adjustment
	if ( pOwner->m_nButtons & IN_USE ) // E key held
	{
		if ( pOwner->m_nButtons & IN_FORWARD ) // W key
		{
			distanceChange = 3.0f;
		}
		else if ( pOwner->m_nButtons & IN_BACK ) // S key
		{
			distanceChange = -3.0f;
		}
	}

	if ( distanceChange != 0.0f )
	{
		m_flHoldDistance += distanceChange;
		m_flHoldDistance = clamp( m_flHoldDistance, bm_physcannon_mindist.GetFloat(), bm_physcannon_maxdist.GetFloat() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle E+Mouse rotation
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::HandleRotation( void )
{
	if ( !m_bActive )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	// Check if E is held for rotation mode
	bool bUseHeld = (pOwner->m_nButtons & IN_USE) != 0;
	bool bShiftHeld = (pOwner->m_nButtons & IN_SPEED) != 0;

	if ( bUseHeld && !m_bRotationMode )
	{
		// Entering rotation mode
		m_bRotationMode = true;
		m_bIsCurrentlyRotating = true; // Enhanced: Update network variable
		// Store current view angles as reference
		QAngle currentViewAngles = pOwner->EyeAngles();
		m_vecLastMousePos.x = currentViewAngles.y;
		m_vecLastMousePos.y = currentViewAngles.x;
	}
	else if ( !bUseHeld && m_bRotationMode )
	{
		// Exiting rotation mode
		m_bRotationMode = false;
		m_bIsCurrentlyRotating = false; // Enhanced: Update network variable
	}

	if ( m_bRotationMode )
	{
		// Get view angle movement
		QAngle currentViewAngles = pOwner->EyeAngles();
		float deltaX = AngleDiff( currentViewAngles.y, m_vecLastMousePos.x );
		float deltaY = AngleDiff( currentViewAngles.x, m_vecLastMousePos.y );

		if ( bShiftHeld )
		{
			// Snap angle rotation
			float snapAngle = bm_snapangles.GetFloat();
			if ( fabs(deltaX) > 5.0f ) // Threshold for snap
			{
				float rotationAmount = (deltaX > 0) ? snapAngle : -snapAngle;
				m_angRotationOffset.y += rotationAmount;
				m_vecLastMousePos.x = currentViewAngles.y; // Reset reference
			}
			if ( fabs(deltaY) > 5.0f )
			{
				float rotationAmount = (deltaY > 0) ? snapAngle : -snapAngle;
				m_angRotationOffset.x += rotationAmount;
				m_vecLastMousePos.y = currentViewAngles.x; // Reset reference
			}
		}
		else
		{
			// Smooth rotation - only apply if change is above threshold to prevent jitter
			if ( fabs(deltaX) > 0.5f )
			{
				m_angRotationOffset.y += deltaX * 2.0f;
				m_vecLastMousePos.x = currentViewAngles.y;
			}
			if ( fabs(deltaY) > 0.5f )
			{
				m_angRotationOffset.x += deltaY * 2.0f;
				m_vecLastMousePos.y = currentViewAngles.x;
			}

			// Keep angles in reasonable range
			m_angRotationOffset.x = AngleNormalize( m_angRotationOffset.x );
			m_angRotationOffset.y = AngleNormalize( m_angRotationOffset.y );
		}
	}
}

//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Balloon System Implementation
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "gmod_balloon.h"
#include "physics.h"
#include "rope_shared.h"
#include "rope.h"
#include "tier1/strtools.h"

// Initialize static members
bool CGModBalloonSystem::m_bInitialized = false;

// Console variables for balloon system
ConVar gm_balloon_reverse("gm_balloon_reverse", "0", FCVAR_NONE, "Will make balloons pull down");
ConVar gm_balloon_power("gm_balloon_power", "1.0", FCVAR_NONE, "Power of new balloons");
ConVar gm_balloon_rope_width("gm_balloon_rope_width", "2.0", FCVAR_NONE, "Width of balloon ropes");
ConVar gm_balloon_rope_length("gm_balloon_rope_length", "100.0", FCVAR_NONE, "Length of balloon ropes");
ConVar gm_balloon_rope_forcelimit("gm_balloon_rope_forcelimit", "1000.0", FCVAR_NONE, "Force limit for balloon ropes");
ConVar gm_balloon_rope_rigid("gm_balloon_rope_rigid", "0", FCVAR_NONE, "Make balloon ropes rigid");
ConVar gm_balloon_rope_type("gm_balloon_rope_type", "0", FCVAR_NONE, "Type of balloon rope");
ConVar gm_balloon_explode("gm_balloon_explode", "1", FCVAR_NONE, "Balloons explode when destroyed");

//-----------------------------------------------------------------------------
// Entity linking
//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(gmod_balloon, CGModBalloon);

//-----------------------------------------------------------------------------
// Network table
//-----------------------------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST(CGModBalloon, DT_GMod_Balloon)
    SendPropFloat(SENDINFO(m_flBalloonPower), 0, SPROP_NOSCALE),
    SendPropBool(SENDINFO(m_bBalloonReverse)),
    SendPropBool(SENDINFO(m_bExplodeOnDamage)),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Data description
//-----------------------------------------------------------------------------
BEGIN_DATADESC(CGModBalloon)
    DEFINE_FIELD(CGModBalloon, m_flBalloonPower, FIELD_FLOAT),
    DEFINE_FIELD(CGModBalloon, m_bBalloonReverse, FIELD_BOOLEAN),
    DEFINE_FIELD(CGModBalloon, m_bExplodeOnDamage, FIELD_BOOLEAN),
    DEFINE_FIELD(CGModBalloon, m_hRopeEntity, FIELD_EHANDLE),
    DEFINE_FIELD(CGModBalloon, m_flRopeWidth, FIELD_FLOAT),
    DEFINE_FIELD(CGModBalloon, m_flRopeLength, FIELD_FLOAT),
    DEFINE_FIELD(CGModBalloon, m_flRopeForceLimit, FIELD_FLOAT),
    DEFINE_FIELD(CGModBalloon, m_bRopeRigid, FIELD_BOOLEAN),
    DEFINE_FIELD(CGModBalloon, m_iRopeType, FIELD_INTEGER),
    DEFINE_FIELD(CGModBalloon, m_flLiftThink, FIELD_TIME),
    DEFINE_FIELD(CGModBalloon, m_vecLiftForce, FIELD_VECTOR),

    // Inputs
    DEFINE_INPUTFUNC(FIELD_FLOAT, "SetPower", InputSetPower),
    DEFINE_INPUTFUNC(FIELD_BOOLEAN, "SetReverse", InputSetReverse),
    DEFINE_INPUTFUNC(FIELD_VOID, "Explode", InputExplode),

    // Think function
    DEFINE_THINKFUNC(Think),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGModBalloon::CGModBalloon()
{
    m_flBalloonPower = 1.0f;
    m_bBalloonReverse = false;
    m_bExplodeOnDamage = true;
    m_hRopeEntity = NULL;

    // Initialize rope settings from ConVars
    m_flRopeWidth = 2.0f;
    m_flRopeLength = 100.0f;
    m_flRopeForceLimit = 1000.0f;
    m_bRopeRigid = false;
    m_iRopeType = 0;

    m_flLiftThink = 0.0f;
    m_vecLiftForce.Init();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CGModBalloon::~CGModBalloon()
{
    DestroyRope();
}

//-----------------------------------------------------------------------------
// Purpose: Precache balloon resources
//-----------------------------------------------------------------------------
void CGModBalloon::Precache()
{
    PrecacheModel("models/maxofs2d/balloon_classic.mdl");
    PrecacheSound("weapons/physcannon/energy_bounce1.wav");
    PrecacheSound("weapons/physcannon/energy_bounce2.wav");

    BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the balloon
//-----------------------------------------------------------------------------
void CGModBalloon::Spawn()
{
    Precache();

    SetupBalloonModel();
    SetupBalloonPhysics();

    // Get settings from console variables
    m_flBalloonPower = gm_balloon_power.GetFloat();
    m_bBalloonReverse = gm_balloon_reverse.GetBool();
    m_bExplodeOnDamage = gm_balloon_explode.GetBool();

    m_flRopeWidth = gm_balloon_rope_width.GetFloat();
    m_flRopeLength = gm_balloon_rope_length.GetFloat();
    m_flRopeForceLimit = gm_balloon_rope_forcelimit.GetFloat();
    m_bRopeRigid = gm_balloon_rope_rigid.GetBool();
    m_iRopeType = gm_balloon_rope_type.GetInt();

    // Start thinking for lift force
    SetThink(&CGModBalloon::Think);
    SetNextThink(gpGlobals->curtime + 0.1f);

    BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Set up balloon model
//-----------------------------------------------------------------------------
void CGModBalloon::SetupBalloonModel()
{
    SetModel("models/maxofs2d/balloon_classic.mdl");
    SetMoveType(MOVETYPE_VPHYSICS);
    SetSolid(SOLID_VPHYSICS);
}

//-----------------------------------------------------------------------------
// Purpose: Set up balloon physics
//-----------------------------------------------------------------------------
void CGModBalloon::SetupBalloonPhysics()
{
    VPhysicsInitNormal(SOLID_VPHYSICS, 0, false);

    IPhysicsObject* pPhysics = VPhysicsGetObject();
    if (pPhysics)
    {
        // Make balloon light and bouncy
        pPhysics->SetMass(1.0f);
        pPhysics->SetDamping(0.8f, 0.8f);
        pPhysics->EnableDrag(false);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Think function - apply lift force
//-----------------------------------------------------------------------------
void CGModBalloon::Think()
{
    if (CanApplyLift())
    {
        ApplyLiftForce();
    }

    SetNextThink(gpGlobals->curtime + 0.1f);
}

//-----------------------------------------------------------------------------
// Purpose: Check if balloon can apply lift
//-----------------------------------------------------------------------------
bool CGModBalloon::CanApplyLift()
{
    if (!VPhysicsGetObject())
        return false;

    if (m_flBalloonPower <= 0.0f)
        return false;

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Apply lift force to balloon
//-----------------------------------------------------------------------------
void CGModBalloon::ApplyLiftForce()
{
    IPhysicsObject* pPhysics = VPhysicsGetObject();
    if (!pPhysics)
        return;

    // Calculate lift force
    Vector liftForce = Vector(0, 0, 1) * m_flBalloonPower * 600.0f; // Base lift force

    // Reverse if enabled
    if (m_bBalloonReverse)
    {
        liftForce = -liftForce;
    }

    // Apply some damping based on velocity
    Vector velocity;
    pPhysics->GetVelocity(&velocity, NULL);

    // Reduce lift force based on upward velocity to prevent infinite acceleration
    if (!m_bBalloonReverse && velocity.z > 100.0f)
    {
        float dampening = 1.0f - (velocity.z - 100.0f) / 200.0f;
        dampening = clamp(dampening, 0.1f, 1.0f);
        liftForce *= dampening;
    }
    else if (m_bBalloonReverse && velocity.z < -100.0f)
    {
        float dampening = 1.0f - (-velocity.z - 100.0f) / 200.0f;
        dampening = clamp(dampening, 0.1f, 1.0f);
        liftForce *= dampening;
    }

    pPhysics->ApplyForceCenter(liftForce);
}

//-----------------------------------------------------------------------------
// Purpose: Create rope to target entity
//-----------------------------------------------------------------------------
void CGModBalloon::CreateRope(CBaseEntity* pTarget)
{
    if (!pTarget || HasRope())
        return;

    // Create rope entity
    CRopeKeyframe* pRope = CRopeKeyframe::Create(
        this,
        pTarget,
        0, // attachment point on balloon
        0, // attachment point on target
        (int)m_flRopeLength,
        m_flRopeWidth,
        "cable/cable.vmt",
        5 // num segments
    );

    if (pRope)
    {
        m_hRopeEntity = pRope;

        // Configure rope properties
        if (m_bRopeRigid)
        {
            pRope->EnableWind(false);
        }

        DevMsg("Balloon rope created\n");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Destroy rope
//-----------------------------------------------------------------------------
void CGModBalloon::DestroyRope()
{
    if (m_hRopeEntity.Get())
    {
        UTIL_Remove(m_hRopeEntity);
        m_hRopeEntity = NULL;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Set balloon power
//-----------------------------------------------------------------------------
void CGModBalloon::SetBalloonPower(float power)
{
    m_flBalloonPower = max(0.0f, power);
}

//-----------------------------------------------------------------------------
// Purpose: Set balloon reverse
//-----------------------------------------------------------------------------
void CGModBalloon::SetBalloonReverse(bool reverse)
{
    m_bBalloonReverse = reverse;
}

//-----------------------------------------------------------------------------
// Purpose: Set explode on damage
//-----------------------------------------------------------------------------
void CGModBalloon::SetExplodeOnDamage(bool explode)
{
    m_bExplodeOnDamage = explode;
}

//-----------------------------------------------------------------------------
// Purpose: Handle physics gun pickup
//-----------------------------------------------------------------------------
void CGModBalloon::OnPhysGunPickup(CBasePlayer* pPhysGunUser)
{
    BaseClass::OnPhysGunPickup(pPhysGunUser);

    // Play pickup sound
    EmitSound("weapons/physcannon/energy_bounce1.wav");
}

//-----------------------------------------------------------------------------
// Purpose: Handle physics gun drop
//-----------------------------------------------------------------------------
void CGModBalloon::OnPhysGunDrop(CBasePlayer* pPhysGunUser, bool wasLaunched)
{
    BaseClass::OnPhysGunDrop(pPhysGunUser, wasLaunched);

    // Play drop sound
    EmitSound("weapons/physcannon/energy_bounce2.wav");
}

//-----------------------------------------------------------------------------
// Input handlers
//-----------------------------------------------------------------------------
void CGModBalloon::InputSetPower(inputdata_t &inputdata)
{
    SetBalloonPower(inputdata.value.Float());
}

void CGModBalloon::InputSetReverse(inputdata_t &inputdata)
{
    SetBalloonReverse(inputdata.value.Bool());
}

void CGModBalloon::InputExplode(inputdata_t &inputdata)
{
    if (m_bExplodeOnDamage)
    {
        // Create explosion effect
        Vector origin = GetAbsOrigin();
        ExplosionCreate(origin, QAngle(0,0,0), this, 50, 100, false, 0.0f, true);

        // Remove balloon
        UTIL_Remove(this);
    }
}

//-----------------------------------------------------------------------------
// Balloon System Management
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Initialize balloon system
//-----------------------------------------------------------------------------
void CGModBalloonSystem::Initialize()
{
    if (m_bInitialized)
        return;

    m_bInitialized = true;
    Msg("Balloon System initialized\n");
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown balloon system
//-----------------------------------------------------------------------------
void CGModBalloonSystem::Shutdown()
{
    m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Create a balloon at position
//-----------------------------------------------------------------------------
CGModBalloon* CGModBalloonSystem::CreateBalloon(const Vector& position, CBasePlayer* pOwner)
{
    if (!m_bInitialized)
        return NULL;

    CGModBalloon* pBalloon = (CGModBalloon*)CreateEntityByName("gmod_balloon");
    if (pBalloon)
    {
        pBalloon->SetAbsOrigin(position);
        pBalloon->Spawn();

        if (pOwner)
        {
            pBalloon->SetOwnerEntity(pOwner);
        }

        return pBalloon;
    }

    return NULL;
}

//-----------------------------------------------------------------------------
// System initialization hook
//-----------------------------------------------------------------------------
class CBalloonSystemInit : public CAutoGameSystem
{
public:
    CBalloonSystemInit() : CAutoGameSystem("BalloonSystemInit") {}

    virtual bool Init()
    {
        CGModBalloonSystem::Initialize();
        return true;
    }

    virtual void Shutdown()
    {
        CGModBalloonSystem::Shutdown();
    }
};

static CBalloonSystemInit g_BalloonSystemInit;
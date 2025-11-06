//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Balloon System - GMod 9.0.4b compatible implementation
//
// $NoKeywords: $
//=============================================================================

#ifndef GMOD_BALLOON_H
#define GMOD_BALLOON_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "props.h"

//-----------------------------------------------------------------------------
// Balloon Entity - Provides lift force to attached objects
//-----------------------------------------------------------------------------
class CGModBalloon : public CPhysicsProp
{
    DECLARE_CLASS(CGModBalloon, CPhysicsProp);
    DECLARE_SERVERCLASS();
    DECLARE_DATADESC();

public:
    CGModBalloon();
    ~CGModBalloon();

    // Entity management
    virtual void Spawn();
    virtual void Precache();
    virtual void OnPhysGunPickup(CBasePlayer* pPhysGunUser);
    virtual void OnPhysGunDrop(CBasePlayer* pPhysGunUser, bool wasLaunched);

    // Balloon behavior
    virtual void Think();
    void ApplyLiftForce();

    // Rope management
    void CreateRope(CBaseEntity* pTarget);
    void DestroyRope();
    bool HasRope() const { return m_hRopeEntity.Get() != NULL; }

    // Balloon settings
    void SetBalloonPower(float power);
    void SetBalloonReverse(bool reverse);
    void SetExplodeOnDamage(bool explode);

    // Input handlers
    void InputSetPower(inputdata_t &inputdata);
    void InputSetReverse(inputdata_t &inputdata);
    void InputExplode(inputdata_t &inputdata);

    // Network variables
    CNetworkVar(float, m_flBalloonPower);
    CNetworkVar(bool, m_bBalloonReverse);
    CNetworkVar(bool, m_bExplodeOnDamage);

private:
    // Rope entity
    EHANDLE m_hRopeEntity;

    // Rope configuration
    float m_flRopeWidth;
    float m_flRopeLength;
    float m_flRopeForceLimit;
    bool m_bRopeRigid;
    int m_iRopeType;

    // Balloon settings
    float m_flLiftThink;
    Vector m_vecLiftForce;

    // Setup functions
    void SetupBalloonPhysics();
    void SetupBalloonModel();
    bool CanApplyLift();
};

//-----------------------------------------------------------------------------
// Balloon Management System
//-----------------------------------------------------------------------------
class CGModBalloonSystem
{
public:
    // System initialization
    static void Initialize();
    static void Shutdown();

    // Console variable management
    static void UpdateBalloonSettings();

    // Balloon creation
    static CGModBalloon* CreateBalloon(const Vector& position, CBasePlayer* pOwner);

private:
    static bool m_bInitialized;
};

// Console variables for balloon system
extern ConVar gm_balloon_reverse;
extern ConVar gm_balloon_power;
extern ConVar gm_balloon_rope_width;
extern ConVar gm_balloon_rope_length;
extern ConVar gm_balloon_rope_forcelimit;
extern ConVar gm_balloon_rope_rigid;
extern ConVar gm_balloon_rope_type;
extern ConVar gm_balloon_explode;

#endif // GMOD_BALLOON_H
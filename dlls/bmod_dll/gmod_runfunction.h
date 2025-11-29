#ifndef GMOD_RUNFUNCTION_H
#define GMOD_RUNFUNCTION_H

#pragma once

#include "cbase.h"
#include "baseentity.h"

// Forward declarations
class CBasePlayer;

//-----------------------------------------------------------------------------
// CGmodRunFunction - Entity that executes Lua functions when triggered
// Based on IDA analysis: CGmodRunFunction, RunFunction, InputRunFunction
//-----------------------------------------------------------------------------
class CGmodRunFunction : public CBaseEntity
{
    DECLARE_CLASS(CGmodRunFunction, CBaseEntity);
    DECLARE_DATADESC();

public:
    virtual void Spawn();
    virtual void Think();

    // Input handlers
    void InputRunFunction(inputdata_t &inputdata);

    // Function execution
    void SetFunction(const char *pszFunction);
    void SetDelay(float flDelay);

private:
    string_t m_iszFunction;     // Lua function to call
    float m_flDelay;            // Delay before execution

    COutputEvent m_OnFinished;  // Fired when function completes
};

// Forward declaration for game setup entity (defined in gmod_gamesetup.cpp)
class CGModGameSetup;

#endif // GMOD_RUNFUNCTION_H
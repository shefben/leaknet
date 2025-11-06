#include "cbase.h"
#include "gmod_expressions.h"
#include "player.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "convar.h"
#include "baseanimating.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Console variables - based on IDA strings
ConVar gm_facescale("gm_facescale", "1.0", FCVAR_NONE, "The scale of the ragdoll's expression");
ConVar gm_faceposer_reload("gm_faceposer_reload", "0", FCVAR_NONE, "Reload the expressions file every time you pose a face");
ConVar special_facepose("special_facepose", "", FCVAR_NONE, "Special face pose mode");

// Flex controller ConVars (based on IDA strings: gm_facepose_flex0-64)
ConVar* g_FlexConVars[MAX_FLEX_CONTROLLERS];

// Console commands
ConCommand gmod_list_expressions("gmod_list_expressions", CC_GMod_ListExpressions, "List all registered expressions");
ConCommand gmod_show_expression("gmod_show_expression", CC_GMod_ShowExpression, "Show an expression by name");
ConCommand gmod_clear_expression("gmod_clear_expression", CC_GMod_ClearExpression, "Clear current expression");
ConCommand gmod_set_flex("gmod_set_flex", CC_GMod_SetFlex, "Set flex controller value");
ConCommand gmod_reset_flex("gmod_reset_flex", CC_GMod_ResetFlex, "Reset all flex controllers");
ConCommand gmod_reload_expressions("gmod_reload_expressions", CC_GMod_ReloadExpressions, "Reload expression settings");
ConCommand facepose("facepose", CC_GMod_FacePose, "Face pose command");

// Global instance
CGModExpressionsSystem g_GMod_ExpressionsSystem;
CGModExpressionsSystem* g_pGModExpressionsSystem = &g_GMod_ExpressionsSystem;

//-----------------------------------------------------------------------------
// CGModExpressionsSystem implementation
//-----------------------------------------------------------------------------
CGModExpressionsSystem::CGModExpressionsSystem() : CAutoGameSystem()
{
    m_iNumFlexControllers = 0;
    m_bReloadOnPose = false;

    SetDefLessFunc(m_PlayerExpressionStates);
}

CGModExpressionsSystem::~CGModExpressionsSystem()
{
    Shutdown();
}

bool CGModExpressionsSystem::Init()
{
    InitializeFlexControllers();
    return true;
}

void CGModExpressionsSystem::Shutdown()
{
    // Clear all player expression states
    FOR_EACH_MAP(m_PlayerExpressionStates, i)
    {
        // Nothing special to clean up for expressions
    }
    m_PlayerExpressionStates.RemoveAll();

    m_RegisteredExpressions.RemoveAll();
}

void CGModExpressionsSystem::LevelInitPostEntity()
{
    LoadExpressions();
}

void CGModExpressionsSystem::FrameUpdatePreEntityThink()
{
    UpdatePlayerExpressions();
}

// Implementation based on GMod_LoadExpressions function found in IDA (0x2418ea60)
bool CGModExpressionsSystem::LoadExpressions()
{
    // Based on IDA string: "settings/gmod_expressions.txt"
    const char* pszFilename = "settings/gmod_expressions.txt";

    // Check if file exists
    if (!filesystem->FileExists(pszFilename, "GAME"))
    {
        // Based on IDA string: "Error loading expressions..."
        DevMsg("Error loading expressions...\n");
        return false;
    }

    KeyValues* pKV = new KeyValues("GMod_Expressions");
    if (!pKV->LoadFromFile(filesystem, pszFilename, "GAME"))
    {
        DevMsg("Failed to parse gmod_expressions.txt\n");
        pKV->deleteThis();
        return false;
    }

    // Clear existing expressions
    m_RegisteredExpressions.RemoveAll();

    // Parse expression definitions
    ParseExpressionFile(pKV);

    pKV->deleteThis();
    DevMsg("Loaded %d expression definitions from gmod_expressions.txt\n", m_RegisteredExpressions.Count());
    return true;
}

void CGModExpressionsSystem::ReloadExpressions()
{
    LoadExpressions();
}

bool CGModExpressionsSystem::InitializeFlexControllers()
{
    // Initialize flex controller mappings based on IDA strings (gm_facepose_flex0-64)
    for (int i = 0; i < MAX_FLEX_CONTROLLERS; i++)
    {
        m_FlexControllers[i].iIndex = i;
        Q_snprintf(m_FlexControllers[i].szConVarName, sizeof(m_FlexControllers[i].szConVarName), "gm_facepose_flex%d", i);
        Q_snprintf(m_FlexControllers[i].szFlexName, sizeof(m_FlexControllers[i].szFlexName), "flex_%d", i);

        // Create ConVar for this flex controller
        char szDesc[64];
        Q_snprintf(szDesc, sizeof(szDesc), "Flex controller %d value", i);

        // Note: These ConVars are created dynamically to match IDA findings
        // In a real implementation, these would be statically defined
        g_FlexConVars[i] = new ConVar(m_FlexControllers[i].szConVarName, "0.0", FCVAR_NONE, szDesc);
    }

    m_iNumFlexControllers = MAX_FLEX_CONTROLLERS;
    return true;
}

void CGModExpressionsSystem::ParseExpressionFile(KeyValues* pKV)
{
    // Based on IDA string: "FaceExpressions" section
    KeyValues* pExpressions = pKV->FindKey("FaceExpressions");
    if (!pExpressions)
    {
        DevMsg("No FaceExpressions section found in expressions file\n");
        return;
    }

    // Parse expression entries
    for (KeyValues* pExpression = pExpressions->GetFirstSubKey(); pExpression; pExpression = pExpression->GetNextKey())
    {
        const char* pszName = pExpression->GetName();
        const char* pszDescription = pExpression->GetString("description", "");

        ExpressionData_t expressionData;
        Q_strncpy(expressionData.szName, pszName, sizeof(expressionData.szName));
        Q_strncpy(expressionData.szDescription, pszDescription, sizeof(expressionData.szDescription));

        // Parse flex values
        int iFlexCount = 0;
        for (int i = 0; i < MAX_FLEX_CONTROLLERS && iFlexCount < MAX_FLEX_CONTROLLERS; i++)
        {
            char szFlexKey[32];
            Q_snprintf(szFlexKey, sizeof(szFlexKey), "flex%d", i);

            if (pExpression->FindKey(szFlexKey))
            {
                expressionData.flFlexValues[i] = pExpression->GetFloat(szFlexKey, 0.0f);
                if (expressionData.flFlexValues[i] != 0.0f)
                    iFlexCount++;
            }
        }

        expressionData.iNumFlexUsed = iFlexCount;
        expressionData.bEnabled = pExpression->GetBool("enabled", true);

        RegisterExpression(pszName, pszDescription, expressionData.flFlexValues, iFlexCount);
    }
}

bool CGModExpressionsSystem::RegisterExpression(const char* pszName, const char* pszDescription, const float* pFlexValues, int iNumFlex)
{
    if (!pszName || !pFlexValues)
        return false;

    ExpressionData_t expressionData;
    Q_strncpy(expressionData.szName, pszName, sizeof(expressionData.szName));
    Q_strncpy(expressionData.szDescription, pszDescription, sizeof(expressionData.szDescription));

    memcpy(expressionData.flFlexValues, pFlexValues, sizeof(float) * MAX_FLEX_CONTROLLERS);
    expressionData.iNumFlexUsed = iNumFlex;
    expressionData.bEnabled = true;

    m_RegisteredExpressions.AddToTail(expressionData);
    return true;
}

ExpressionData_t* CGModExpressionsSystem::GetExpressionByName(const char* pszName)
{
    if (!pszName)
        return NULL;

    for (int i = 0; i < m_RegisteredExpressions.Count(); i++)
    {
        if (Q_stricmp(m_RegisteredExpressions[i].szName, pszName) == 0)
            return &m_RegisteredExpressions[i];
    }

    return NULL;
}

void CGModExpressionsSystem::SetPlayerExpression(CBasePlayer* pPlayer, const char* pszExpressionName, float flDuration)
{
    if (!pPlayer || !pszExpressionName)
        return;

    // Reload expressions if enabled (based on gm_faceposer_reload ConVar)
    if (gm_faceposer_reload.GetBool())
    {
        LoadExpressions();
    }

    ExpressionData_t* pExpressionData = GetExpressionByName(pszExpressionName);
    if (!pExpressionData)
    {
        DevMsg("Expression '%s' not found\n", pszExpressionName);
        return;
    }

    PlayerExpressionState_t* pState = GetPlayerExpressionState(pPlayer);
    if (!pState)
        return;

    // Set new expression
    pState->currentExpression = *pExpressionData;
    pState->flExpressionTime = gpGlobals->curtime;
    pState->flExpressionDuration = flDuration;
    pState->bExpressionActive = true;
    pState->flExpressionScale = gm_facescale.GetFloat();

    // Apply expression immediately
    ApplyExpressionToPlayer(pPlayer, pExpressionData, pState->flExpressionScale);

    DevMsg("Set expression '%s' for player %s\n", pszExpressionName, STRING(pPlayer->pl.netname));
}

void CGModExpressionsSystem::ClearPlayerExpression(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    PlayerExpressionState_t* pState = GetPlayerExpressionState(pPlayer);
    if (!pState)
        return;

    // Reset all flex values to 0
    memset(pState->flFlexValues, 0, sizeof(pState->flFlexValues));
    pState->bExpressionActive = false;

    // Apply reset values to player
    CBaseAnimating* pAnimating = GetPlayerAnimating(pPlayer);
    if (pAnimating)
    {
        for (int i = 0; i < MAX_FLEX_CONTROLLERS; i++)
        {
            SetFlexController(pAnimating, i, 0.0f);
        }
    }
}

void CGModExpressionsSystem::SetPlayerFlexController(CBasePlayer* pPlayer, int iFlexIndex, float flValue)
{
    if (!pPlayer || iFlexIndex < 0 || iFlexIndex >= MAX_FLEX_CONTROLLERS)
        return;

    PlayerExpressionState_t* pState = GetPlayerExpressionState(pPlayer);
    if (!pState)
        return;

    pState->flFlexValues[iFlexIndex] = flValue;

    // Apply to player model
    CBaseAnimating* pAnimating = GetPlayerAnimating(pPlayer);
    if (pAnimating)
    {
        SetFlexController(pAnimating, iFlexIndex, flValue * pState->flExpressionScale);
    }

    // Update corresponding ConVar (based on IDA pattern: gm_facepose_flex%i %.2f)
    if (g_FlexConVars[iFlexIndex])
    {
        g_FlexConVars[iFlexIndex]->SetValue(flValue);
        DevMsg("gm_facepose_flex%i %.2f\n", iFlexIndex, flValue);
    }
}

void CGModExpressionsSystem::SetPlayerFaceScale(CBasePlayer* pPlayer, float flScale)
{
    if (!pPlayer)
        return;

    PlayerExpressionState_t* pState = GetPlayerExpressionState(pPlayer);
    if (!pState)
        return;

    pState->flExpressionScale = flScale;
    gm_facescale.SetValue(flScale);

    // Re-apply current expression with new scale
    if (pState->bExpressionActive)
    {
        ApplyExpressionToPlayer(pPlayer, &pState->currentExpression, flScale);
    }
}

void CGModExpressionsSystem::UpdatePlayerExpressions()
{
    // Update all player expressions
    FOR_EACH_MAP(m_PlayerExpressionStates, i)
    {
        PlayerExpressionState_t& state = m_PlayerExpressionStates[i];

        if (!state.bExpressionActive)
            continue;

        // Check if expression has expired
        if (state.flExpressionDuration > 0 &&
            gpGlobals->curtime >= state.flExpressionTime + state.flExpressionDuration)
        {
            state.bExpressionActive = false;

            // Clear expression
            CBasePlayer* pPlayer = UTIL_PlayerByIndex(m_PlayerExpressionStates.Key(i));
            if (pPlayer)
            {
                ClearPlayerExpression(pPlayer);
            }
        }
    }

    // Clean up disconnected players
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
        if (!pPlayer)
        {
            CleanupPlayerExpressionState(i);
        }
    }
}

PlayerExpressionState_t* CGModExpressionsSystem::GetPlayerExpressionState(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return NULL;

    int playerIndex = pPlayer->entindex();
    int index = m_PlayerExpressionStates.Find(playerIndex);

    if (index == m_PlayerExpressionStates.InvalidIndex())
    {
        index = m_PlayerExpressionStates.Insert(playerIndex);
    }

    return &m_PlayerExpressionStates[index];
}

void CGModExpressionsSystem::CleanupPlayerExpressionState(int playerIndex)
{
    int index = m_PlayerExpressionStates.Find(playerIndex);
    if (index != m_PlayerExpressionStates.InvalidIndex())
    {
        m_PlayerExpressionStates.RemoveAt(index);
    }
}

void CGModExpressionsSystem::ApplyExpressionToPlayer(CBasePlayer* pPlayer, const ExpressionData_t* pExpression, float flScale)
{
    if (!pPlayer || !pExpression)
        return;

    CBaseAnimating* pAnimating = GetPlayerAnimating(pPlayer);
    if (!pAnimating)
        return;

    // Apply all flex values from expression
    for (int i = 0; i < MAX_FLEX_CONTROLLERS; i++)
    {
        if (pExpression->flFlexValues[i] != 0.0f)
        {
            SetFlexController(pAnimating, i, pExpression->flFlexValues[i] * flScale);
        }
    }
}

CBaseAnimating* CGModExpressionsSystem::GetPlayerAnimating(CBasePlayer* pPlayer)
{
    return dynamic_cast<CBaseAnimating*>(pPlayer);
}

void CGModExpressionsSystem::SetFlexController(CBaseAnimating* pAnimating, int iFlexIndex, float flValue)
{
    if (!pAnimating || iFlexIndex < 0)
        return;

    // Set flex controller on the animating entity
    pAnimating->SetFlexWeight(iFlexIndex, flValue);
}

// Implementation based on VFE file support (IDA string: "expressions/%s.vfe")
bool CGModExpressionsSystem::LoadVFEFile(const char* pszVFEPath)
{
    if (!pszVFEPath)
        return false;

    char szFullPath[MAX_PATH];
    Q_snprintf(szFullPath, sizeof(szFullPath), "expressions/%s.vfe", pszVFEPath);

    if (!filesystem->FileExists(szFullPath, "GAME"))
    {
        DevMsg("VFE file not found: %s\n", szFullPath);
        return false;
    }

    KeyValues* pKV = new KeyValues("VFE_Expression");
    if (!pKV->LoadFromFile(filesystem, szFullPath, "GAME"))
    {
        DevMsg("Failed to parse VFE file: %s\n", szFullPath);
        pKV->deleteThis();
        return false;
    }

    ParseVFEFile(pKV, pszVFEPath);
    pKV->deleteThis();
    return true;
}

void CGModExpressionsSystem::ParseVFEFile(KeyValues* pKV, const char* pszVFEName)
{
    // Parse VFE format (simplified implementation)
    float flFlexValues[MAX_FLEX_CONTROLLERS];
    memset(flFlexValues, 0, sizeof(flFlexValues));

    // Parse flex settings from VFE
    for (KeyValues* pFlex = pKV->GetFirstSubKey(); pFlex; pFlex = pFlex->GetNextKey())
    {
        const char* pszFlexName = pFlex->GetName();
        float flValue = pFlex->GetFloat();

        // Map flex name to index (simplified)
        for (int i = 0; i < MAX_FLEX_CONTROLLERS; i++)
        {
            if (Q_stricmp(m_FlexControllers[i].szFlexName, pszFlexName) == 0)
            {
                flFlexValues[i] = flValue;
                break;
            }
        }
    }

    // Register as expression
    RegisterExpression(pszVFEName, "VFE Expression", flFlexValues, MAX_FLEX_CONTROLLERS);
}

void CGModExpressionsSystem::ListExpressions()
{
    Msg("Registered expressions (%d):\n", m_RegisteredExpressions.Count());
    for (int i = 0; i < m_RegisteredExpressions.Count(); i++)
    {
        const ExpressionData_t& expression = m_RegisteredExpressions[i];
        Msg("  %s: %s (flex used: %d, enabled: %s)\n",
            expression.szName, expression.szDescription, expression.iNumFlexUsed,
            expression.bEnabled ? "yes" : "no");
    }
}

void CGModExpressionsSystem::ShowExpression(const char* pszName, float flDuration)
{
    if (!pszName)
        return;

    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (pPlayer)
    {
        SetPlayerExpression(pPlayer, pszName, flDuration);
    }
}

void CGModExpressionsSystem::ClearExpression()
{
    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (pPlayer)
    {
        ClearPlayerExpression(pPlayer);
    }
}

void CGModExpressionsSystem::SetFlexValue(int iIndex, float flValue)
{
    if (iIndex < 0 || iIndex >= MAX_FLEX_CONTROLLERS)
        return;

    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (pPlayer)
    {
        SetPlayerFlexController(pPlayer, iIndex, flValue);
    }
}

void CGModExpressionsSystem::ResetAllFlexValues()
{
    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (pPlayer)
    {
        ClearPlayerExpression(pPlayer);
    }
}

//-----------------------------------------------------------------------------
// Console command implementations
//-----------------------------------------------------------------------------
void CC_GMod_ListExpressions(void)
{
    if (g_pGModExpressionsSystem)
    {
        g_pGModExpressionsSystem->ListExpressions();
    }
}

void CC_GMod_ShowExpression(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_show_expression <expression_name> [duration]\n");
        return;
    }

    const char* pszName = engine->Cmd_Argv(1);
    float flDuration = -1.0f;

    if (engine->Cmd_Argc() >= 3)
    {
        flDuration = atof(engine->Cmd_Argv(2));
    }

    if (g_pGModExpressionsSystem)
    {
        g_pGModExpressionsSystem->ShowExpression(pszName, flDuration);
    }
}

void CC_GMod_ClearExpression(void)
{
    if (g_pGModExpressionsSystem)
    {
        g_pGModExpressionsSystem->ClearExpression();
    }
}

void CC_GMod_SetFlex(void)
{
    if (engine->Cmd_Argc() < 3)
    {
        Msg("Usage: gmod_set_flex <flex_index> <value>\n");
        return;
    }

    int iIndex = atoi(engine->Cmd_Argv(1));
    float flValue = atof(engine->Cmd_Argv(2));

    if (g_pGModExpressionsSystem)
    {
        g_pGModExpressionsSystem->SetFlexValue(iIndex, flValue);
    }
}

void CC_GMod_ResetFlex(void)
{
    if (g_pGModExpressionsSystem)
    {
        g_pGModExpressionsSystem->ResetAllFlexValues();
    }
}

void CC_GMod_ReloadExpressions(void)
{
    if (g_pGModExpressionsSystem)
    {
        g_pGModExpressionsSystem->ReloadExpressions();
        Msg("Expression settings reloaded.\n");
    }
}

void CC_GMod_FacePose(void)
{
    // Implementation based on "facepose" command from IDA
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: facepose <expression_name>\n");
        return;
    }

    const char* pszExpression = engine->Cmd_Argv(1);

    if (g_pGModExpressionsSystem)
    {
        g_pGModExpressionsSystem->ShowExpression(pszExpression);
    }
}
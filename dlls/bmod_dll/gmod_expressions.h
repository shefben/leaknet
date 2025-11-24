#ifndef GMOD_EXPRESSIONS_H
#define GMOD_EXPRESSIONS_H

#include "cbase.h"
#include "igamesystem.h"
#include "filesystem.h"
#include "utlmap.h"

// Forward declarations
class CBasePlayer;
class CBaseAnimating;

// Maximum number of flex controllers supported (based on IDA analysis: gm_facepose_flex0-64)
#define MAX_FLEX_CONTROLLERS 65

// Expression data structure
struct ExpressionData_t
{
    char szName[64];
    char szDescription[256];
    float flFlexValues[MAX_FLEX_CONTROLLERS];
    int iNumFlexUsed;
    bool bEnabled;

    ExpressionData_t()
    {
        szName[0] = '\0';
        szDescription[0] = '\0';
        memset(flFlexValues, 0, sizeof(flFlexValues));
        iNumFlexUsed = 0;
        bEnabled = true;
    }
};

// Flex controller names mapping (based on IDA strings)
struct FlexControllerMapping_t
{
    int iIndex;
    char szConVarName[32];
    char szFlexName[32];
};

// Player expression state
struct PlayerExpressionState_t
{
    ExpressionData_t currentExpression;
    float flExpressionScale; // Based on gm_facescale ConVar
    float flFlexValues[MAX_FLEX_CONTROLLERS];
    bool bExpressionActive;
    float flExpressionTime;
    float flExpressionDuration;

    PlayerExpressionState_t()
    {
        flExpressionScale = 1.0f;
        memset(flFlexValues, 0, sizeof(flFlexValues));
        bExpressionActive = false;
        flExpressionTime = 0.0f;
        flExpressionDuration = 0.0f;
    }
};

//-----------------------------------------------------------------------------
// GMod Facial Expressions System
//-----------------------------------------------------------------------------
class CGModExpressionsSystem : public CAutoGameSystem
{
public:
    CGModExpressionsSystem();
    virtual ~CGModExpressionsSystem();

    // CAutoGameSystem overrides
    virtual bool Init();
    virtual void Shutdown();
    virtual void LevelInitPostEntity();
    virtual void FrameUpdatePreEntityThink();

    // Core expression functionality
    bool LoadExpressions(); // Based on GMod_LoadExpressions function (IDA: 0x2418ea60)
    void ReloadExpressions();

    // Expression management
    bool RegisterExpression(const char* pszName, const char* pszDescription, const float* pFlexValues, int iNumFlex);
    ExpressionData_t* GetExpressionByName(const char* pszName);
    void ListExpressions();

    // Player expression control
    void SetPlayerExpression(CBasePlayer* pPlayer, const char* pszExpressionName, float flDuration = -1.0f);
    void ClearPlayerExpression(CBasePlayer* pPlayer);
    void SetPlayerFlexController(CBasePlayer* pPlayer, int iFlexIndex, float flValue);
    void SetPlayerFaceScale(CBasePlayer* pPlayer, float flScale);
    void UpdatePlayerExpressions();

    // Flex controller management
    bool InitializeFlexControllers();
    int GetFlexControllerIndex(const char* pszFlexName);
    const char* GetFlexControllerName(int iIndex);
    void SetFlexController(CBaseAnimating* pAnimating, int iFlexIndex, float flValue);

    // VFE file support (based on IDA string: "expressions/%s.vfe")
    bool LoadVFEFile(const char* pszVFEPath);
    void ApplyVFEToPlayer(CBasePlayer* pPlayer, const char* pszVFEName);

    // Console commands
    void ShowExpression(const char* pszName, float flDuration = -1.0f);
    void ClearExpression();
    void SetFlexValue(int iIndex, float flValue);
    void ResetAllFlexValues();

private:
    // Internal data
    CUtlVector<ExpressionData_t> m_RegisteredExpressions;
    CUtlMap<int, PlayerExpressionState_t> m_PlayerExpressionStates; // Player expression states mapped by player index
    FlexControllerMapping_t m_FlexControllers[MAX_FLEX_CONTROLLERS];
    int m_iNumFlexControllers;

    // Settings
    bool m_bReloadOnPose; // Based on gm_faceposer_reload ConVar

    // Helper functions
    void ParseExpressionFile(KeyValues* pKV);
    void ParseVFEFile(KeyValues* pKV, const char* pszVFEName);
    PlayerExpressionState_t* GetPlayerExpressionState(CBasePlayer* pPlayer);
    void CleanupPlayerExpressionState(int playerIndex);
    void ApplyExpressionToPlayer(CBasePlayer* pPlayer, const ExpressionData_t* pExpression, float flScale = 1.0f);
    CBaseAnimating* GetPlayerAnimating(CBasePlayer* pPlayer);
};

// Global access
extern CGModExpressionsSystem* g_pGModExpressionsSystem;

// Console commands
void CC_GMod_ListExpressions(void);
void CC_GMod_ShowExpression(void);
void CC_GMod_ClearExpression(void);
void CC_GMod_SetFlex(void);
void CC_GMod_ResetFlex(void);
void CC_GMod_ReloadExpressions(void);
void CC_GMod_FacePose(void);

#endif // GMOD_EXPRESSIONS_H
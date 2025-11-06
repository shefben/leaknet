#ifndef GMOD_SPRITES_H
#define GMOD_SPRITES_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "convar.h"
#include "utlvector.h"
#include "utlstring.h"
#include "utldict.h"

// Forward declarations
class CBaseEntity;
class CSprite;

// Sprite types discovered from IDA string analysis
enum SpriteType_t
{
    SPRITE_TYPE_DEFAULT = 0,
    SPRITE_TYPE_GLOW,           // sprites/glow01.vmt
    SPRITE_TYPE_LASER,          // sprites/laserbeam.vmt, sprites/laser.vmt
    SPRITE_TYPE_LIGHT,          // sprites/bluelight1.vmt, sprites/light_glow03.vmt
    SPRITE_TYPE_EFFECT,         // sprites/fire1.vmt, sprites/steam1.vmt
    SPRITE_TYPE_EXPLOSION,      // sprites/WXplo1.vmt, sprites/zerogxplode.vmt
    SPRITE_TYPE_BEAM,           // sprites/physbeam.vmt
    SPRITE_TYPE_PARTICLE,       // sprites/twinkle01.vmt
    SPRITE_TYPE_MAX
};

// Sprite rendering modes
enum SpriteRenderMode_t
{
    SPRITE_RENDER_NORMAL = 0,
    SPRITE_RENDER_ADDITIVE,
    SPRITE_RENDER_ALPHA_TEST,
    SPRITE_RENDER_ALPHA_BLEND,
    SPRITE_RENDER_GLOW,
    SPRITE_RENDER_MAX
};

// Sprite configuration data loaded from settings/gmod_sprites.txt
struct SpriteData_t
{
    CUtlString spriteName;
    CUtlString materialPath;
    CUtlString category;

    // Visual properties
    float flScale;              // Sprite scale
    float flBrightness;         // Sprite brightness
    Color spriteColor;          // Sprite color tint
    SpriteRenderMode_t renderMode;

    // Animation properties
    bool bAnimated;
    float flFrameRate;
    int iFrameCount;
    bool bLooping;

    // Behavior properties
    bool bWorldSprite;          // World sprite vs screen sprite
    bool bFacePlayer;           // Always face player
    bool bNoFog;                // Ignore fog
    bool bNoCull;               // Don't cull
    float flLifeTime;           // Auto-remove after time

    // Physics properties
    bool bCollide;              // Collides with world
    float flFadeDistance;       // Distance to start fading
    float flMaxDistance;        // Max visible distance

    SpriteType_t spriteType;
    bool bEnabled;

    SpriteData_t()
    {
        flScale = 1.0f;
        flBrightness = 1.0f;
        spriteColor = Color(255, 255, 255, 255);
        renderMode = SPRITE_RENDER_NORMAL;
        bAnimated = false;
        flFrameRate = 10.0f;
        iFrameCount = 1;
        bLooping = true;
        bWorldSprite = true;
        bFacePlayer = true;
        bNoFog = false;
        bNoCull = false;
        flLifeTime = 0.0f;
        bCollide = false;
        flFadeDistance = 512.0f;
        flMaxDistance = 1024.0f;
        spriteType = SPRITE_TYPE_DEFAULT;
        bEnabled = true;
    }
};

// Sprite instance management
struct SpriteInstance_t
{
    CSprite* pSprite;
    SpriteData_t config;
    float flCreationTime;
    bool bActive;
    int iInstanceId;

    SpriteInstance_t()
    {
        pSprite = NULL;
        flCreationTime = 0.0f;
        bActive = false;
        iInstanceId = -1;
    }
};

//-----------------------------------------------------------------------------
// GMod Sprites System - Manages sprite configurations and instances
// Based on IDA analysis: settings/gmod_sprites.txt, m_sprites discovered strings
//-----------------------------------------------------------------------------
class CGModSpritesSystem : public CAutoGameSystem
{
public:
    CGModSpritesSystem() : CAutoGameSystem("GMod Sprites System") {}

    // CAutoGameSystem overrides
    virtual bool Init() override;
    virtual void Shutdown() override;
    virtual void LevelInitPostEntity() override;
    virtual void FrameUpdatePreEntityThink() override;

    // Sprite system functions discovered from IDA
    static bool LoadSprites(); // Loads settings/gmod_sprites.txt
    static bool ReloadSprites();
    static SpriteData_t* FindSpriteData(const char* pszSpriteName);
    static bool RegisterSpriteConfig(const char* pszSpriteName, const SpriteData_t& config);

    // Sprite creation and management
    static CSprite* CreateSprite(const char* pszSpriteName, const Vector& origin, CBaseEntity* pOwner = NULL);
    static CSprite* CreateConfiguredSprite(const char* pszSpriteName, const Vector& origin, const QAngle& angles = vec3_angle);
    static bool ApplySpriteConfig(CSprite* pSprite, const SpriteData_t& config);
    static void RemoveSprite(CSprite* pSprite);
    static void RemoveAllSprites();

    // Sprite instance management
    static int CreateSpriteInstance(const char* pszSpriteName, const Vector& origin);
    static bool DestroySpriteInstance(int iInstanceId);
    static SpriteInstance_t* FindSpriteInstance(int iInstanceId);
    static void UpdateSpriteInstances();

    // Sprite manipulation
    static bool SetSpriteScale(CSprite* pSprite, float flScale);
    static bool SetSpriteColor(CSprite* pSprite, const Color& color);
    static bool SetSpriteBrightness(CSprite* pSprite, float flBrightness);
    static bool SetSpriteFrame(CSprite* pSprite, int iFrame);
    static bool PlaySpriteAnimation(CSprite* pSprite, bool bLoop = true);

    // Utility functions for discovered sprite types
    static CSprite* CreateGlowSprite(const Vector& origin, const Color& color, float flScale = 1.0f);
    static CSprite* CreateLaserDot(const Vector& origin, const Color& color = Color(255, 0, 0, 255));
    static CSprite* CreateLightGlow(const Vector& origin, const Color& color, float flBrightness = 1.0f);
    static CSprite* CreateExplosionSprite(const Vector& origin, float flScale = 1.0f);

    // Query functions
    static int GetSpriteConfigCount();
    static int GetActiveSpriteCount();
    static void GetSpriteList(CUtlVector<CUtlString>& spriteList);
    static void GetActiveSpriteInstances(CUtlVector<int>& instanceList);
    static bool IsSpriteConfigured(const char* pszSpriteName);
    static bool IsSpriteEnabled(const char* pszSpriteName);

    // Utility functions
    static const char* GetSpriteTypeName(SpriteType_t type);
    static const char* GetRenderModeName(SpriteRenderMode_t mode);
    static const char* GetSpritesPath() { return "settings/gmod_sprites.txt"; }

private:
    static CUtlDict<SpriteData_t, int> s_SpriteConfigs;
    static CUtlVector<SpriteInstance_t> s_SpriteInstances;
    static int s_iNextInstanceId;
    static bool s_bSystemInitialized;
    static bool s_bSpritesLoaded;

    // Helper functions based on IDA analysis
    static bool ParseSpritesFile(const char* pszFilePath);
    static bool ParseSpriteSection(KeyValues* pKV, SpriteData_t* pSprite);
    static SpriteType_t ParseSpriteType(const char* pszTypeName);
    static SpriteRenderMode_t ParseRenderMode(const char* pszModeName);
    static void ApplyDefaultSprites();
    static void CleanupExpiredSprites();
};

// Global instance
extern CGModSpritesSystem g_GMod_SpritesSystem;

// Console command handlers
void CMD_gmod_sprites_list(void);
void CMD_gmod_sprites_reload(void);
void CMD_gmod_sprites_create(void);
void CMD_gmod_sprites_remove(void);
void CMD_gmod_sprites_clear(void);
void CMD_gmod_sprites_info(void);

#endif // GMOD_SPRITES_H
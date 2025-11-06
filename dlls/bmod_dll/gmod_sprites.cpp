#include "cbase.h"
#include "gmod_sprites.h"
#include "player.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "sprite.h"
#include "beam_shared.h"
#include "te_effect_dispatch.h"
#include "util.h"

// ConCommand registration
static ConCommand gmod_sprites_list("gmod_sprites_list", CMD_gmod_sprites_list, "List all sprite configurations");
static ConCommand gmod_sprites_reload("gmod_sprites_reload", CMD_gmod_sprites_reload, "Reload sprite configurations");
static ConCommand gmod_sprites_create("gmod_sprites_create", CMD_gmod_sprites_create, "Create sprite");
static ConCommand gmod_sprites_remove("gmod_sprites_remove", CMD_gmod_sprites_remove, "Remove sprite");
static ConCommand gmod_sprites_clear("gmod_sprites_clear", CMD_gmod_sprites_clear, "Remove all sprites");
static ConCommand gmod_sprites_info("gmod_sprites_info", CMD_gmod_sprites_info, "Show sprite info");

// Static member initialization
CUtlDict<SpriteData_t, int> CGModSpritesSystem::s_SpriteConfigs;
CUtlVector<SpriteInstance_t> CGModSpritesSystem::s_SpriteInstances;
int CGModSpritesSystem::s_iNextInstanceId = 1;
bool CGModSpritesSystem::s_bSystemInitialized = false;
bool CGModSpritesSystem::s_bSpritesLoaded = false;

// Global instance
CGModSpritesSystem g_GMod_SpritesSystem;

//-----------------------------------------------------------------------------
// Purpose: Initialize the sprites system
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::Init()
{
    if (s_bSystemInitialized)
        return true;

    Msg("Initializing GMod Sprites System...\n");

    s_SpriteConfigs.Purge();
    s_SpriteInstances.Purge();
    s_iNextInstanceId = 1;

    // Load sprite configurations from settings/gmod_sprites.txt
    if (!LoadSprites())
    {
        Warning("Error loading gmod_sprites.txt!\n");
        // Continue with default sprites
    }

    s_bSystemInitialized = true;
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown the sprites system
//-----------------------------------------------------------------------------
void CGModSpritesSystem::Shutdown()
{
    if (!s_bSystemInitialized)
        return;

    // Remove all active sprites
    RemoveAllSprites();

    s_SpriteConfigs.Purge();
    s_SpriteInstances.Purge();
    s_bSystemInitialized = false;
    s_bSpritesLoaded = false;
}

//-----------------------------------------------------------------------------
// Purpose: Level initialization
//-----------------------------------------------------------------------------
void CGModSpritesSystem::LevelInitPostEntity()
{
    // Remove all sprites when level changes
    RemoveAllSprites();
    DevMsg("Sprites: Level initialized with %d sprite configs\n", s_SpriteConfigs.Count());
}

//-----------------------------------------------------------------------------
// Purpose: Frame update
//-----------------------------------------------------------------------------
void CGModSpritesSystem::FrameUpdatePreEntityThink()
{
    if (!s_bSystemInitialized)
        return;

    // Update sprite instances
    UpdateSpriteInstances();

    // Clean up expired sprites
    CleanupExpiredSprites();
}

//-----------------------------------------------------------------------------
// Purpose: Load sprites from settings/gmod_sprites.txt
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::LoadSprites()
{
    const char* pszConfigPath = GetSpritesPath();

    if (!filesystem->FileExists(pszConfigPath, "GAME"))
    {
        Warning("Sprites: Config file not found: %s\n", pszConfigPath);
        return false;
    }

    return ParseSpritesFile(pszConfigPath);
}

//-----------------------------------------------------------------------------
// Purpose: Parse sprites file
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::ParseSpritesFile(const char* pszFilePath)
{
    KeyValues* pKV = new KeyValues("Sprites");
    if (!pKV->LoadFromFile(filesystem, pszFilePath, "GAME"))
    {
        pKV->deleteThis();
        return false;
    }

    s_SpriteConfigs.Purge();

    // Load default sprites
    ApplyDefaultSprites();

    int spriteCount = 0;

    // Parse sprites section
    FOR_EACH_SUBKEY(pKV, pSprite)
    {
        SpriteData_t sprite;
        sprite.spriteName = pSprite->GetName();

        if (ParseSpriteSection(pSprite, &sprite))
        {
            int index = s_SpriteConfigs.Insert(sprite.spriteName.Get(), sprite);
            if (index != s_SpriteConfigs.InvalidIndex())
            {
                spriteCount++;
                DevMsg("Loaded sprite config: %s (%s)\n", sprite.spriteName.Get(), sprite.materialPath.Get());
            }
        }
    }

    pKV->deleteThis();
    s_bSpritesLoaded = true;

    DevMsg("Sprites: Loaded %d sprite configurations\n", spriteCount);
    return spriteCount > 0;
}

//-----------------------------------------------------------------------------
// Purpose: Parse sprite section
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::ParseSpriteSection(KeyValues* pKV, SpriteData_t* pSprite)
{
    if (!pKV || !pSprite)
        return false;

    // Parse basic properties
    pSprite->materialPath = pKV->GetString("material", "sprites/glow01");
    pSprite->category = pKV->GetString("category", "default");

    // Parse visual properties
    pSprite->flScale = pKV->GetFloat("scale", 1.0f);
    pSprite->flBrightness = pKV->GetFloat("brightness", 1.0f);

    const char* pszColor = pKV->GetString("color", "255 255 255 255");
    int r, g, b, a;
    if (sscanf(pszColor, "%d %d %d %d", &r, &g, &b, &a) == 4)
    {
        pSprite->spriteColor = Color(r, g, b, a);
    }

    const char* pszRenderMode = pKV->GetString("rendermode", "normal");
    pSprite->renderMode = ParseRenderMode(pszRenderMode);

    // Parse animation properties
    pSprite->bAnimated = pKV->GetBool("animated", false);
    pSprite->flFrameRate = pKV->GetFloat("framerate", 10.0f);
    pSprite->iFrameCount = pKV->GetInt("framecount", 1);
    pSprite->bLooping = pKV->GetBool("looping", true);

    // Parse behavior properties
    pSprite->bWorldSprite = pKV->GetBool("worldsprite", true);
    pSprite->bFacePlayer = pKV->GetBool("faceplayer", true);
    pSprite->bNoFog = pKV->GetBool("nofog", false);
    pSprite->bNoCull = pKV->GetBool("nocull", false);
    pSprite->flLifeTime = pKV->GetFloat("lifetime", 0.0f);

    // Parse physics properties
    pSprite->bCollide = pKV->GetBool("collide", false);
    pSprite->flFadeDistance = pKV->GetFloat("fadedistance", 512.0f);
    pSprite->flMaxDistance = pKV->GetFloat("maxdistance", 1024.0f);

    // Parse sprite type
    const char* pszType = pKV->GetString("type", "default");
    pSprite->spriteType = ParseSpriteType(pszType);

    pSprite->bEnabled = pKV->GetBool("enabled", true);

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Parse sprite type
//-----------------------------------------------------------------------------
SpriteType_t CGModSpritesSystem::ParseSpriteType(const char* pszTypeName)
{
    if (!pszTypeName)
        return SPRITE_TYPE_DEFAULT;

    if (!Q_stricmp(pszTypeName, "glow"))
        return SPRITE_TYPE_GLOW;
    else if (!Q_stricmp(pszTypeName, "laser"))
        return SPRITE_TYPE_LASER;
    else if (!Q_stricmp(pszTypeName, "light"))
        return SPRITE_TYPE_LIGHT;
    else if (!Q_stricmp(pszTypeName, "effect"))
        return SPRITE_TYPE_EFFECT;
    else if (!Q_stricmp(pszTypeName, "explosion"))
        return SPRITE_TYPE_EXPLOSION;
    else if (!Q_stricmp(pszTypeName, "beam"))
        return SPRITE_TYPE_BEAM;
    else if (!Q_stricmp(pszTypeName, "particle"))
        return SPRITE_TYPE_PARTICLE;

    return SPRITE_TYPE_DEFAULT;
}

//-----------------------------------------------------------------------------
// Purpose: Parse render mode
//-----------------------------------------------------------------------------
SpriteRenderMode_t CGModSpritesSystem::ParseRenderMode(const char* pszModeName)
{
    if (!pszModeName)
        return SPRITE_RENDER_NORMAL;

    if (!Q_stricmp(pszModeName, "additive"))
        return SPRITE_RENDER_ADDITIVE;
    else if (!Q_stricmp(pszModeName, "alphatest"))
        return SPRITE_RENDER_ALPHA_TEST;
    else if (!Q_stricmp(pszModeName, "alphablend"))
        return SPRITE_RENDER_ALPHA_BLEND;
    else if (!Q_stricmp(pszModeName, "glow"))
        return SPRITE_RENDER_GLOW;

    return SPRITE_RENDER_NORMAL;
}

//-----------------------------------------------------------------------------
// Purpose: Apply default sprites discovered from IDA analysis
//-----------------------------------------------------------------------------
void CGModSpritesSystem::ApplyDefaultSprites()
{
    // Create default sprite configurations based on strings found in IDA
    struct DefaultSpriteInfo_t
    {
        const char* name;
        const char* material;
        SpriteType_t type;
        SpriteRenderMode_t renderMode;
    };

    DefaultSpriteInfo_t defaultSprites[] = {
        // Glow sprites
        { "glow01", "sprites/glow01.vmt", SPRITE_TYPE_GLOW, SPRITE_RENDER_GLOW },
        { "glow_test02", "sprites/glow_test02.vmt", SPRITE_TYPE_GLOW, SPRITE_RENDER_GLOW },
        { "bluelight1", "sprites/bluelight1.vmt", SPRITE_TYPE_LIGHT, SPRITE_RENDER_ADDITIVE },
        { "light_glow03", "sprites/light_glow03.vmt", SPRITE_TYPE_LIGHT, SPRITE_RENDER_ADDITIVE },

        // Laser sprites
        { "laserbeam", "sprites/laserbeam.vmt", SPRITE_TYPE_LASER, SPRITE_RENDER_ADDITIVE },
        { "laser", "sprites/laser.vmt", SPRITE_TYPE_LASER, SPRITE_RENDER_ADDITIVE },
        { "laserdot", "sprites/laserdot.vmt", SPRITE_TYPE_LASER, SPRITE_RENDER_ADDITIVE },

        // Effect sprites
        { "fire1", "sprites/fire1.vmt", SPRITE_TYPE_EFFECT, SPRITE_RENDER_ADDITIVE },
        { "steam1", "sprites/steam1.vmt", SPRITE_TYPE_EFFECT, SPRITE_RENDER_ALPHA_BLEND },
        { "smoke", "sprites/smoke.vmt", SPRITE_TYPE_EFFECT, SPRITE_RENDER_ALPHA_BLEND },

        // Explosion sprites
        { "WXplo1", "sprites/WXplo1.vmt", SPRITE_TYPE_EXPLOSION, SPRITE_RENDER_ADDITIVE },
        { "zerogxplode", "sprites/zerogxplode.vmt", SPRITE_TYPE_EXPLOSION, SPRITE_RENDER_ADDITIVE },

        // Beam sprites
        { "physbeam", "sprites/physbeam.vmt", SPRITE_TYPE_BEAM, SPRITE_RENDER_ADDITIVE },

        // Particle sprites
        { "twinkle01", "sprites/twinkle01.vmt", SPRITE_TYPE_PARTICLE, SPRITE_RENDER_ADDITIVE },
        { "flare6", "sprites/flare6.vmt", SPRITE_TYPE_PARTICLE, SPRITE_RENDER_ADDITIVE },

        // Color variations
        { "bluelaser1", "sprites/bluelaser1.vmt", SPRITE_TYPE_LASER, SPRITE_RENDER_ADDITIVE },
        { "purplelaser1", "sprites/purplelaser1.vmt", SPRITE_TYPE_LASER, SPRITE_RENDER_ADDITIVE },
        { "redglow1", "sprites/redglow1.vmt", SPRITE_TYPE_GLOW, SPRITE_RENDER_GLOW },
        { "blueglow1", "sprites/blueglow1.vmt", SPRITE_TYPE_GLOW, SPRITE_RENDER_GLOW },
        { "greenglow1", "sprites/greenglow1.vmt", SPRITE_TYPE_GLOW, SPRITE_RENDER_GLOW },
        { "orangeglow1", "sprites/orangeglow1.vmt", SPRITE_TYPE_GLOW, SPRITE_RENDER_GLOW },
        { "yellowglow1", "sprites/yellowglow1.vmt", SPRITE_TYPE_GLOW, SPRITE_RENDER_GLOW },
    };

    for (int i = 0; i < ARRAYSIZE(defaultSprites); i++)
    {
        SpriteData_t sprite;
        sprite.spriteName = defaultSprites[i].name;
        sprite.materialPath = defaultSprites[i].material;
        sprite.spriteType = defaultSprites[i].type;
        sprite.renderMode = defaultSprites[i].renderMode;
        sprite.category = "default";

        // Set type-specific defaults
        switch (sprite.spriteType)
        {
            case SPRITE_TYPE_LASER:
                sprite.flScale = 0.1f;
                sprite.bFacePlayer = false;
                break;

            case SPRITE_TYPE_EXPLOSION:
                sprite.flLifeTime = 2.0f;
                sprite.bAnimated = true;
                sprite.flFrameRate = 15.0f;
                sprite.bLooping = false;
                break;

            case SPRITE_TYPE_EFFECT:
                sprite.bAnimated = true;
                sprite.flFrameRate = 12.0f;
                break;

            default:
                break;
        }

        s_SpriteConfigs.Insert(sprite.spriteName.Get(), sprite);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Reload sprites
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::ReloadSprites()
{
    s_SpriteConfigs.Purge();
    s_bSpritesLoaded = false;

    return LoadSprites();
}

//-----------------------------------------------------------------------------
// Purpose: Find sprite data
//-----------------------------------------------------------------------------
SpriteData_t* CGModSpritesSystem::FindSpriteData(const char* pszSpriteName)
{
    if (!pszSpriteName || !s_bSpritesLoaded)
        return NULL;

    int index = s_SpriteConfigs.Find(pszSpriteName);
    if (index != s_SpriteConfigs.InvalidIndex())
        return &s_SpriteConfigs[index];

    return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Register sprite config
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::RegisterSpriteConfig(const char* pszSpriteName, const SpriteData_t& config)
{
    if (!pszSpriteName)
        return false;

    int index = s_SpriteConfigs.Insert(pszSpriteName, config);
    return index != s_SpriteConfigs.InvalidIndex();
}

//-----------------------------------------------------------------------------
// Purpose: Create sprite
//-----------------------------------------------------------------------------
CSprite* CGModSpritesSystem::CreateSprite(const char* pszSpriteName, const Vector& origin, CBaseEntity* pOwner)
{
    SpriteData_t* pConfig = FindSpriteData(pszSpriteName);
    if (!pConfig || !pConfig->bEnabled)
        return NULL;

    CSprite* pSprite = CSprite::SpriteCreate(pConfig->materialPath.Get(), origin, false);
    if (!pSprite)
        return NULL;

    // Apply configuration
    if (!ApplySpriteConfig(pSprite, *pConfig))
    {
        UTIL_Remove(pSprite);
        return NULL;
    }

    return pSprite;
}

//-----------------------------------------------------------------------------
// Purpose: Create configured sprite
//-----------------------------------------------------------------------------
CSprite* CGModSpritesSystem::CreateConfiguredSprite(const char* pszSpriteName, const Vector& origin, const QAngle& angles)
{
    CSprite* pSprite = CreateSprite(pszSpriteName, origin);
    if (pSprite)
    {
        pSprite->SetAbsAngles(angles);
    }

    return pSprite;
}

//-----------------------------------------------------------------------------
// Purpose: Apply sprite config
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::ApplySpriteConfig(CSprite* pSprite, const SpriteData_t& config)
{
    if (!pSprite)
        return false;

    // Set scale
    pSprite->SetScale(config.flScale);

    // Set brightness
    pSprite->SetBrightness(config.flBrightness * 255.0f);

    // Set color
    pSprite->SetColor(config.spriteColor.r(), config.spriteColor.g(), config.spriteColor.b());
    pSprite->SetTransparency(kRenderTransAlpha, config.spriteColor.r(), config.spriteColor.g(), config.spriteColor.b(), config.spriteColor.a(), kRenderFxNone);

    // Set render mode
    switch (config.renderMode)
    {
        case SPRITE_RENDER_ADDITIVE:
            pSprite->SetTransparency(kRenderTransAdd, config.spriteColor.r(), config.spriteColor.g(), config.spriteColor.b(), config.spriteColor.a(), kRenderFxNone);
            break;

        case SPRITE_RENDER_ALPHA_TEST:
            pSprite->SetTransparency(kRenderTransAlphaTest, config.spriteColor.r(), config.spriteColor.g(), config.spriteColor.b(), config.spriteColor.a(), kRenderFxNone);
            break;

        case SPRITE_RENDER_GLOW:
            pSprite->SetTransparency(kRenderGlow, config.spriteColor.r(), config.spriteColor.g(), config.spriteColor.b(), config.spriteColor.a(), kRenderFxNoDissipation);
            break;

        default:
            break;
    }

    // Set animation
    if (config.bAnimated && config.iFrameCount > 1)
    {
        pSprite->AnimateAndDie(config.flFrameRate);
    }

    // Set lifetime
    if (config.flLifeTime > 0.0f)
    {
        pSprite->SetThink(&CSprite::SUB_Remove);
        pSprite->SetNextThink(gpGlobals->curtime + config.flLifeTime);
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Remove sprite
//-----------------------------------------------------------------------------
void CGModSpritesSystem::RemoveSprite(CSprite* pSprite)
{
    if (!pSprite)
        return;

    // Remove from instances list
    for (int i = 0; i < s_SpriteInstances.Count(); i++)
    {
        if (s_SpriteInstances[i].pSprite == pSprite)
        {
            s_SpriteInstances[i].bActive = false;
            s_SpriteInstances[i].pSprite = NULL;
            break;
        }
    }

    UTIL_Remove(pSprite);
}

//-----------------------------------------------------------------------------
// Purpose: Remove all sprites
//-----------------------------------------------------------------------------
void CGModSpritesSystem::RemoveAllSprites()
{
    for (int i = 0; i < s_SpriteInstances.Count(); i++)
    {
        if (s_SpriteInstances[i].bActive && s_SpriteInstances[i].pSprite)
        {
            UTIL_Remove(s_SpriteInstances[i].pSprite);
        }
    }

    s_SpriteInstances.Purge();
    DevMsg("Removed all sprites\n");
}

//-----------------------------------------------------------------------------
// Purpose: Create sprite instance
//-----------------------------------------------------------------------------
int CGModSpritesSystem::CreateSpriteInstance(const char* pszSpriteName, const Vector& origin)
{
    CSprite* pSprite = CreateSprite(pszSpriteName, origin);
    if (!pSprite)
        return -1;

    SpriteInstance_t instance;
    instance.pSprite = pSprite;
    instance.flCreationTime = gpGlobals->curtime;
    instance.bActive = true;
    instance.iInstanceId = s_iNextInstanceId++;

    SpriteData_t* pConfig = FindSpriteData(pszSpriteName);
    if (pConfig)
    {
        instance.config = *pConfig;
    }

    s_SpriteInstances.AddToTail(instance);
    return instance.iInstanceId;
}

//-----------------------------------------------------------------------------
// Purpose: Destroy sprite instance
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::DestroySpriteInstance(int iInstanceId)
{
    for (int i = 0; i < s_SpriteInstances.Count(); i++)
    {
        if (s_SpriteInstances[i].iInstanceId == iInstanceId && s_SpriteInstances[i].bActive)
        {
            if (s_SpriteInstances[i].pSprite)
            {
                UTIL_Remove(s_SpriteInstances[i].pSprite);
            }

            s_SpriteInstances.Remove(i);
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
// Purpose: Find sprite instance
//-----------------------------------------------------------------------------
SpriteInstance_t* CGModSpritesSystem::FindSpriteInstance(int iInstanceId)
{
    for (int i = 0; i < s_SpriteInstances.Count(); i++)
    {
        if (s_SpriteInstances[i].iInstanceId == iInstanceId && s_SpriteInstances[i].bActive)
        {
            return &s_SpriteInstances[i];
        }
    }

    return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Update sprite instances
//-----------------------------------------------------------------------------
void CGModSpritesSystem::UpdateSpriteInstances()
{
    for (int i = s_SpriteInstances.Count() - 1; i >= 0; i--)
    {
        SpriteInstance_t& instance = s_SpriteInstances[i];

        if (!instance.bActive || !instance.pSprite)
        {
            s_SpriteInstances.Remove(i);
            continue;
        }

        // Check if sprite entity still exists
        if (instance.pSprite->IsMarkedForDeletion())
        {
            instance.bActive = false;
            instance.pSprite = NULL;
            s_SpriteInstances.Remove(i);
            continue;
        }

        // Update sprite properties if needed
        // This could include animation updates, distance checks, etc.
    }
}

//-----------------------------------------------------------------------------
// Purpose: Clean up expired sprites
//-----------------------------------------------------------------------------
void CGModSpritesSystem::CleanupExpiredSprites()
{
    float currentTime = gpGlobals->curtime;

    for (int i = s_SpriteInstances.Count() - 1; i >= 0; i--)
    {
        SpriteInstance_t& instance = s_SpriteInstances[i];

        if (!instance.bActive)
        {
            s_SpriteInstances.Remove(i);
            continue;
        }

        // Check lifetime
        if (instance.config.flLifeTime > 0.0f)
        {
            if (currentTime - instance.flCreationTime >= instance.config.flLifeTime)
            {
                if (instance.pSprite)
                {
                    UTIL_Remove(instance.pSprite);
                }
                s_SpriteInstances.Remove(i);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Set sprite scale
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::SetSpriteScale(CSprite* pSprite, float flScale)
{
    if (!pSprite)
        return false;

    pSprite->SetScale(flScale);
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Set sprite color
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::SetSpriteColor(CSprite* pSprite, const Color& color)
{
    if (!pSprite)
        return false;

    pSprite->SetColor(color.r(), color.g(), color.b());
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Set sprite brightness
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::SetSpriteBrightness(CSprite* pSprite, float flBrightness)
{
    if (!pSprite)
        return false;

    pSprite->SetBrightness(flBrightness * 255.0f);
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Set sprite frame
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::SetSpriteFrame(CSprite* pSprite, int iFrame)
{
    if (!pSprite)
        return false;

    pSprite->SetFrame(iFrame);
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play sprite animation
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::PlaySpriteAnimation(CSprite* pSprite, bool bLoop)
{
    if (!pSprite)
        return false;

    if (bLoop)
    {
        pSprite->TurnOn();
    }
    else
    {
        pSprite->AnimateAndDie(10.0f);
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Create glow sprite
//-----------------------------------------------------------------------------
CSprite* CGModSpritesSystem::CreateGlowSprite(const Vector& origin, const Color& color, float flScale)
{
    CSprite* pSprite = CreateSprite("glow01", origin);
    if (pSprite)
    {
        SetSpriteColor(pSprite, color);
        SetSpriteScale(pSprite, flScale);
    }

    return pSprite;
}

//-----------------------------------------------------------------------------
// Purpose: Create laser dot
//-----------------------------------------------------------------------------
CSprite* CGModSpritesSystem::CreateLaserDot(const Vector& origin, const Color& color)
{
    CSprite* pSprite = CreateSprite("laserdot", origin);
    if (pSprite)
    {
        SetSpriteColor(pSprite, color);
    }

    return pSprite;
}

//-----------------------------------------------------------------------------
// Purpose: Create light glow
//-----------------------------------------------------------------------------
CSprite* CGModSpritesSystem::CreateLightGlow(const Vector& origin, const Color& color, float flBrightness)
{
    CSprite* pSprite = CreateSprite("light_glow03", origin);
    if (pSprite)
    {
        SetSpriteColor(pSprite, color);
        SetSpriteBrightness(pSprite, flBrightness);
    }

    return pSprite;
}

//-----------------------------------------------------------------------------
// Purpose: Create explosion sprite
//-----------------------------------------------------------------------------
CSprite* CGModSpritesSystem::CreateExplosionSprite(const Vector& origin, float flScale)
{
    CSprite* pSprite = CreateSprite("WXplo1", origin);
    if (pSprite)
    {
        SetSpriteScale(pSprite, flScale);
    }

    return pSprite;
}

//-----------------------------------------------------------------------------
// Purpose: Get sprite config count
//-----------------------------------------------------------------------------
int CGModSpritesSystem::GetSpriteConfigCount()
{
    return s_SpriteConfigs.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Get active sprite count
//-----------------------------------------------------------------------------
int CGModSpritesSystem::GetActiveSpriteCount()
{
    int activeCount = 0;
    for (int i = 0; i < s_SpriteInstances.Count(); i++)
    {
        if (s_SpriteInstances[i].bActive)
            activeCount++;
    }

    return activeCount;
}

//-----------------------------------------------------------------------------
// Purpose: Get sprite list
//-----------------------------------------------------------------------------
void CGModSpritesSystem::GetSpriteList(CUtlVector<CUtlString>& spriteList)
{
    spriteList.Purge();

    for (int i = s_SpriteConfigs.First(); i != s_SpriteConfigs.InvalidIndex(); i = s_SpriteConfigs.Next(i))
    {
        spriteList.AddToTail(s_SpriteConfigs.GetElementName(i));
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get active sprite instances
//-----------------------------------------------------------------------------
void CGModSpritesSystem::GetActiveSpriteInstances(CUtlVector<int>& instanceList)
{
    instanceList.Purge();

    for (int i = 0; i < s_SpriteInstances.Count(); i++)
    {
        if (s_SpriteInstances[i].bActive)
        {
            instanceList.AddToTail(s_SpriteInstances[i].iInstanceId);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Check if sprite is configured
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::IsSpriteConfigured(const char* pszSpriteName)
{
    return FindSpriteData(pszSpriteName) != NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Check if sprite is enabled
//-----------------------------------------------------------------------------
bool CGModSpritesSystem::IsSpriteEnabled(const char* pszSpriteName)
{
    SpriteData_t* pSprite = FindSpriteData(pszSpriteName);
    return pSprite ? pSprite->bEnabled : false;
}

//-----------------------------------------------------------------------------
// Purpose: Get sprite type name
//-----------------------------------------------------------------------------
const char* CGModSpritesSystem::GetSpriteTypeName(SpriteType_t type)
{
    switch (type)
    {
        case SPRITE_TYPE_DEFAULT:   return "default";
        case SPRITE_TYPE_GLOW:      return "glow";
        case SPRITE_TYPE_LASER:     return "laser";
        case SPRITE_TYPE_LIGHT:     return "light";
        case SPRITE_TYPE_EFFECT:    return "effect";
        case SPRITE_TYPE_EXPLOSION: return "explosion";
        case SPRITE_TYPE_BEAM:      return "beam";
        case SPRITE_TYPE_PARTICLE:  return "particle";
        default:                    return "unknown";
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get render mode name
//-----------------------------------------------------------------------------
const char* CGModSpritesSystem::GetRenderModeName(SpriteRenderMode_t mode)
{
    switch (mode)
    {
        case SPRITE_RENDER_NORMAL:      return "normal";
        case SPRITE_RENDER_ADDITIVE:    return "additive";
        case SPRITE_RENDER_ALPHA_TEST:  return "alphatest";
        case SPRITE_RENDER_ALPHA_BLEND: return "alphablend";
        case SPRITE_RENDER_GLOW:        return "glow";
        default:                        return "unknown";
    }
}

//=============================================================================
// Console Commands
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: List all sprite configurations
//-----------------------------------------------------------------------------
void CMD_gmod_sprites_list(void)
{
    CUtlVector<CUtlString> spriteList;
    CGModSpritesSystem::GetSpriteList(spriteList);

    Msg("Sprite configurations (%d):\n", spriteList.Count());
    for (int i = 0; i < spriteList.Count(); i++)
    {
        SpriteData_t* pSprite = CGModSpritesSystem::FindSpriteData(spriteList[i].Get());
        if (pSprite)
        {
            Msg("  %-20s - %s (%s) [%s]\n",
                spriteList[i].Get(),
                pSprite->materialPath.Get(),
                CGModSpritesSystem::GetSpriteTypeName(pSprite->spriteType),
                pSprite->bEnabled ? "enabled" : "disabled");
        }
    }

    int activeCount = CGModSpritesSystem::GetActiveSpriteCount();
    if (activeCount > 0)
    {
        Msg("\nActive sprite instances: %d\n", activeCount);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Reload sprite configurations
//-----------------------------------------------------------------------------
void CMD_gmod_sprites_reload(void)
{
    if (CGModSpritesSystem::ReloadSprites())
    {
        Msg("Sprite configurations reloaded successfully\n");
    }
    else
    {
        Msg("Failed to reload sprite configurations\n");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Create sprite
//-----------------------------------------------------------------------------
void CMD_gmod_sprites_create(void)
{
    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_sprites_create <sprite_name>\n");
        return;
    }

    const char* pszSpriteName = engine->Cmd_Argv(1);

    // Get player's aim position
    Vector forward;
    pPlayer->EyeVectors(&forward);
    Vector origin = pPlayer->EyePosition() + forward * 100.0f;

    int instanceId = CGModSpritesSystem::CreateSpriteInstance(pszSpriteName, origin);
    if (instanceId != -1)
    {
        Msg("Created sprite '%s' (ID: %d)\n", pszSpriteName, instanceId);
    }
    else
    {
        Msg("Failed to create sprite '%s'\n", pszSpriteName);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Remove sprite
//-----------------------------------------------------------------------------
void CMD_gmod_sprites_remove(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_sprites_remove <instance_id>\n");
        return;
    }

    int instanceId = atoi(engine->Cmd_Argv(1));

    if (CGModSpritesSystem::DestroySpriteInstance(instanceId))
    {
        Msg("Removed sprite instance %d\n", instanceId);
    }
    else
    {
        Msg("Sprite instance %d not found\n", instanceId);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Remove all sprites
//-----------------------------------------------------------------------------
void CMD_gmod_sprites_clear(void)
{
    CGModSpritesSystem::RemoveAllSprites();
    Msg("All sprites removed\n");
}

//-----------------------------------------------------------------------------
// Purpose: Show sprite info
//-----------------------------------------------------------------------------
void CMD_gmod_sprites_info(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_sprites_info <sprite_name>\n");
        return;
    }

    const char* pszSpriteName = engine->Cmd_Argv(1);
    SpriteData_t* pSprite = CGModSpritesSystem::FindSpriteData(pszSpriteName);

    if (!pSprite)
    {
        Msg("Sprite not found: %s\n", pszSpriteName);
        return;
    }

    Msg("Sprite configuration for '%s':\n", pszSpriteName);
    Msg("  Material: %s\n", pSprite->materialPath.Get());
    Msg("  Type: %s\n", CGModSpritesSystem::GetSpriteTypeName(pSprite->spriteType));
    Msg("  Category: %s\n", pSprite->category.Get());
    Msg("  Scale: %.2f\n", pSprite->flScale);
    Msg("  Brightness: %.2f\n", pSprite->flBrightness);
    Msg("  Render Mode: %s\n", CGModSpritesSystem::GetRenderModeName(pSprite->renderMode));
    Msg("  Animated: %s\n", pSprite->bAnimated ? "yes" : "no");
    if (pSprite->bAnimated)
    {
        Msg("  Frame Rate: %.1f fps\n", pSprite->flFrameRate);
        Msg("  Frame Count: %d\n", pSprite->iFrameCount);
    }
    Msg("  Lifetime: %.1f seconds\n", pSprite->flLifeTime);
    Msg("  Enabled: %s\n", pSprite->bEnabled ? "yes" : "no");
}
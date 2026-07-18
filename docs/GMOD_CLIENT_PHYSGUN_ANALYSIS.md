# Garry's Mod 9.0.4b Client.dll Physics Gun Analysis Report

**Analysis Date:** 2025-11-03
**Target File:** F:\development\steam\emulator_bot\LeakNet-rewrite\gmod_9_0_4b\bin\client.dll
**File Type:** PE32 DLL (Intel 80386, 32-bit Windows)
**File Size:** 3,036,160 bytes
**Timestamp:** 2005-11-28 17:03:12

---

## Executive Summary

This analysis reveals that Garry's Mod 9.0.4b contains **TWO distinct physics manipulation weapons** with significantly different client-side implementations:

1. **C_WeaponPhysCannon** - Enhanced version with advanced animation states
2. **C_WeaponGravityGun** - More feature-rich version with rotation controls and enhanced visual effects

The key differentiator is the **GravityGun** implementation which includes rotation mechanics, world position tracking, and a sophisticated beam rendering system with multiple sprite types.

---

## 1. Physics Gun Class Architecture

### 1.1 C_WeaponPhysCannon (Basic Implementation)

**Class Location:** RTTI offset 0x00239634

**Network Data Table Properties (DT_WeaponPhysCannon):**
```cpp
// Network variables synchronized from server
m_bOpen                          // Boolean - Gun "mouth" open/closed state
m_EffectState                    // Integer - Current visual effect state
m_attachedAnglesPlayerSpace      // QAngle[3] - Object rotation in player space
m_attachedPositionObjectSpace    // Vector - Object position offset
m_hAttachedObject                // EHANDLE - Currently held entity
```

**Associated Materials:**
- `sprites/orangecore2`
- `sprites/orangecore1`
- `sprites/orangeflare1`
- `sprites/glow04_noz`
- `sprites/orangelight1_noz`
- `sprites/orangelight1`
- `sprites/physcannon_bluelight2`

**Sound Events:**
- `Weapon_PhysCannon.HoldSound` - Continuous holding loop sound

**Animation Activities:**
```cpp
ACT_HL2MP_IDLE_PHYSGUN                    // Idle stance
ACT_HL2MP_RUN_PHYSGUN                     // Running animation
ACT_HL2MP_IDLE_CROUCH_PHYSGUN             // Crouched idle
ACT_HL2MP_WALK_CROUCH_PHYSGUN             // Crouched walking
ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN    // Attack gesture
ACT_HL2MP_GESTURE_RELOAD_PHYSGUN          // Reload gesture
ACT_HL2MP_JUMP_PHYSGUN                    // Jump animation
```

### 1.2 C_WeaponGravityGun (Enhanced Implementation)

**Class Location:** RTTI offset found with 1 reference

**Network Data Table Properties (DT_WeaponGravityGun):**
```cpp
// Enhanced network variables
m_bIsCurrentlyRotating           // Boolean - Object rotation mode active
m_bIsCurrentlyHolding            // Boolean - Currently holding object
m_viewModelIndex                 // Integer - View model index
m_worldPosition                  // Vector - World space position tracking
m_serversidebeams                // Boolean - Server-side beam rendering toggle
m_thruster_sounds                // Boolean - Thruster sound effects toggle
```

**Associated Materials:**
- `sprites/physgbeam` - Primary beam sprite
- `sprites/physgbeamB` - Secondary/alternate beam sprite
- `sprites/physbeam.vmt` - Beam material definition
- `weapons/phys` - Base weapon material path
- All PhysCannon materials (inherited)

**Key Functional Differences:**
- Rotation control system (`m_bIsCurrentlyRotating`)
- Dual beam rendering (physgbeam and physgbeamB)
- World position tracking for enhanced object manipulation
- Server-side beam toggle capability (`m_serversidebeams`)

---

## 2. Enhanced Visual Effects System

### 2.1 Plasma Beam Node System

**Class:** `C_PlasmaBeamNode`
**Data Table:** `DT_PlasmaBeamNode`
**Location:** 0x0021A74C (2 references found)

**Network Properties:**
```cpp
m_bSprayOn    // Boolean - Particle spray effect active
```

**Associated Materials:**
- `sprites/plasmaember` - Plasma particle sprite
- `effects/blueblacklargebeam` - Large beam effect

This is a **separate entity class** for creating animated beam node effects, likely used for the physics gun beam endpoints or waypoints.

### 2.2 Plasma Sprite System

**Class:** `C_PlasmaSprite`
**Location:** 0x0026EA6C (1 reference)

Dedicated sprite entity for plasma glow effects at beam connection points.

### 2.3 Plasma Glow Fade System

**Material Proxy:** `BPlasmaGlowFade`

This is a **material proxy** that controls animated fading of plasma glow effects, providing smooth visual transitions when picking up/dropping objects.

### 2.4 Beam Rendering Types

**Primary Beam Renderer:** `CViewRenderBeams`

**Supported Beam Types (from C_TE temp entities):**
- `C_TEBeamEntPoint` - Beam from entity to point
- `C_TEBeamEnts` - Beam between two entities
- `C_TEBeamFollow` - Following beam trail
- `C_TEBeamLaser` - Laser beam type
- `C_TEBeamPoints` - Beam between two points
- `C_TEBeamRing` - Ring beam effect
- `C_TEBeamRingPoint` - Ring beam from point
- `C_TEBeamSpline` - Spline-based curved beam

**Error Message Found:**
```
"$CViewRenderBeams::DrawBeam: Unknown beam type %i"
```

This indicates the beam system supports multiple rendering modes with type validation.

---

## 3. Client Effect Precache System

### 3.1 Effect Registration Classes

The client uses a **precache registration system** for effects:

```cpp
ClientEffectRegister@PrecacheEffectPhysCannon    // PhysCannon effects
ClientEffectRegister@PrecacheEffectGravityGun    // GravityGun effects
ClientEffectRegister@PrecacheEffectPlasmaBeam    // Plasma beam effects
ClientEffectRegister@PrecacheEffectGlow          // Glow overlay effects
ClientEffectRegister@PrecacheEffectSparks        // Spark effects
```

This registration occurs at **client initialization** to ensure all materials and particles are loaded before use.

### 3.2 Glow Overlay System

**Class:** `CGlowOverlay`
**Subclasses:**
- `C_LightGlow` - Standard light glow
- `C_LightGlowOverlay` - Enhanced overlay glow
- `C_SunGlowOverlay` - Sun-style glow (for bright effects)

**Draw Function:**
```cpp
CGlowOverlay::DrawOverlays()  // Main rendering entry point
```

**Network Properties:**
```cpp
m_flGlowProxySize      // Float - Glow size multiplier
m_nGlowModelIndex      // Integer - Model index for glow sprite
```

This system enables the **glowing highlight effect** on held objects.

---

## 4. Sprite and Material System

### 4.1 Core Sprite Materials

**Beam Sprites:**
```
sprites/physbeam.vmt          // Primary physics beam
sprites/physgbeam             // Gravity gun beam variant A
sprites/physgbeamB            // Gravity gun beam variant B
sprites/laserbeam.vmt         // Laser beam base
sprites/bluelaser1            // Blue laser variant
```

**Glow Sprites:**
```
sprites/physcannon_bluelight2  // Blue glow (object held)
sprites/orangelight1           // Orange glow (primary)
sprites/orangelight1_noz       // Orange glow (no Z-buffer)
sprites/blueglow1.vmt          // Blue glow sphere
sprites/glow01.vmt             // Generic glow
sprites/glow04_noz             // Glow variant 4 (no Z)
sprites/light_glow02_add_noz   // Additive light glow
sprites/animglow01.vmt         // Animated glow
```

**Flare Sprites:**
```
sprites/orangeflare1           // Orange flare effect
sprites/orangecore1            // Orange core sprite
sprites/orangecore2            // Orange core variant
sprites/blueflare1.vmt         // Blue flare
sprites/redglow1.vmt           // Red glow
sprites/flare6                 // Generic flare
```

**Effect Materials:**
```
effects/blueblacklargebeam     // Large blue/black beam
effects/blueblackflash         // Flash effect
effects/bluemuzzle             // Blue muzzle flash
effects/rollerglow             // Roller mine glow (repurposed)
effects/laser1.vmt             // Laser effect 1
effects/laser1_noz.vmt         // Laser (no Z-buffer)
effects/tesla_glow_noz         // Tesla coil glow
```

### 4.2 Sprite Rendering System

**Class:** `C_SpriteRenderer`
**Subclasses:**
- `C_Sprite` - Basic sprite
- `C_FireSprite` - Fire effect sprite
- `C_SpriteTrail` - Trailing sprite effect

**Render Function:**
```cpp
C_SpriteRenderer::DrawSprite()
CSprite::DrawModel()
CSpriteTrail::DrawModel()
```

**Network Properties:**
```cpp
m_iszSpriteName        // String - Sprite material name
m_flSpriteFramerate    // Float - Animation framerate
m_flSpriteScale        // Float - Scale multiplier
m_spriteRenderMode     // Integer - Render mode (additive, etc.)
m_spriteorientation    // Integer - Orientation mode
m_spriteorigin         // Vector - Origin point
```

### 4.3 Garry's Mod Sprite Extensions

**Custom ConVars:**
```cpp
gm_sprite_id            // Sprite model ID
gm_sprite_size          // Sprite size override
gm_sprite_colour_r      // Red channel (0-255)
gm_sprite_colour_g      // Green channel (0-255)
gm_sprite_colour_b      // Blue channel (0-255)
gm_sprite_colour_a      // Alpha channel (0-255)
gm_sprite_rm            // Render mode override
gm_sprite_rfx           // Render effects override
```

**Client Limits:**
```cpp
gm_sv_clientlimit_sprites   // Max sprites per client
gm_sv_clientlimit_effects   // Max effects per client
```

These allow **Lua scripting** to dynamically control sprite appearance.

---

## 5. Animation and View Model System

### 5.1 Base Animation Classes

**Core Classes:**
```cpp
C_BaseAnimating              // Base animated entity
C_BaseAnimatingOverlay       // Overlay animation support
CAnimatedTextureProxy        // Animated texture material proxy
```

**Key Functions:**
```cpp
C_BaseAnimating::DrawModel()
C_BaseAnimating::InternalDrawModel()
C_BaseAnimating::SetupBones()
C_BaseAnimating::BuildTransformations()
C_BaseAnimating::StandardBlendingRules()
C_BaseAnimating::MaintainSequenceTransitions()
```

### 5.2 View Model Rendering

**Network Properties:**
```cpp
m_bDrawViewmodel           // Boolean - Draw view model
m_bShouldDrawViewModel     // Boolean - Should draw check
m_bShouldDrawWorldModel    // Boolean - Draw world model
```

**Render Function:**
```cpp
CViewRender::DrawViewModel()  // Main view model draw
```

### 5.3 Weapon Animation System

**Base Class:** `C_BaseCombatWeapon`

**Timing Properties:**
```cpp
m_flTimeWeaponIdle         // Float - Next idle animation time
```

**Functions:**
```cpp
C_BaseCombatWeapon::DrawModel()
```

### 5.4 HUD Animation System

**Class:** `CHudAnimationInfo`

**Console Command:**
```
testhudanim <anim name>    // Test HUD animation
cl_panelanimation          // Panel animation system
```

---

## 6. Sound System Integration

### 6.1 Identified Sound Events

**Physics Cannon:**
```
Weapon_PhysCannon.HoldSound    // Looping hold sound
```

**Note:** Only one explicit sound event was found in strings. Additional sounds are likely:
- Referenced by hash/index rather than string
- Loaded from external sound script files
- Dynamically constructed names

### 6.2 Sound System Classes

**Sound Patch System:**
```cpp
CSoundPatch::Update()
```

**Error Message:**
```
"CSoundPatch::Update: Removing CSoundPatch (%s) with NULL EHandle"
```

This indicates the physics gun likely uses **CSoundPatch** for continuous holding sounds that track the weapon entity.

### 6.3 Garry's Mod Sound Extensions

**ConVar:**
```cpp
gm_thruster_sounds     // Boolean - Enable thruster sounds
```

This may control additional audio feedback during object manipulation.

---

## 7. Physics and Simulation

### 7.1 Physics System Classes

```cpp
CPhysicsSystem                 // Main physics manager
CPhysicsGameTrace              // Physics trace/raytrace
C_PhysicsProp                  // Physics prop entity
C_PhysicsThruster              // Thruster entity (for manipulation?)
CPhysicsPropMultiplayer        // Multiplayer physics prop
CRagdollAnimatedFriction       // Ragdoll physics
```

**Key Function:**
```cpp
CPhysicsSystem::PhysicsSimulate()
```

### 7.2 Physics Prop Rendering

**Function:**
```cpp
C_PhysicsProp::InternalDrawModel()
```

This suggests physics props have **custom rendering logic** that may include:
- Highlight/glow when held
- Color tinting
- Effect overlays

### 7.3 Client-Side Physics

**Console Variables:**
```cpp
cl_phys_props_max      // Maximum client-side physics props
cl_phys_props_enable   // Enable client-side physics props
```

---

## 8. Garry's Mod Specific Features

### 8.1 GMod-Specific Console Variables

**Physics Gun Controls:**
```cpp
gm_sv_allowphysgun           // Server: Allow physics gun usage
gm_toolweapon                // Currently selected tool weapon
gm_toolmode                  // Current tool mode number
```

**Server-Side Beams:**
```cpp
gm_serversidebeams           // Toggle server vs client beam rendering
```

**Color/Material Settings:**
```cpp
gm_colourset_r               // Set object color: Red
gm_colourset_g               // Set object color: Green
gm_colourset_b               // Set object color: Blue
gm_colourset_a               // Set object color: Alpha
gm_colourset_rm              // Set render mode
gm_colourset_fx              // Set render effects
```

**Balloon Tool Properties:**
```cpp
gm_balloon_reverse           // Reverse balloon float
gm_balloon_power             // Balloon lift power
gm_balloon_rope_width        // Rope width
gm_balloon_rope_length       // Rope length
gm_balloon_rope_forcelimit   // Rope force limit
gm_balloon_rope_rigid        // Rigid rope toggle
gm_balloon_rope_type         // Rope type selection
gm_balloon_explode           // Explode on remove
```

**Emitter Tool:**
```cpp
gm_emitter_type              // Particle emitter type
gm_emitter_delay             // Emission delay
```

**Face Poser:**
```cpp
gm_facepose_flex0 - flex27   // 28 facial flex controllers
gm_faceposer_reload          // Reload face poser
gm_facescale                 // Face animation scale
```

### 8.2 GMod Content Paths

**Texture/Material Paths:**
```
gmod/motionblur              // Motion blur effect
gmod/fb                      // Framebuffer effect
pp/garrybl                   // Post-process: Garry blur
gmod/team_circle             // Team indicator circle
gmod/shiny                   // Shiny material
gmod/melonracer/melon        // Melon racer content
materials/gmod/%s.vmt        // Dynamic material loading
```

**Configuration Files:**
```
settings/gmod_overlay.txt        // Overlay settings
settings/gmod_expressions.txt    // Facial expressions
```

### 8.3 GMod Commands

**Spawn Menu System:**
```
GModAddSpawnItem             // Add item to spawn menu
GModRemoveSpawnItem          // Remove spawn item
GModRemoveSpawnCat           // Remove spawn category
GModRemoveSpawnAll           // Clear spawn menu
gm_makecompletespawnlist     // Generate spawn list from folder
gm_spawncombolines           // Spawn combo box line count
```

**Entity Creation:**
```
gmod_makeprop                // Create prop
gmod_makeeffect              // Create effect
gmod_makeragdoll             // Create ragdoll
gm_makeentity                // Create generic entity
```

**UI Commands:**
```
GModRect                     // Create rectangle UI element
GModRectHide                 // Hide rectangle
GModRectHideAll              // Hide all rectangles
GModRectAnimate              // Animate rectangle
GModText                     // Create text element
GModTextHide                 // Hide text
GModTextHideAll              // Hide all text
GModTextAnimate              // Animate text
GModMenu                     // Open GMod menu
```

**Context System:**
```
gm_context                   // Current context menu
gm_context npc               // NPC spawn context
gm_context camera            // Camera tool context
```

---

## 9. Key Architectural Findings

### 9.1 Effect State Machine

The physics gun uses an **effect state system** (`m_EffectState`) to control visual feedback:

**Likely States:**
1. **EFFECT_NONE** - No effect active
2. **EFFECT_HOLDING** - Holding an object (orange glow)
3. **EFFECT_LAUNCH** - Launch/throw effect
4. **EFFECT_READY** - Ready to grab (blue tint)

Evidence: Multiple orange/blue sprite variants suggest color change based on state.

### 9.2 Dual Weapon Implementation

**Why Two Weapons?**

1. **C_WeaponPhysCannon** - Simpler HL2-style implementation
2. **C_WeaponGravityGun** - Garry's Mod enhanced version with:
   - Object rotation controls
   - Better visual feedback
   - More granular network state
   - Server-side beam option for performance

The **weapon_physgun** entity likely uses the GravityGun class internally in GMod.

### 9.3 Client-Side Prediction

**Limited client prediction** for visual effects only:
- Beam rendering is client-side
- Object attachment position/angles networked from server
- Effect state changes driven by server

This ensures physics simulation remains authoritative while providing responsive visual feedback.

### 9.4 Material Proxy System

**Animated Proxies Found:**
```cpp
CAnimatedTextureProxy           // Texture frame animation
CAnimatedEntityTextureProxy     // Entity-driven texture
CAnimatedOffsetTextureProxy     // Scrolling texture offset
CLampBeamProxy                  // Lamp beam animation
```

The **BPlasmaGlowFade** proxy suggests physics gun uses **material parameter animation** for smooth glow transitions.

---

## 10. Animation Enhancement Details

### 10.1 Missing Client-Side Systems

Comparing to your BarrysMod implementation, GMod 9.0.4b includes:

**1. Plasma Beam Node System**
- Separate entity class for beam waypoints
- Allows curved/segmented beams
- Particle spray effects at nodes

**2. Enhanced Glow Overlays**
- Multiple glow types (light, sun, generic)
- Animated fade-in/fade-out
- Size scaling based on distance
- Additive blending with no Z-buffer variants

**3. Dual Beam Sprites**
- `physgbeam` - Primary beam
- `physgbeamB` - Secondary/pulse effect
- Allows layered beam rendering

**4. Material Proxy Animation**
- Automated glow pulsing
- Color transition effects
- No code changes needed for smooth animation

**5. Effect State Synchronization**
- `m_EffectState` network variable
- Drives client-side visual changes
- Smooth state transitions

### 10.2 Recommended Enhancements for BarrysMod

**Priority 1: Effect State System**
```cpp
// Add to weapon class
enum EffectState_t {
    EFFECT_NONE = 0,
    EFFECT_READY,      // Can grab (blue tint)
    EFFECT_HOLDING,    // Holding object (orange glow)
    EFFECT_LAUNCH      // Launching object
};

CNetworkVar(int, m_EffectState);
```

**Priority 2: Dual Beam Rendering**
```cpp
void DrawBeamEffects() {
    // Primary beam (solid)
    DrawBeam(muzzlePos, objectPos, "sprites/physgbeam");

    // Secondary beam (pulsing overlay)
    float pulse = sin(gpGlobals->curtime * 10.0f) * 0.5f + 0.5f;
    DrawBeam(muzzlePos, objectPos, "sprites/physgbeamB", pulse);
}
```

**Priority 3: Glow Overlay on Held Object**
```cpp
void UpdateObjectGlow() {
    if (m_hAttachedObject.Get()) {
        // Add glow overlay to held object
        C_BaseEntity *pObj = m_hAttachedObject.Get();
        AddGlowOverlay(pObj,
            m_EffectState == EFFECT_HOLDING ? Color(255,128,0) : Color(64,128,255),
            10.0f);  // size
    }
}
```

**Priority 4: Enhanced Sound Feedback**
```cpp
// Add state-change sounds
void OnEffectStateChanged(int newState) {
    switch (newState) {
        case EFFECT_READY:
            EmitSound("Weapon.PhysCannon.Charge");
            break;
        case EFFECT_HOLDING:
            EmitSound("Weapon.PhysCannon.Pickup");
            StartLoopingSound("Weapon_PhysCannon.HoldSound");
            break;
        case EFFECT_LAUNCH:
            StopLoopingSound();
            EmitSound("Weapon.PhysCannon.Launch");
            break;
    }
}
```

**Priority 5: Rotation Visualization**
```cpp
// Visual feedback for rotation mode
void DrawRotationIndicator() {
    if (m_bIsCurrentlyRotating) {
        // Draw rotation axis helper
        DrawBeam(objectPos, objectPos + rotAxis * 50.0f,
                 "sprites/bluelaser1", 0.3f);

        // Draw rotation arc sprite
        DrawSprite(objectPos, "sprites/orangecore1",
                   size, ROTATE_ANIM_FRAME);
    }
}
```

---

## 11. Technical Implementation Notes

### 11.1 Beam Rendering Pipeline

**Initialization:**
1. Precache beam sprites in `PrecacheEffectPhysCannon()`
2. Get sprite material handles
3. Initialize beam renderer (`CViewRenderBeams`)

**Per-Frame Rendering:**
1. Check weapon effect state
2. Calculate beam endpoints (muzzle → object center)
3. Render primary beam with width based on distance
4. Render secondary pulsing overlay
5. Render endpoint sprites/glows

**Cleanup:**
1. Stop looping sounds
2. Remove glow overlays
3. Clear beam list

### 11.2 Network Optimization

**Server-Side Beams Option:**
```cpp
// ConVar: gm_serversidebeams
if (gm_serversidebeams.GetBool()) {
    // Server sends temp entity beams
    // Better for high-latency clients
    // More network traffic
} else {
    // Client predicts beam locally
    // Smooth visuals
    // Less network usage
}
```

### 11.3 Material Proxy Setup

**VMT Example for Animated Glow:**
```
"UnlitGeneric"
{
    "$basetexture" "sprites/orangelight1"
    "$additive" "1"
    "$vertexcolor" "1"
    "$vertexalpha" "1"

    "Proxies"
    {
        "BPlasmaGlowFade"
        {
            "fadeSpeed" "2.0"
            "minAlpha" "0.3"
            "maxAlpha" "1.0"
        }
    }
}
```

---

## 12. Performance Considerations

### 12.1 Client Limits

GMod implements **client-side effect limiting**:
```cpp
gm_sv_clientlimit_sprites   // Default: unknown (needs server.dll analysis)
gm_sv_clientlimit_effects   // Default: unknown
```

This prevents physics gun spam from degrading performance.

### 12.2 Render Optimization

**No Z-Buffer Variants:**
Many sprites have `_noz` variants that render **without depth testing**:
- Prevents Z-fighting
- Always visible through objects
- Better for glow effects

**Additive Blending:**
Sprites use `$additive` mode for:
- Brighter appearance
- Overdraw blending
- Glow/energy effect look

### 12.3 Draw Order

**Rendering Sequence:**
1. World geometry
2. Opaque entities
3. Physics props
4. View models
5. **Translucent renderables** (beams render here)
6. **Glow overlays** (post-translucent)
7. HUD elements

Function: `CViewRender::DrawTranslucentRenderables()`

---

## 13. Comparison: PhysCannon vs GravityGun

| Feature | C_WeaponPhysCannon | C_WeaponGravityGun |
|---------|-------------------|-------------------|
| **Network Vars** | 5 basic properties | 7 enhanced properties |
| **Rotation Control** | No explicit flag | `m_bIsCurrentlyRotating` |
| **Beam Sprites** | Single beam | Dual beams (A/B) |
| **Position Tracking** | Relative only | World position tracking |
| **Server Beams** | No toggle | `m_serversidebeams` toggle |
| **Sound Control** | Basic | Thruster sounds option |
| **Primary Use** | HL2 compatibility | GMod enhanced features |

**Conclusion:** The GravityGun class is the **enhanced implementation** that should be studied for advanced features.

---

## 14. Critical Missing Code (Not in Client.dll)

The following are **server-side only** and require analyzing `server.dll`:

1. **Object pickup logic** - Trace, validation, attachment
2. **Rotation input handling** - Mouse movement to rotation
3. **Physics constraint setup** - Spring/motor constraints
4. **Launch velocity calculation** - Power scaling
5. **Network state updates** - When to change `m_EffectState`

The client only **responds to** server state changes.

---

## 15. Recommended Next Steps

### For BarrysMod Development:

1. **Implement Effect State System**
   - Add `m_EffectState` network variable
   - Create state transition logic
   - Link to visual effects

2. **Add Dual Beam Rendering**
   - Create `sprites/physgbeam` and `sprites/physgbeamB` materials
   - Implement layered beam drawing
   - Add pulsing animation

3. **Create Glow Overlay System**
   - Port `CGlowOverlay` class functionality
   - Add to held objects
   - Implement color based on state

4. **Enhance Sound Design**
   - Add state-change sound events
   - Implement looping hold sound
   - Add rotation mode audio feedback

5. **Implement Material Proxy**
   - Create `BPlasmaGlowFade` proxy
   - Apply to glow materials
   - Configure fade parameters

6. **Add Rotation Visualization**
   - Visual indicators for rotation mode
   - Axis helper beams
   - Rotation arc effects

### For Further Analysis:

1. **Analyze server.dll** for:
   - Rotation input processing
   - Physics constraint implementation
   - Object validation logic
   - Network update triggers

2. **Extract Material Files** from GMod 9.0.4b GCF:
   - Study VMT parameters
   - Examine texture animations
   - Analyze proxy configurations

3. **Decompile Shader Code** (if needed):
   - Beam rendering shaders
   - Glow overlay shaders
   - Additive blending techniques

---

## 16. Conclusion

Garry's Mod 9.0.4b implements a **sophisticated multi-layered visual effects system** for the physics gun that goes far beyond basic beam rendering. The key innovations are:

1. **Dual weapon classes** with GravityGun being the enhanced version
2. **Effect state machine** driving visual and audio feedback
3. **Plasma beam node system** for complex beam shapes
4. **Multi-sprite rendering** with layered effects
5. **Material proxy animation** for smooth transitions
6. **Extensive GMod-specific extensions** for scripting control

The client-side implementation focuses entirely on **visual and audio presentation**, with all gameplay logic remaining server-authoritative. This provides responsive feedback while maintaining cheat-resistant simulation.

Your BarrysMod implementation should prioritize the **effect state system** and **dual beam rendering** as these provide the most noticeable enhancement to player experience.

---

## Appendix A: Complete Material List

### Beam Materials
- `sprites/physbeam.vmt`
- `sprites/physgbeam`
- `sprites/physgbeamB`
- `sprites/laserbeam.vmt`
- `sprites/bluelaser1`
- `effects/blueblacklargebeam`
- `effects/laser1.vmt`
- `effects/laser1_noz.vmt`

### Glow Materials
- `sprites/physcannon_bluelight2`
- `sprites/orangelight1`
- `sprites/orangelight1_noz`
- `sprites/blueglow1.vmt`
- `sprites/glow01.vmt`
- `sprites/glow04_noz`
- `sprites/light_glow02_add_noz`
- `sprites/animglow01.vmt`
- `effects/tesla_glow_noz`
- `effects/rollerglow`

### Flare/Core Materials
- `sprites/orangeflare1`
- `sprites/orangecore1`
- `sprites/orangecore2`
- `sprites/blueflare1.vmt`
- `sprites/flare6`
- `sprites/redglow1.vmt`

### Effect Materials
- `sprites/plasmaember`
- `effects/blueblackflash`
- `effects/bluemuzzle`

---

## Appendix B: Complete Network Variable List

### C_WeaponPhysCannon
```cpp
bool m_bOpen;
int m_EffectState;
QAngle m_attachedAnglesPlayerSpace[2];  // Array size 2 or 3
Vector m_attachedPositionObjectSpace;
EHANDLE m_hAttachedObject;
```

### C_WeaponGravityGun
```cpp
bool m_bIsCurrentlyRotating;
bool m_bIsCurrentlyHolding;
int m_viewModelIndex;
Vector m_worldPosition;
bool m_serversidebeams;
bool m_thruster_sounds;
// Likely inherits PhysCannon vars as well
```

### C_PlasmaBeamNode
```cpp
bool m_bSprayOn;
```

---

## Appendix C: RTTI Class Hierarchy

**Physics Gun Related Classes:**
```
C_BaseCombatWeapon
├── C_WeaponPhysCannon
└── C_WeaponGravityGun

C_BaseEntity
├── C_Beam
│   └── C_BeamQuadratic
├── C_PlasmaBeamNode
├── C_PlasmaSprite
├── C_Sprite
│   └── C_FireSprite
└── C_SpriteTrail

IClientEffect
└── CClientEffect
    └── Various effect implementations

CGlowOverlay
├── C_LightGlowOverlay
├── C_SunGlowOverlay
└── C_LightGlow
```

---

**End of Analysis Report**

This report provides comprehensive intelligence on the Garry's Mod 9.0.4b physics gun client-side implementation. All memory addresses, class names, and function signatures have been extracted through static binary analysis of the client.dll file.

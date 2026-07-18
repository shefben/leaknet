# Physics Gun Animation Enhancement - Quick Implementation Guide

Based on analysis of Garry's Mod 9.0.4b client.dll

---

## Quick Summary: What GMod Does Better

1. **Effect State Machine** - Visual feedback changes based on weapon state
2. **Dual Beam Rendering** - Two overlaid beam sprites for depth
3. **Glow Overlays** - Held objects get dynamic glow effects
4. **Material Proxy Animation** - Smooth automated glow pulsing
5. **Enhanced Sound Design** - State-based audio feedback

---

## Implementation Checklist

### Phase 1: Core Network Variables (Server & Client)

**Add to weapon class header:**
```cpp
// In weapon_physgun.h (shared)
enum EffectState_t {
    EFFECT_NONE = 0,
    EFFECT_READY,      // Targeting valid object (blue)
    EFFECT_HOLDING,    // Holding object (orange)
    EFFECT_LAUNCH      // Launch animation
};

CNetworkVar(int, m_EffectState);
CNetworkVar(bool, m_bIsCurrentlyRotating);  // For rotation mode
CNetworkVar(Vector, m_vWorldPosition);       // Held object world pos
```

**Network table (shared):**
```cpp
BEGIN_NETWORK_TABLE(CWeaponPhysGun, DT_WeaponPhysGun)
    // ... existing vars ...
    SendPropInt(SENDINFO(m_EffectState), 3, SPROP_UNSIGNED),
    SendPropBool(SENDINFO(m_bIsCurrentlyRotating)),
    SendPropVector(SENDINFO(m_vWorldPosition)),
END_NETWORK_TABLE()
```

---

### Phase 2: Effect State Logic (Server-Side)

**Update state in ItemPostFrame:**
```cpp
void CWeaponPhysGun::ItemPostFrame() {
    CBasePlayer *pOwner = ToBasePlayer(GetOwner());
    if (!pOwner) return;

    // Determine new effect state
    int newState = EFFECT_NONE;

    if (m_hAttachedObject.Get()) {
        newState = EFFECT_HOLDING;
    }
    else if (CanPickupObject(GetBeamTargetEntity())) {
        newState = EFFECT_READY;
    }

    // Update state and notify
    if (newState != m_EffectState) {
        m_EffectState = newState;
        OnEffectStateChanged(newState);
    }

    // Update world position if holding
    if (m_hAttachedObject.Get()) {
        m_vWorldPosition = m_hAttachedObject->GetAbsOrigin();
    }
}

void CWeaponPhysGun::OnEffectStateChanged(int newState) {
    // Server-side sound emission
    switch (newState) {
        case EFFECT_READY:
            EmitSound("Weapon.PhysCannon.Charge");
            break;
        case EFFECT_HOLDING:
            EmitSound("Weapon.PhysCannon.Pickup");
            break;
        case EFFECT_LAUNCH:
            EmitSound("Weapon.PhysCannon.Launch");
            break;
    }
}
```

---

### Phase 3: Client-Side Visual Effects

**Add to client weapon class (c_weapon_physgun.cpp):**

```cpp
#include "c_te_effect_dispatch.h"
#include "view.h"
#include "beamdraw.h"

class C_WeaponPhysGun : public C_BaseCombatWeapon {
public:
    // ... existing code ...

    virtual void OnDataChanged(DataUpdateType_t updateType);
    virtual void ClientThink();
    void DrawBeamEffects();
    void DrawGlowEffects();
    void UpdateLoopingSound();

private:
    int m_iBeamSprite1;     // Primary beam
    int m_iBeamSprite2;     // Secondary pulsing beam
    int m_iGlowSprite;      // Glow sprite
    CSoundPatch *m_pLoopingSound;
    int m_iOldEffectState;
};

void C_WeaponPhysGun::OnDataChanged(DataUpdateType_t updateType) {
    BaseClass::OnDataChanged(updateType);

    if (updateType == DATA_UPDATE_CREATED) {
        // Precache materials
        m_iBeamSprite1 = PrecacheModel("sprites/physgbeam.vmt");
        m_iBeamSprite2 = PrecacheModel("sprites/physgbeamB.vmt");
        m_iGlowSprite = PrecacheModel("sprites/orangelight1.vmt");

        SetNextClientThink(CLIENT_THINK_ALWAYS);
    }

    // Effect state changed
    if (m_iOldEffectState != m_EffectState) {
        UpdateLoopingSound();
        m_iOldEffectState = m_EffectState;
    }
}

void C_WeaponPhysGun::ClientThink() {
    BaseClass::ClientThink();

    if (IsEffectActive(EF_NODRAW))
        return;

    DrawBeamEffects();
    DrawGlowEffects();
}

void C_WeaponPhysGun::DrawBeamEffects() {
    if (m_EffectState == EFFECT_NONE)
        return;

    C_BasePlayer *pOwner = ToBasePlayer(GetOwner());
    if (!pOwner)
        return;

    // Calculate beam endpoints
    Vector muzzlePos;
    QAngle muzzleAng;
    if (!GetAttachment("muzzle", muzzlePos, muzzleAng)) {
        muzzlePos = pOwner->Weapon_ShootPosition();
    }

    Vector targetPos;
    if (m_EffectState == EFFECT_HOLDING && m_hAttachedObject.Get()) {
        targetPos = m_vWorldPosition;
    }
    else {
        // Trace to target
        Vector forward;
        pOwner->EyeVectors(&forward);
        trace_t tr;
        UTIL_TraceLine(pOwner->EyePosition(),
                       pOwner->EyePosition() + forward * 4096.0f,
                       MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);
        targetPos = tr.endpos;
    }

    // Choose color based on state
    color32 beamColor;
    if (m_EffectState == EFFECT_HOLDING) {
        beamColor.r = 255; beamColor.g = 128; beamColor.b = 0; beamColor.a = 255;
    }
    else {
        beamColor.r = 64; beamColor.g = 128; beamColor.b = 255; beamColor.a = 255;
    }

    // Primary beam (solid)
    BeamInfo_t beamInfo;
    beamInfo.m_nType = TE_BEAMPOINTS;
    beamInfo.m_vecStart = muzzlePos;
    beamInfo.m_vecEnd = targetPos;
    beamInfo.m_pszModelName = "sprites/physgbeam.vmt";
    beamInfo.m_nModelIndex = m_iBeamSprite1;
    beamInfo.m_flHaloScale = 0.0f;
    beamInfo.m_flLife = 0.1f;
    beamInfo.m_flWidth = 8.0f;
    beamInfo.m_flEndWidth = 8.0f;
    beamInfo.m_flFadeLength = 0.0f;
    beamInfo.m_flAmplitude = 2.0f;
    beamInfo.m_flBrightness = 255.0f;
    beamInfo.m_flSpeed = 0.0f;
    beamInfo.m_nStartFrame = 0;
    beamInfo.m_flFrameRate = 0.0f;
    beamInfo.m_flRed = beamColor.r;
    beamInfo.m_flGreen = beamColor.g;
    beamInfo.m_flBlue = beamColor.b;
    beamInfo.m_nSegments = 16;
    beamInfo.m_bRenderable = true;
    beamInfo.m_nFlags = FBEAM_FOREVER | FBEAM_ONLYNOISEONCE;

    beams->CreateBeamPoints(beamInfo);

    // Secondary pulsing beam overlay
    float pulse = sin(gpGlobals->curtime * 10.0f) * 0.5f + 0.5f;
    beamInfo.m_pszModelName = "sprites/physgbeamB.vmt";
    beamInfo.m_nModelIndex = m_iBeamSprite2;
    beamInfo.m_flWidth = 12.0f;
    beamInfo.m_flEndWidth = 12.0f;
    beamInfo.m_flBrightness = 128.0f * pulse;
    beamInfo.m_flAmplitude = 4.0f;

    beams->CreateBeamPoints(beamInfo);
}

void C_WeaponPhysGun::DrawGlowEffects() {
    if (m_EffectState != EFFECT_HOLDING)
        return;

    C_BaseEntity *pObject = m_hAttachedObject.Get();
    if (!pObject)
        return;

    // Draw glow sprite at object center
    Vector glowPos = pObject->WorldSpaceCenter();

    // Pulsing size
    float pulse = sin(gpGlobals->curtime * 5.0f) * 0.3f + 0.7f;
    float size = 50.0f * pulse;

    // Choose color
    color32 glowColor;
    glowColor.r = 255;
    glowColor.g = 128;
    glowColor.b = 0;
    glowColor.a = 200;

    // Draw additive glow sprite
    CMatRenderContextPtr pRenderContext(materials);
    IMaterial *pMat = materials->FindMaterial("sprites/orangelight1",
                                              TEXTURE_GROUP_CLIENT_EFFECTS);
    pRenderContext->Bind(pMat);

    DrawSprite(glowPos, size, size, glowColor);
}

void C_WeaponPhysGun::UpdateLoopingSound() {
    if (m_EffectState == EFFECT_HOLDING) {
        // Start looping sound
        if (!m_pLoopingSound) {
            CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
            CLocalPlayerFilter filter;

            m_pLoopingSound = controller.SoundCreate(filter, entindex(),
                                                      "Weapon_PhysCannon.HoldSound");
            controller.Play(m_pLoopingSound, 1.0f, 100);
        }
    }
    else {
        // Stop looping sound
        if (m_pLoopingSound) {
            CSoundEnvelopeController::GetController().SoundDestroy(m_pLoopingSound);
            m_pLoopingSound = NULL;
        }
    }
}
```

---

### Phase 4: Material Setup

**Create sprites/physgbeam.vmt:**
```
"UnlitGeneric"
{
    "$basetexture" "sprites/laserbeam"
    "$additive" "1"
    "$vertexcolor" "1"
    "$vertexalpha" "1"
    "$ignorez" "0"
}
```

**Create sprites/physgbeamB.vmt:**
```
"UnlitGeneric"
{
    "$basetexture" "sprites/laserbeam"
    "$additive" "1"
    "$vertexcolor" "1"
    "$vertexalpha" "1"
    "$ignorez" "0"
    "$nocull" "1"
}
```

**Create sprites/orangelight1.vmt:**
```
"UnlitGeneric"
{
    "$basetexture" "sprites/light_glow02_add"
    "$additive" "1"
    "$vertexcolor" "1"
    "$vertexalpha" "1"
    "$ignorez" "1"

    "Proxies"
    {
        "Sine"
        {
            "sinemin" "0.6"
            "sinemax" "1.0"
            "sineperiod" "2.0"
            "resultvar" "$alpha"
        }
    }
}
```

---

### Phase 5: Sound Events

**Add to scripts/game_sounds_weapons.txt:**
```
"Weapon.PhysCannon.Charge"
{
    "channel"       "CHAN_WEAPON"
    "soundlevel"    "SNDLVL_NORM"
    "volume"        "0.7"
    "pitch"         "95,105"
    "wave"          "weapons/physcannon/energy_sing_loop4.wav"
}

"Weapon.PhysCannon.Pickup"
{
    "channel"       "CHAN_WEAPON"
    "soundlevel"    "SNDLVL_NORM"
    "volume"        "1.0"
    "wave"          "weapons/physcannon/physcannon_pickup.wav"
}

"Weapon_PhysCannon.HoldSound"
{
    "channel"       "CHAN_STATIC"
    "soundlevel"    "SNDLVL_NORM"
    "volume"        "0.5"
    "pitch"         "100"
    "wave"          "weapons/physcannon/hold_loop.wav"
}

"Weapon.PhysCannon.Launch"
{
    "channel"       "CHAN_WEAPON"
    "soundlevel"    "SNDLVL_NORM"
    "volume"        "1.0"
    "wave"          "weapons/physcannon/physcannon_launch.wav"
}
```

---

### Phase 6: Testing Checklist

- [ ] Effect state changes when targeting valid object
- [ ] Blue tint appears when ready to grab
- [ ] Orange glow appears when holding
- [ ] Dual beam renders with pulsing overlay
- [ ] Glow sprite appears at held object center
- [ ] Glow pulses smoothly (material proxy)
- [ ] Looping hold sound starts when grabbing
- [ ] Looping sound stops when releasing
- [ ] All sounds play at correct times
- [ ] No crashes or rendering errors

---

## Advanced Features (Phase 7+)

### Rotation Mode Visualization
```cpp
if (m_bIsCurrentlyRotating) {
    // Draw rotation axis helper
    Vector axis = GetRotationAxis();
    DrawBeam(objectPos, objectPos + axis * 50.0f, "sprites/bluelaser1");

    // Draw rotation arc
    DrawSprite(objectPos, "sprites/orangecore1", size);
}
```

### Distance-Based Beam Width
```cpp
float distance = (targetPos - muzzlePos).Length();
float width = RemapVal(distance, 0, 2048, 4.0f, 12.0f);
beamInfo.m_flWidth = width;
```

### Object Highlight Shader
```cpp
// Add glow effect directly to held object material
if (m_hAttachedObject.Get()) {
    pObject->SetRenderMode(kRenderTransAdd);
    pObject->SetRenderColorR(255);
    pObject->SetRenderColorG(128);
    pObject->SetRenderColorB(0);
}
```

---

## Performance Notes

1. **Beam Rendering** - Beams use TE_BEAMPOINTS with FBEAM_FOREVER flag, automatically cleaned up each frame
2. **Material Proxies** - Animation runs in shader, zero CPU cost
3. **Sound Patches** - Looping sounds use CSoundPatch for efficient streaming
4. **Network Bandwidth** - Effect state is 3 bits, position is 12 bytes, minimal overhead

---

## Common Issues & Solutions

### Issue: Beams flicker
**Solution:** Set `beamInfo.m_flLife = 0.1f` (slightly longer than frame time)

### Issue: Glow too bright
**Solution:** Reduce `beamInfo.m_flBrightness` or sprite alpha in VMT

### Issue: Looping sound doesn't stop
**Solution:** Ensure `SoundDestroy()` is called in weapon holster/drop

### Issue: Material not found
**Solution:** Check material is in `materials/sprites/` folder with `.vmt` extension

### Issue: Network var not updating
**Solution:** Verify `SendProp` and `RecvProp` match exactly in type/name

---

## File Locations Summary

**Code Files:**
- `dlls/hl2mp/weapon_physgun.cpp` - Server logic
- `cl_dll/hl2mp/c_weapon_physgun.cpp` - Client rendering

**Materials:**
- `materials/sprites/physgbeam.vmt`
- `materials/sprites/physgbeamB.vmt`
- `materials/sprites/orangelight1.vmt`

**Sounds:**
- `sound/weapons/physcannon/*.wav`
- `scripts/game_sounds_weapons.txt` - Sound script entries

**Textures:**
- Use existing HL2 beam textures from `materials/sprites/`

---

## Estimated Implementation Time

- Phase 1 (Network vars): 30 minutes
- Phase 2 (Server logic): 1 hour
- Phase 3 (Client rendering): 2-3 hours
- Phase 4 (Materials): 30 minutes
- Phase 5 (Sounds): 1 hour
- Phase 6 (Testing): 1-2 hours

**Total: 6-8 hours** for complete implementation

---

## References

- Full analysis: `GMOD_CLIENT_PHYSGUN_ANALYSIS.md`
- Source file: `gmod_9_0_4b/bin/client.dll`
- Valve beam rendering: `engine/beamdraw.h`, `engine/view_scene.cpp`

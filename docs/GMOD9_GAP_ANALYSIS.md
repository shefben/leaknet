# GMod 9.0.4b vs BMod Gap Analysis

This document compares the functionality found in the original Garry's Mod 9.0.4b server.dll (via IDA Pro reverse engineering) against the current BMod implementation.

## Summary

| Category | GMod 9 Count | BMod Implemented | Gap |
|----------|--------------|------------------|-----|
| User Messages | 17 | 17 | 0 |
| CVars | 26 | ~15 | ~11 |
| Console Commands | 20 | ~12 | ~8 |
| Lua Functions | 170+ | ~150 | ~20 |
| Entity Classes | 10+ | ~5 | ~5 |

---

## User Messages (usermsg_*) - COMPLETE

All 17 user messages from GMod 9.0.4b are implemented in BMod:

| GMod 9 Function | Status | BMod Location |
|-----------------|--------|---------------|
| usermsg_GModVersion | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_GModText | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_GModTextHide | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_GModTextHideAll | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_GModTextAnimate | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_GModRect | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_GModRectAnimate | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_GModRectHide | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_GModRectHideAll | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_WQuad | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_WQuadHide | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_WQuadHideAll | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_WQuadAnimate | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_GModAddSpawnItem | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_GModRemoveSpawnCat | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_GModRemoveSpawnAll | ✅ Implemented | bmod_usermessages.cpp |
| usermsg_Spawn_SetCategory | ✅ Implemented | bmod_usermessages.cpp |

---

## CVars (CVR_*) - PARTIAL

Found 26 CVars in GMod 9.0.4b:

| GMod 9 CVar | Status | Notes |
|-------------|--------|-------|
| CVR_sv_hl2mp_weapon_respawn_time | ❓ Check | HL2MP inherited |
| CVR_sv_hl2mp_item_respawn_time | ❓ Check | HL2MP inherited |
| CVR_sv_fetchPlayerInfo | ❌ Missing | Player info fetch |
| CVR_plus_gm_thrust | ✅ Implemented | Thruster control |
| CVR_minus_gm_thrust | ✅ Implemented | Thruster control |
| CVR_plus_gm_wheelf | ✅ Implemented | Wheel forward |
| CVR_minus_gm_wheelf | ✅ Implemented | Wheel forward |
| CVR_plus_gm_wheelb | ✅ Implemented | Wheel backward |
| CVR_minus_gm_wheelb | ✅ Implemented | Wheel backward |
| CVR_gm_wheel_allon | ✅ Implemented | All wheels on |
| CVR_gm_wheel_alloff | ✅ Implemented | All wheels off |
| CVR_plus_gm_cam_static | ❌ Missing | Camera static |
| CVR_minus_gm_cam_static | ❌ Missing | Camera static |
| CVR_plus_gm_cam_prop | ❌ Missing | Camera prop |
| CVR_minus_gm_cam_prop | ❌ Missing | Camera prop |
| CVR_plus_gm_cam_view | ❌ Missing | Camera view |
| CVR_minus_gm_cam_view | ❌ Missing | Camera view |
| CVR_g_debug_physcannon | ❓ Check | Debug physcannon |
| CVR_gm_debug_pistoltracepos | ❌ Missing | Debug pistol trace |
| CVR_gm_make_propcam | ❌ Missing | Make prop camera |
| CVR_gm_dynamite_delay | ✅ Implemented | Dynamite delay |
| CVR_gm_dynamite_delay_add | ✅ Implemented | Dynamite delay add |
| CVR_gm_physprops_reload | ❌ Missing | Reload phys props |
| CVR_gm_sv_allowignite | ✅ Implemented | Allow ignite |
| CVR_gm_magnetstrength | ✅ Implemented | Magnet strength |
| CVR_gm_sv_allowlamps | ✅ Implemented | Allow lamps |

---

## Console Commands (CMD_*) - PARTIAL

Found 20 console commands in GMod 9.0.4b:

| GMod 9 Command | Status | Notes |
|----------------|--------|-------|
| CMD_gm_sv_setrules | ❓ Check | Server rules |
| CMD_bot_add | ✅ Implemented | HL2MP inherited |
| CMD_SetPlayerModel | ✅ Implemented | Player model |
| CMD_gmod_makeragdoll | ✅ Implemented | Make ragdoll |
| CMD_gmod_makeprop | ✅ Implemented | Make prop |
| CMD_gmod_undo | ✅ Implemented | Undo system |
| CMD_gmod_makeeffect | ✅ Implemented | Make effect |
| CMD_gm_remove_all | ✅ Implemented | Remove all |
| CMD_gm_remove_my | ✅ Implemented | Remove my props |
| CMD_gm_makeentity | ✅ Implemented | Make entity |
| CMD_SearchPaths | ❓ Check | Search paths |
| CMD_gm_explode | ✅ Implemented | Explode |
| CMD_gm_showhelp | ❌ Missing | Show help menu |
| CMD_gm_showteam | ❌ Missing | Show team menu |
| CMD_gm_showspare1 | ❌ Missing | Spare slot 1 |
| CMD_gm_showspare2 | ❌ Missing | Spare slot 2 |
| CMD_optionselect | ❌ Missing | Option select |
| CMD_Run_Lua_Command | ✅ Implemented | Run Lua |
| CMD_lua_openscript | ✅ Implemented | Open script |
| CMD_lua_listbinds | ✅ Implemented | List binds |

---

## Lua Functions - DETAILED ANALYSIS

### Player Functions (26 in GMod 9) - MOSTLY COMPLETE

| GMod 9 Function | Status | BMod Function |
|-----------------|--------|---------------|
| __PlayerSetSprint | ✅ | _PlayerSetSprint |
| __PlayerFreeze | ✅ | _PlayerFreeze |
| __PlayerGetShootPos | ✅ | _PlayerGetShootPos |
| __PlayerGetShootAng | ✅ | _PlayerGetShootAng |
| __PlayerGetActiveWeapon | ✅ | _PlayerGetActiveWeapon |
| __PlayerKill | ✅ | _PlayerKill |
| __PlayerGiveAmmo | ✅ | _PlayerGiveAmmo |
| __PlayerRespawn | ✅ | _PlayerRespawn |
| __PlayerSetDrawTeamCircle | ❌ Missing | Team circle drawing |
| __PlayerInfo | ✅ | _PlayerInfo |
| __PlayerChangeTeam | ✅ | _PlayerChangeTeam |
| __PlayerSetModel | ✅ | _PlayerSetModel |
| __PlayerGetRandomAllowedModel | ❌ Missing | Random allowed model |
| __PlayerSetHealth | ✅ | _PlayerSetHealth |
| __PlayerSetArmor | ✅ | _PlayerSetArmor |
| __PlayerSetMaxSpeed | ✅ | _PlayerSetMaxSpeed |
| __PlayerAddScore | ✅ | _PlayerAddScore |
| __PlayerSetScore | ✅ | _PlayerSetScore |
| __PlayerAddDeath | ✅ | _PlayerAddDeath |
| __PlayerGiveItem | ✅ | _PlayerGiveItem |
| __PlayerGiveSWEP | ✅ | _PlayerGiveSWEP |
| __PlayerRemoveAllWeapons | ✅ | _PlayerRemoveAllWeapons |
| __PlayerRemoveWeapon | ✅ | _PlayerRemoveWeapon |
| __PlayerAllowDecalPaint | ✅ | _PlayerAllowDecalPaint |

### Entity Functions (~53 in GMod 9) - COMPLETE

All entity functions are implemented in lua_entity_funcs.cpp.

### Physics Functions (14 in GMod 9) - MOSTLY COMPLETE

| GMod 9 Function | Status | BMod Function |
|-----------------|--------|---------------|
| __phys_EnableMotion | ✅ | _phys_EnableMotion |
| __phys_EnableDrag | ✅ | _phys_EnableDrag |
| __phys_EnableGravity | ✅ | _phys_EnableGravity |
| __phys_EnableCollitions | ✅ | _phys_EnableCollisions |
| __phys_GetMass | ✅ | _phys_GetMass |
| __phys_SetMass | ✅ | _phys_SetMass |
| __phys_Sleep | ✅ | _phys_Sleep |
| __phys_Wake | ✅ | _phys_Wake |
| __phys_IsAsleep | ✅ | _phys_IsAsleep |
| __phys_HasPhysics | ✅ | _phys_HasPhysics |
| __phys_ConstraintSetEnts | ❌ Missing | Constraint entities |
| __phys_ApplyForceCenter | ✅ | _phys_ApplyForceCenter |
| __phys_ApplyForceOffset | ✅ | _phys_ApplyForceOffset |
| __phys_ApplyTorqueCenter | ✅ | _phys_ApplyTorqueCenter |

### Trace Functions (15 in GMod 9) - MOSTLY COMPLETE

| GMod 9 Function | Status | BMod Function |
|-----------------|--------|---------------|
| __TraceAttack | ❌ Missing | Attack trace |
| __TraceSetCollisionGroup | ✅ | _TraceSetCollisionGroup |
| __TraceSetMask | ✅ | _TraceSetMask |
| __TraceLine | ✅ | _TraceLine |
| __TraceEndPos | ✅ | _TraceEndPos |
| __TraceFraction | ✅ | _TraceFraction |
| __TraceHitWorld | ✅ | _TraceHitWorld |
| __TraceHitNonWorld | ✅ | _TraceHitNonWorld |
| __TraceHit | ✅ | _TraceHit |
| __TraceGetEnt | ✅ | _TraceGetEnt |
| __TraceGetSurfaceNormal | ✅ | _TraceGetSurfaceNormal |
| __TraceDidHitSky | ✅ | _TraceDidHitSky |
| __TraceDidHitHitbox | ❌ Missing | Hitbox trace |
| __TraceGetTexture | ✅ | _TraceGetTexture |
| __TraceDidHitWater | ✅ | _TraceDidHitWater |

### Effect Functions (12 in GMod 9) - COMPLETE

All effect functions are implemented in lua_effect_funcs.cpp.

### SWEP Functions (7 in GMod 9) - PARTIAL

| GMod 9 Function | Status | BMod Function |
|-----------------|--------|---------------|
| __SWEPSetSound | ✅ | _SWEPSetSound |
| __SWEPUseAmmo | ❌ Missing | Use ammo |
| __SWEPUpdateVariables | ❌ Missing | Update vars |
| __SWEPRunString | ❌ Missing | Run string |
| __swep_GetClipAmmo | ❌ Missing | Get clip ammo |
| __swep_SetClipAmmo | ❌ Missing | Set clip ammo |
| __swep_GetDeathIcon | ❌ Missing | Death icon |

### Weapon Functions (7 in GMod 9) - PARTIAL

| GMod 9 Function | Status | Notes |
|-----------------|--------|-------|
| __WeaponSetModel | ❌ Missing | Set weapon model |
| __WeaponSetSlot | ❌ Missing | Set weapon slot |
| __WeaponSetSound | ❌ Missing | Set weapon sound |
| __WeaponSetFOV | ❌ Missing | Set weapon FOV |
| __WeaponFlipHands | ❌ Missing | Flip hands |
| __WeaponSetDamage | ❌ Missing | Set damage |
| __WeaponSetup | ❌ Missing | Setup weapon |

### Team Functions (5 in GMod 9) - PARTIAL

| GMod 9 Function | Status | BMod Function |
|-----------------|--------|---------------|
| __TeamAddScore | ✅ | _TeamSetScore (via lua_TeamSetScore) |
| __TeamSetScore | ✅ | lua_TeamSetScore |
| __TeamNumPlayers | ❌ Missing | Count team players |
| __TeamCount | ❌ Missing | Total team count |
| __TeamScore | ✅ | lua_TeamGetScore |

### File Functions (8 in GMod 9) - MISSING

| GMod 9 Function | Status | Notes |
|-----------------|--------|-------|
| __file_Exists | ❌ Missing | Check file exists |
| __file_Read | ❌ Missing | Read file |
| __file_Write | ❌ Missing | Write file |
| __file_CreateDir | ❌ Missing | Create directory |
| __file_IsDir | ❌ Missing | Check if directory |
| __file_Find | ❌ Missing | Find files |
| __file_Delete | ❌ Missing | Delete file |
| __file_Rename | ❌ Missing | Rename file |

### Game Event Functions (4 in GMod 9) - MISSING

| GMod 9 Function | Status | Notes |
|-----------------|--------|-------|
| __gameevent_start | ❌ Missing | Start game event |
| __gameevent_SetString | ❌ Missing | Set event string |
| __gameevent_SetInt | ❌ Missing | Set event int |
| __gameevent_Fire | ❌ Missing | Fire event |

### Misc Functions - MOSTLY COMPLETE

| GMod 9 Function | Status | BMod Function |
|-----------------|--------|---------------|
| __StartNextLevel | ❌ Missing | Start next level |
| __GetNextMap | ✅ | _GetNextMap |
| __GetCurrentMap | ✅ | _GetCurrentMap |
| __GetRule | ❌ Missing | Get game rule |
| __ServerCommand | ❌ Missing | Execute server cmd |
| __GameSetTargetIDRules | ❌ Missing | Target ID rules |
| __ForceFileConsistency | ❌ Missing | File consistency |
| __PluginMsg | ❌ Missing | Plugin message |
| __PluginText | ❌ Missing | Plugin text |
| __MakeDecal | ✅ | _MakeDecal |
| __RunString | ✅ | _RunString |
| __GetModPath | ✅ | _GetModPath |
| __IsLinux | ✅ | _IsLinux |
| __IsDedicatedServer | ✅ | _IsDedicatedServer |
| __SetDefaultRelationship | ❌ Missing | NPC relationship |
| __PlaySound | ❌ Missing | Play sound |
| __PlaySoundPlayer | ❌ Missing | Play sound to player |
| __CurTime | ✅ | _CurTime |
| __MaxPlayers | ✅ | lua_MaxPlayers |
| __GetConVar_Float | ✅ | _GetConVar_Float |
| __GetConVar_String | ✅ | _GetConVar_String |
| __GetConVar_Bool | ❌ Missing | Get bool cvar |
| __GetClientConVar_String | ❌ Missing | Client cvar |

---

## Entity Classes - PARTIAL

### GMod 9 Entity Classes Found:

| Entity Class | Status | Notes |
|--------------|--------|-------|
| CPhysicsBalloon / physics_balloon | ❌ Missing | Balloon entity |
| CBalloonController | ❌ Missing | Balloon controller |
| CPropEmitter / prop_emitter | ❌ Missing | Emitter entity |
| CPhysicsThruster / physics_thruster | ❓ Check | Thruster entity |
| CPropWheel / prop_wheel | ❓ Check | Wheel entity |
| CWheelController | ❓ Check | Wheel controller |
| prop_timedexplosion | ❌ Missing | Dynamite entity |
| prop_flashlight | ❌ Missing | Flashlight prop |

### GMod 9 Weapon Classes:

| Weapon Class | Status | Notes |
|--------------|--------|-------|
| CWeaponGravityGun | ✅ Exists | Gravity gun |
| CWeaponPhysCannon | ✅ Exists | Physics cannon |
| CWeaponTool | ❓ Check | Toolgun |
| CWeaponScripted | ❌ Missing | Scripted weapon base |
| CWeaponScriptedPistol | ❌ Missing | Scripted pistol |

---

## Priority Implementation List

### High Priority (Core GMod 9 Features):

1. **File Functions** - Critical for Lua scripts that save/load data
2. **Game Event Functions** - Needed for custom game modes
3. **Missing SWEP Functions** - Required for custom weapons
4. **Entity Classes** (balloon, emitter, dynamite) - Core sandbox tools
5. **__ServerCommand** - Execute server commands from Lua
6. **__PlaySound/__PlaySoundPlayer** - Sound playback

### Medium Priority:

1. **Weapon Functions** - Custom weapon configuration
2. **Missing Team Functions** - Team-based game modes
3. **Camera CVars** - Camera tool functionality
4. **__TraceAttack/__TraceDidHitHitbox** - Combat systems

### Low Priority:

1. **Debug CVars** - Development/debugging only
2. **__PluginMsg/__PluginText** - Plugin communication
3. **__ForceFileConsistency** - Anti-cheat

---

## Notes

- All analysis based on GMod 9.0.4b server.dll IDA database
- Function names prefixed with `__` in IDA correspond to `_` prefix in BMod Lua API
- Some functionality may exist under different names or in inherited HL2MP code
- Entity classes may require both server and client implementations

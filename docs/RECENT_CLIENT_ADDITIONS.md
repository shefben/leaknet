# Recent Client.dll Additions (BMod parity work)

## New client systems
- GMod post-process/overlay pipeline: motion blur (`gmod/motionblur`), overlay load from `settings/gmod_overlay.txt`, shiny overlay (`gmod/shiny`) with `gm_shiny_overlay` toggle.
- Thruster client stub: `C_PhysicsThruster` plays looped audio when `physics_thruster/gmod_thruster` exists (gated by `gm_thruster_sounds`).

## New/updated client commands
- `gm_spawn` (client-forward to server spawn).
- `gm_spawn_model` helper (auto-selects gmod_makeprop/makeragdoll based on name heuristics).
- Existing forwards for `gmod_makeprop`, `gmod_makeragdoll`, `gmod_makeeffect` (already present, noted here for completeness).

## New/updated client ConVars
- Overlay/post-process: `gm_overlay_enable`, `gm_shiny_overlay`, `gm_motionblur_enable`, `gm_motionblur_strength`.
- Balloon controls (client-side to match context panel): `gm_balloon_reverse`, `gm_balloon_power`, `gm_balloon_rope_width`, `gm_balloon_rope_length`, `gm_balloon_rope_forcelimit`, `gm_balloon_rope_rigid`, `gm_balloon_rope_type`, `gm_balloon_explode`.
- Thruster: `gm_thruster_sounds` (clientside audio toggle).
- Spawn/weapon UI: `gm_spawncombolines`, `gm_wepselmode`, `gm_toolweapon`.

## Source integration points
- `view_scene.cpp`: calls `GModPostProcess_Update` before `PerformScreenOverlay` to apply overlay/shiny/motion blur.
- `gmod_client_init.cpp`: initializes/shuts down post-process system and message system.
- `gmod_postprocess.cpp/h`: material loading and ConVar plumbing for overlay/shiny/motion blur.
- `gmod_client_cvars.cpp`: houses client ConVars and spawn/make command forwards.
- `c_physics_thruster.cpp/h`: client class registration and audio handling for thrusters.

## Files touched/added
- cl_dll/bmod_hud/gmod_postprocess.cpp
- cl_dll/bmod_hud/gmod_postprocess.h
- cl_dll/bmod_hud/gmod_client_cvars.cpp
- cl_dll/bmod_hud/gmod_client_init.cpp
- cl_dll/view_scene.cpp
- cl_dll/bmod_hud/c_physics_thruster.cpp
- cl_dll/bmod_hud/c_physics_thruster.h
- cl_dll/client_bmod.cmake

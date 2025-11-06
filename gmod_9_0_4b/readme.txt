
             + ------------------------------------ +
            + Garry's Mod 9.0.4   27th November 2005 +
             + ------------------------------------ +





 Garry's Mod is a modification for Half-Life 2. Since GMod's release in 
  December 2004 the history books of modding have been continually re-written. 

 Finally, in late 2005, GMod 9.0 was released. The history books now
  simply read "Garry Newman Is GOD". Everything else is just a footnote.



 A few people helped me throw this together, here's the SHOUT OUTZ in a 
  warez intro style. n42, DEADBEEF, g33k, Podunkian, helk, andyvincent,
  MrSteak63, hezzy, firerain, appox, DownsP and everyone who I've forgot
  because they are nothings. 

 
 Enjoy!




 Garry Newman
 garrynewman@gmail.com
 http://www.garry.tv/


------------------------------------------------------------------------
 
 -----------
  Changelog
 -----------

9.0.4

[ADD] Spawn menu now grows with a higher screen res
[ADD] Cleaned up spawn menu
[ADD] Changed weapon select menu to fit more weapons (use gm_wepselmode to switch back)
[ADD] SWEP spawn menu
[ADD] CS Weapons
[ADD] NPC Kill notifications
[FIX] Spawn menu crash
[FIX] Single player bug with turrets
[FIX] Melonracer bugs
[FIX] Weapon pickup switch bug
[FIX] Text, rects and wquads showing in the next level
[FIX] Fortwars - Fixed buying weapons as spectator
[FIX] Fortwars - Money resets after game end
[FIX] HideAndSeek - Team names fixed
[FIX] HideAndSeek - Tweaked scoring
[FIX] HideAndSeek - Added help screen
[FIX] HideAndSeek - Hiders get melon and baby guns
[FIX] Chat style messages now show in single player
[FIX] Scope graphics are now drawn behind kills/chat

[LUA] _util.PlayerByName( name )
[LUA] _util.PlayerByUserId( userid )
[LUA] _util.EntsInBox( min, max )
[LUA] _util.DropToFloor( ent )
[LUA] _util.ScreenShake( pos, amp, frequency, duration, radius )
[LUA] _util.PointAtEntity( ent, ent )
[LUA] _npc.ExitScriptedSequence( ent )
[LUA] _npc.SetSchedule( ent, sched )
[LUA] _npc.SetLastPosition( ent, vector )
[LUA] _npc.AddRelationship( ent, ent, disposition, priority )
[LUA] Changed Phys functions to use _phys. table (see lua/includes/backcompat.lua)
[LUA] _phys.ApplyForceOffset( ent, force, worldpos )
[LUA] _phys.ApplyTorqueCenter( ent, vec )
[LUA] _player.ShowPanel( player, name, show )
[LUA] _player.SetContextMenu( player, name )
[LUA] GetPlayerDamageScale( hitgroup ) - see defines.lua
[LUA] _player.GetFlashlight( player )
[LUA] _player.SetFlashlight( player, bool )
[LUA] _player.LastHitGroup( player )
[LUA] Returned tables now start from 1 instead of 0 as is the Lua tradition
[LUA] _player.ShouldDropWeapon( player, bool )
[LUA] SWEP - getTracerFreqPrimary()
[LUA] SWEP - getTracerFreqSecondary()
[LUA] _swep.GetClipAmmo( id, clip )
[LUA] _swep.SetClipAmmo( id, clip, amount )
[LUA] SWEP - onPickup( player )
[LUA] SWEP - onDrop( player )
[LUA] SWEP - onRemove()
[LUA] SWEP - getDeathIcon()
[LUA] _gameevent.Start( name )
[LUA] _gameevent.SetString( name, value )
[LUA] _gameevent.SetInt( name, value )
[LUA] _gameevent.Fire()
[LUA] _spawnmenu.AddItem( player, category, name, command )
[LUA] _spawnmenu.RemoveCategory( player, category )
[LUA] _spawnmenu.RemoveAll( player )
[LUA] _spawnmenu.SetCategory( player, category )
[LUA] eventPlayerInitialSpawn( playerid ) - called just before player's first ever spawn on the server
[LUA] Fixed _file.Read corrupt data
[LUA] _PlayerGetActiveWeapon( player )
[LUA] _swep.GetDeathIcon( weapon )

9.0.3

[ADD] gm_hideandseek by Greg Hughes aka "fayte" and "MrSteak63"
[ADD] Added "cfg/lua.txt" to configure Lua

[FIX] Crash when NPC shoots Jeep
[FIX] Crash when anything happens
[FIX] Crash when hiding GUI text
[FIX] MrSteak's AWFUL scripting in gm_longsight
[FIX] Misc single player crashes


[LUA] Fixed GModQuad_Hide
[LUA] SWEPs with missing scripts no longer spawn


9.0.2

[FIX] Easy Weld crash
[FIX] Easy Axis crash
[FIX] Server hosting crash bug
[FIX] NPC Spawning is always allowed in single player
[FIX] Unable to load from the main menu bug
[FIX] Corrupt save bug after using duplicator
[FIX] Statued ragdolls are a bit more stable
[FIX] Lua memory leak
[ADD] gm_freeze gamemode by Jon Sully aka "Jonny"
[FIX] Updated fortwar map (g33k)
[FIX] Added new weapons to fortwars (g33k)
[FIX] Added buy system to fortwars (g33k)
[FIX] Disabled auto switch in all build gamemodes (g33k)

[LUA] Removed io/os library
[LUA] Added loadlib library
[LUA] _EntSetParent( <ent>, <parent> )
[LUA] _EntGetParent( <ent> )
[LUA] _IsDedicatedServer( )
[LUA] GmodRect/GModText indexes can now be over 128 (~32000 max)
[LUA] GmodRect/GModText doesn't draw when followed ent isn't in PVS
[LUA] _GModQuad_Start( <material> )
[LUA] _GModQuad_SetVector( <index [0-3]>,<vector> )
[LUA] _GModQuad_Hide( <userid>, <index>, <fadetime>, <delay> )
[LUA] _GModQuad_HideAll( <userid> )
[LUA] _GModQuad_SetTimings( <delay>, <fadein>, <life>, <fadeout> )
[LUA] _GModQuad_Send( <player>, <index> )
[LUA] _GModQuad_SendAnimate( <player>, <index>, <length>, <ease> )
[LUA] _EntGetName( <ent> )
[LUA] _EntSetName( <ent>, <name> )
[LUA] eventPlayerUseEntity( playerid, entity )
[LUA] Added lifetype class defs to defines.lua
[LUA] _SetDefaultRelationship( <class>, <class>, <disposition> )
[LUA] _TraceSetCollisionGroup( <group> )
[LUA] Added masks to defines.lua
[LUA] _TraceSetMask( <mask> )
[LUA] _file.Exists( <filename> )
[LUA] _file.Read( <filename> )
[LUA] _file.Write( <filename>, <string> )
[LUA] _file.CreateDir( <folder> )
[LUA] _file.Find( <wildcard> )
[LUA] _file.Delete( <file> )
[LUA] _file.Rename( <before>, <after> )
[LUA] Error messages are now actually useful
[LUA] _TraceAttack( <victim ent>, <inflictor ent>, <attacker ent>, <amount> )
[LUA] 8, 9 and 0 can be used with PlayerOption now (assuming default cam binds)
[LUA] _EntSetMaxHealth( <ent>, <max> )
[LUA] _EntGetMaxHealth( <ent> )

9.0.1


[FIX] Tiny memory leak in MP game panel
[FIX] Possible problem with long GUI Text messages
[ADD] Warning when CS:S reload animations are missing on server
[FIX] lastinv fast firing exploit
[CHA] Bloom is now off by default
[CHA] NPC spawning is now allowed by default in server settings
[FIX] Some Coop/NPC fixes
[FIX] Missing GUI Rects in widescreen mode
[ADD] Some missing server files


9.0.0

[ADD] LUA scripting support
[ADD] 'lua' console command to run LUA commands
[ADD] 'lua_openscript' command to run a script
[ADD] 'lua_listbinds' to list available bound functions
[ADD] 'gm_defaultgamerules' command
[FIX] Possible phys properties bug
[FIX] Bots not interacting with physics
[FIX] Random spawn crash
[ADD] Motion blur effect
[ADD] ALL maps now listed at the bottom of the SP/MP game panels
[FIX] MP Game Panel now remembers maxplayers
[ADD] Wind alter sliders in the Admin menu (buggy)
[FIX] Blood now shows properly online
[ADD] Enabled blood for our German speaking friends
[ADD] New scoreboard
[ADD] New admin options
[FIX] Non chat text in chat area text being weird
[FIX] Weapon selection popup when scrolling in spawn menu
[FIX] Modcache optimizations
[ADD] Zombie player model (Thanks to Podunkian)
[ADD] Stalker player model (Thanks to Podunkian)
[ADD] Classic Gordon player model (Thanks to Podunkian)
[ADD] Charple player model (Thanks to Podunkian)
[ADD] Corpse player model (Thanks to Podunkian)
[FIX] Removed black screen on fall damage
[ADD] Working spectator mode
[FIX] Avatars now work for listen server hosts
[ADD] gmod_gamesetup entity
[ADD] gmod_runfunction entity
[ADD] gmod_player_start entity
[ADD] weapon_swep entity
[FIX] Can now spawn using DoD spawnpoints
[FIX] Bloom hitching when switched on for the first time
[FIX] Colour mod hitching when switched on for the first time
[FIX] Bots not registering as bots when they first joined a server
[ADD] Scrollbar on spawnmenu dropdown box. (use gm_spawncombolines to alter)
[ADD] Bloom presets menu
[FIX] Colour mod shader now works
[FIX] Reduced telefragging
[ADD] cs_assault to CS:S minimod
[ADD] 910ths gamemode by Garry Newman (aka garry)
[ADD] Melon Racer gamemode by Garry Newman (aka garry)
[ADD] Laser Dance gamemode by Garry Newman (aka garry)
[ADD] Tactical Police Cops gamemode by Garry (really just Lua SWEP examples)
[ADD] BirdPoo gamemode by Andy Vincent
[ADD] Football gamemode by Tristan Pemble (aka n42)
[ADD] Zombie Infestation gamemode by Matthew Mullins (aka g33k)
[ADD] gm_build_tothetop gamemode by Matthew Mullins (aka g33k)
[ADD] gm_build_bridge gamemode by Matthew Mullins (aka g33k)
[ADD] Longsight gamemode by "MrSteak63"
[FIX] Importing custom spraypaints now works
[ADD] SetModel console command ("SetModel alyx", "SetModel breen" etc)
[FIX] Memory leak in mod manager
[ADD] gm_build_fortwars gamemode by Matthew Mullins (aka g33k)








9412

----------------------------------------------------------------------------------
-- This script defines the default game rules.
-- These rules are the same as the default game rules for Half-Life 2 deathmatch.
----------------------------------------------------------------------------------

fIntermissionEnd = 0
bEndGame = false
TimeLimit = 0;
FragLimit = 0;


	-- This is called every single frame
	function gamerulesThink ()

		-- bEndGame is true when the game has ended and everyone
		-- is looking at the scoreboard blaming lag for their score
		if ( bEndGame ) then

			if ( fIntermissionEnd < _CurTime() ) then
				_StartNextLevel();
			end

			return
			
		end;

		-- Check the limits to make sure it's not time to go to the next map
		CheckFragLimit();
		CheckTimeLimit();	 		

	end 

	

		

	function CheckFragLimit()
		if ( FragLimit == 0 ) then return end;
		if ( _GetRule( "Teamplay" ) ) then
			local NumTeams = _TeamCount();
			for i=0, NumTeams do
				if ( _TeamScore(i) >= FragLimit ) then
					StartIntermission()
				end
			end
		else 
			for i=0, _MaxPlayers() do

				if ( _PlayerInfo( i, "connected" ) and  _PlayerInfo( i, "kills" ) >= FragLimit ) then

					StartIntermission()

				end
			end
		end

	end

	
	function CheckTimeLimit()

		if ( TimeLimit == 0 ) then return end;
		if ( TimeLimit < _CurTime() ) then
			
			StartIntermission();
			
		end
	
	end

	

	

	

-- Give the players the default weapons --

	function GiveDefaultItems( playerid )

		if ( _GetRule( "SpawnWithAllWeapons" ) ) then

			_PlayerGiveItem( playerid, "weapon_stunstick" )
			_PlayerGiveItem( playerid, "weapon_357" )
			_PlayerGiveItem( playerid, "weapon_smg1" )
			_PlayerGiveItem( playerid, "weapon_ar2" )
			_PlayerGiveItem( playerid, "weapon_shotgun" )
			_PlayerGiveItem( playerid, "weapon_frag" )
			_PlayerGiveItem( playerid, "weapon_crossbow" )
			_PlayerGiveItem( playerid, "weapon_rpg" )
			_PlayerGiveItem( playerid, "weapon_slam" )
			_PlayerGiveItem( playerid, "weapon_bugbait" )
			_PlayerGiveAmmo( playerid, 255, "Pistol", false )
			_PlayerGiveAmmo( playerid, 255, "Buckshot", false )	
			_PlayerGiveAmmo( playerid, 255, "SMG1", false )
			_PlayerGiveAmmo( playerid, 255, "AR2", false )	
			_PlayerGiveAmmo( playerid, 255, "AR2AltFire", false )
			_PlayerGiveAmmo( playerid, 5, "smg1_grenade", false )	
			_PlayerGiveAmmo( playerid, 255, "357", false )
			_PlayerGiveAmmo( playerid, 5, "rpg_round", false )				
			_PlayerGiveAmmo( playerid, 2, "grenade", false )
			_PlayerGiveAmmo( playerid, 2, "slam", false )	
			_PlayerGiveAmmo( playerid, 10, "XBowBolt", false )	

		end

	
		_PlayerGiveItem( playerid, "weapon_crowbar" )
		_PlayerGiveItem( playerid, "weapon_pistol" )
		_PlayerGiveItem( playerid, "weapon_physcannon" )
		_PlayerGiveAmmo( playerid, 100, "SMG1", false )
		_PlayerGiveAmmo( playerid, 25, "Buckshot", false )	

		if ( _GetRule( "AllowPhysgun" ) ) then
			
			_PlayerGiveItem( playerid, "weapon_physgun" )
			
		end

		if ( _GetRule( "AllowMultigun" ) ) then

			_PlayerGiveItem( playerid, "weapon_tool" )

		end

	end

	



--  The current map has ended, show the scoreboard ----------------------------

	function StartIntermission ()

		bEndGame = true;
		fIntermissionEnd = _CurTime() + _GetConVar_Float( "mp_chattime" );

		for i=0, _MaxPlayers() do

			_PlayerShowScoreboard( i );
			_PlayerFreeze( i, true );		

		end

	end

	

	

--  Called right before the new map starts ------------------------------------

	function gamerulesStartMap ()

		bEndGame = false;
		fIntermissionEnd = 0;
		PlayerFreezeAll( false );
		
		-- Set the default team names
		_TeamSetName( TEAM_BLUE, "Blue Team" );
		_TeamSetName( TEAM_GREEN, "Green Team" );
		_TeamSetName( TEAM_YELLOW, "Yellow Team" );
		_TeamSetName( TEAM_RED, "Red Team" );
		
		-- Lets cache these so we don't have to keep grabbing them every frame
		TimeLimit = _GetConVar_Float( "mp_timelimit" ) * 60 -- Minutes to seconds!
		FragLimit = _GetConVar_Float( "mp_fraglimit" )

	end



	function PlayerSpawnChooseModel ( playerid )	
				
		-- Preferred model allows the player to always be one model by running
		-- SetModel alyx (and) by having cl_preferredmodel set in their config
		if ( _PlayerPreferredModel( playerid ) == "" ) then

			-- Only choose a random model once, we don't want them changing every round.
			if ( _PlayerInfo( playerid, "model" ) == DEFAULT_PLAYER_MODEL ) then
				
				_PlayerSetModel( playerid, _PlayerGetRandomAllowedModel() )
			
			end

		else

			_PlayerSetModel( playerid, _PlayerPreferredModel( playerid ) )

		end

	end


-- Player died, add score, add death.	

	function eventPlayerKilled ( killed, attacker, weapon )

		_PlayerAddDeath( killed, 1 )	

		-- Player killed himself - what a moby!
		if ( killed == attacker ) then

			_PlayerAddScore( killed, -1 )

		-- Was killed by another player..
		elseif ( attacker > 0 ) then

			_PlayerAddScore( attacker, 1 )
			
			-- Let the attacking player desecrate the corpse
			_PlayerAllowDecalPaint( attacker ); 

		-- I told you I was hardcore - player killed himself using the world
		else

			_PlayerAddScore( killed, -1 )			

		end	

	end


	-- Player has spawned
	function eventPlayerSpawn ( userid )

		-- If we're in teamplay mode switch team circles on for everyone
		_PlayerSetDrawTeamCircle( userid, _GetRule( "Teamplay" ) )

	end


	-- Scale the damage based on where we were hit.
	function GetPlayerDamageScale( hitgroup ) 
	
		if (hitgroup == HITGROUP_HEAD) then
			return 4.0;
		elseif (hitgroup == HITGROUP_LEFTARM or
				hitgroup == HITGROUP_RIGHTARM or
				hitgroup == HITGROUP_LEFTLEG or
				hitgroup == HITGROUP_RIGHTLEG) then
			return 0.4;
		end
	
		return 1.0;
	
	end
	
	
	function eventNPCKilled( Attacker, NPC, Weapon )
		
		if ( not IsPlayer( Attacker ) ) then return end;
		
		local WeaponName = _EntGetType( Weapon )
		
		if ( WeaponName == "player" ) then 
			Weapon = _PlayerGetActiveWeapon( Attacker )
			WeaponName = _EntGetType( Weapon )
		end
		
		WeaponName = string.gsub(WeaponName, "npc_", "")
		WeaponName = string.gsub(WeaponName, "weapon_", "")
		WeaponName = string.gsub(WeaponName, "grenade_ar2", "smg1_grenade")
		WeaponName = string.gsub(WeaponName, "prop_combine_ball", "combine_ball")
		
		local DeathIcon = _swep.GetDeathIcon( Weapon );
		if (DeathIcon == nil) then DeathIcon = "death_" .. WeaponName end;
					
		local PlayerName = _PlayerInfo( Attacker, "name" );
		
		local KilledName = _EntGetType( NPC );
		KilledName = string.gsub(KilledName, "npc_", "")
		
		local iDsp = _EntGetDisposition( NPC, Attacker )
		local EnemyTeam = TEAM_RED
		
		if (iDsp == D_LI) then
			_PlayerAddScore( Attacker, -1 );
			EnemyTeam = _PlayerInfo( Attacker, "team")
		else
			_PlayerAddScore( Attacker, 1 );	
		end
	
		_gameevent.Start( "gmod_death" )
			_gameevent.SetString( "weapon", WeaponName )
			_gameevent.SetString( "killer", PlayerName )
			_gameevent.SetString( "victim", KilledName )
			_gameevent.SetString( "killicon", DeathIcon )
			_gameevent.SetInt( "killerteam", _PlayerInfo( Attacker, "team") )
			_gameevent.SetInt( "victimteam", EnemyTeam )
		_gameevent.Fire( )
		
	end
	

-- gm_football
-- This code is shit, it was made around the Build 004/005 era and never really upgraded. I can do better, but I've spent too long with this code.
-- I'm pretty sure there's a whole shitload of bugs in here.
-- Please send any fixes/suggestions to n42.

_OpenScript( "includes/timers.lua" )
_OpenScript( "includes/defines.lua" )
_OpenScript( "includes/player.lua" )
_OpenScript( "includes/concommands.lua" )

Timer = {}

iGameMode = 1

Team = {}
Team[TEAM_BLUE] = {}
Team[TEAM_YELLOW] = {}

Team[TEAM_BLUE].Model = {}
Team[TEAM_YELLOW].Model = {}

Team[TEAM_BLUE].Model.Default = "models/player/combine_soldier_prisonguard.mdl"
Team[TEAM_YELLOW].Model.Default = "models/player/Kleiner.mdl"

Team[TEAM_BLUE].Model.Goalie = "models/player/combine_super_soldier.mdl"
Team[TEAM_YELLOW].Model.Goalie = "models/player/eli.mdl"

Team[TEAM_BLUE].Model.Color = {}
Team[TEAM_YELLOW].Model.Color = {}

Team[TEAM_BLUE].Model.Color.Default = "0 0 255"
Team[TEAM_YELLOW].Model.Color.Default = "255 255 0"

Team[TEAM_BLUE].Model.Color.Goalie = nil
Team[TEAM_YELLOW].Model.Color.Goalie = nil

Team[TEAM_BLUE].Goalie = nil
Team[TEAM_YELLOW].Goalie = nil

Team[TEAM_BLUE].Name = "Blue Jays"
Team[TEAM_YELLOW].Name = "Yellow Jackets"

Team[TEAM_BLUE].Goals = 0
Team[TEAM_YELLOW].Goals = 0

Team[TEAM_BLUE].Penalties = 0
Team[TEAM_YELLOW].Penalties = 0

Team[TEAM_BLUE].Logo = "maps/gm_football/bluejay"
Team[TEAM_YELLOW].Logo = "maps/gm_football/yellowjacket"

Ball = {}
Ball.model = "models/props_phx/misc/soccerball.mdl"
Ball.vec = nil
Ball.ent = nil
Ball.entid = 0

Sound = {}
Sound.goal = "gm_football/goal.wav"
Sound.foul = "gm_football/foul.wav"

Game = {}
Game.Round = 0
Game.Rules = {}
Game.Rules.Mode = 1
Game.Rules.HelpTimeout = 10
Game.Rules.TeamTimeout = 10
Game.Mode = {}
Game.Mode[-1] = {}
Game.Mode[-1][1] = "weapon_physcannon"
Game.Mode[0] = {}
Game.Mode[1] = {}
Game.Mode[1][1] = {}
Game.Mode[1][1].Weapon = "weapon_crowbar"
Game.Mode[1][2] = {}
Game.Mode[1][2].Weapon = "weapon_pistol"
Game.Mode[1][2].Ammo = "Pistol"
Game.Mode[1][2].AmmoAmt = 180
Game.Mode[1][3] = {}
Game.Mode[1][3].Weapon = "weapon_shotgun"
Game.Mode[1][3].Ammo = "Buckshot"
Game.Mode[1][3].AmmoAmt = 60
Game.Mode[2] = {}
Game.Mode[2][1] = {}
Game.Mode[2][1].Weapon = "weapon_physcannon"

function gamerulesThink ()
	DoTimers()
end

function gamerulesStartMap ()
	_EntPrecacheModel( Team[TEAM_BLUE].Model.Default )
	_EntPrecacheModel( Team[TEAM_YELLOW].Model.Default )
	_EntPrecacheModel( Team[TEAM_BLUE].Model.Goalie )
	_EntPrecacheModel( Team[TEAM_YELLOW].Model.Goalie )
	_EntPrecacheModel( Ball.model )

	_TeamSetName( TEAM_BLUE, Team[TEAM_BLUE].Name )
	_TeamSetName( TEAM_YELLOW, Team[TEAM_YELLOW].Name )
end

function doScore ( activator, caller, str )
	local teamID = str
	if ( activator == Ball.entid ) then
		_TeamAddScore( teamID, 10 )
		if ( _PlayerInfo( Ball.lasthit, "team" ) == teamID ) then
			_PlayerAddScore( Ball.lasthit, 1 )
		else
			_PlayerAddDeath( Ball.lasthit, 1 )
		end
		GoalText( teamID, Ball.lasthit )
		Team[teamID].Goals = Team[teamID].Goals + 1
		_PlaySound( Sound.goal )
		_EntRemove( activator )
		for i=0, _MaxPlayers() do
			if (_PlayerInfo(i, "connected")) then
				if ( i ~= Ball.lasthit ) then
					_PlayerRemoveAllWeapons( i )
					_PlayerSpectatorStart( i, OBS_MODE_CHASE )
					_PlayerSpectatorTarget( i, Ball.lasthit )
					AddTimer( 3, 1, _PlayerSpectatorEnd, i )
				end
				AddTimer( 3, 1, _PlayerRespawn, i )
			end
		end
		AddTimer( 3, 1, giveAllWeapons )
		AddTimer( 3, 1, BallSpawn, false )
		AddTimer( 3.1, 1, PlayerFreezeAll, true )
		AddTimer( 6, 1, PlayerFreezeAll, false )
		UpdateUI( 0 )
	end
end

function eventPlayerKilled ( killed, attacker, weapon )
	_PrintMessageAll( HUD_PRINTCENTER, "How did " .. _PlayerInfo( killed, "name" ) .. " die?" )
end

function PickDefaultSpawnTeam ( userID )
	ChooseTeam( userID, 5 )
end

function PlayerSpawnChooseModel ( userid )
	local teamID = _PlayerInfo( userid, "team" )
	if ( teamID == 2 or teamID == 3 ) then
		_PlayerSetDrawTeamCircle( userid, true )
	
		_PlayerSetModel( userid, Team[teamID].Model.Default )
	
		_EntFire( _PlayerInfo( userid, "entindex" ), "color", Team[teamID].Model.Color.Default, 0)
	
		if ( Game.Round == 0 ) then
			BallSpawn( false )
			AddTimer( 3, 1, _PhysEnableMotion, Ball.entid, true )
			Game.Round = 1
		end
	end
	UpdateUI( userid )
end

function giveAllWeapons ()
	for i=0, _MaxPlayers() do
		if ( _PlayerInfo( i, "connected" ) ) then
			GiveDefaultItems( i )
		end
	end
end

function GiveDefaultItems ( userID )
	local gMode = Game.Rules.Mode

	for key2,value2 in Game.Mode[gMode] do
		if ( _PlayerInfo( userID, "team" ) ~= TEAM_SPECTATOR ) then
			_PlayerGiveItem( userID, value2.Weapon )
			if ( value2.Ammo ) then
				_PlayerGiveAmmo( userID, value2.AmmoAmt, value2.Ammo, false )
			end
		end
	end
	if ( _PlayerInfo( userID, "team" ) == TEAM_SPECTATOR ) then
		_PlayerRemoveAllWeapons( userID )
	end
end

function setInGoalie ( activator, caller, teamID )
	if ( IsPlayer( activator ) ) then
		local aTeam = teamID + 0
		local pTeam = _PlayerInfo( activator, "team" )
		if ( aTeam ~= pTeam ) then
			_PlayerRespawn( activator )
			_PlaySound( Sound.foul )
			_PlayerFreeze( activator, true )
			AddTimer( 4, 1, _PlayerFreeze, activator, false )
			_TeamAddScore( pTeam, -5 )
			_PlayerAddDeath( activator, 1 )
			Team[pTeam].Penalties = Team[pTeam].Penalties + 1
			UpdateUI( 0 )
		end
	end
end

function setNotInGoalie ( activator, caller, teamID )

end

function setInBox ( activator, caller, teamID )
	if ( IsPlayer( activator ) ) then
		local aTeam = teamID + 0
		local pTeam = _PlayerInfo( activator, "team" )
		if ( aTeam == pTeam ) then
			if ( not Team[pTeam].Goalie ) then
				Team[pTeam].Goalie = activator
				if ( Team[pTeam].Model.Color.Goalie ) then _EntFire( activator, "color", Team[pTeam].Model.Color.Goalie, 0 ) end
				if ( Team[pTeam].Model.Goalie ) then _PlayerSetModel( activator, Team[pTeam].Model.Goalie ) end
				_PlayerGiveItem( activator, "weapon_physcannon" )
				-- _PlayerSelectWeapon( activator, "weapon_physcannon" )
				GoalieText( activator )
				UpdateUI( 0 )
			end
		end
	elseif ( activator == Ball.entid ) then
		_EntFire( Ball.entid, "EnablePhyscannonPickup", "", 0 )
	end
end

function setNotInBox ( activator, caller, teamID )
	if ( IsPlayer( activator ) ) then
		local aTeam = teamID + 0
		local pTeam = _PlayerInfo( activator, "team" )
		if ( pTeam == aTeam ) then
			if ( IsGoalie( activator ) ) then
				Team[pTeam].Goalie = nil
				if ( Team[pTeam].Model.Color.Default ) then _EntFire( activator, "color", Team[pTeam].Model.Color.Default, 0 ) end
				if ( Team[pTeam].Model.Default ) then _PlayerSetModel( activator, Team[pTeam].Model.Default ) end
				_PlayerSelectWeapon( activator, "weapon_crowbar" )
				_PlayerSelectWeapon( activator, "weapon_pistol" )
				_PlayerSelectWeapon( activator, "weapon_shotgun" )
				_PlayerRemoveWeapon( activator, "weapon_physcannon" )
				GoalieText( activator )
				UpdateUI( 0 )
			end
		end
	elseif ( activator == Ball.entid ) then
		_EntFire( Ball.entid, "DisablePhyscannonPickup", "", 0 )
	end
end

function BallSpawn ( bool )
	Ball.vec = _EntGetPos( _EntGetByName( "soccer_ball_pos" ) )
	Ball.ent = _EntCreate( "prop_physics" )
	_EntSetKeyValue( Ball.ent, "model", Ball.model )
	_EntSetKeyValue( Ball.ent, "targetname", "entBall" )
	_PhysEnableMotion( Ball.ent, bool )
	_EntSetPos( Ball.ent, Ball.vec )
	_EntSpawn( Ball.ent )
	Ball.entid = _EntGetByName( "entBall" )
	_EntFire( Ball.entid, "DisablePhyscannonPickup", "", 0 )
	if ( not Ball.hitent ) then
		Ball.hitent = _EntCreate( "gmod_runfunction" )
		_EntSetKeyValue( Ball.hitent, "targetname", "entHit" )
		_EntSetKeyValue( Ball.hitent, "FunctionName", "BallHit" )
		_EntSpawn( Ball.hitent )
	end	
	_EntFire( Ball.entid, "AddOutput", "OnHealthChanged entHit:RunScript", 0 )
end

function BallHit ( activator, caller )
	Ball.lasthit = activator
end

function IsGoalie ( userID )
	local teamID = _PlayerInfo( userID, "team" )
	if ( Team[teamID].Goalie == userID ) then
		return true
	else
		return false
	end
end

-- User Interface

function UpdateUI( userID )
	_GModText_Start( "ImpactMassive" );
	 _GModText_SetPos( 0.025, 0 );
	 _GModText_SetColor( 255, 200, 0, 128 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( "gm_football" );
	_GModText_Send( userID, 0 );
	_GModText_Start( "HudHintTextLarge" );
	 _GModText_SetPos( 0.15, 0.05 );
	 _GModText_SetColor( 255, 255, 255, 128 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( "by n42" );
	_GModText_Send( userID, 1 );
	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.3015, 0.0065 );
	 _GModText_SetColor( 255, 255, 255, 50 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( "Goals:\nPenalties:\nScore:\nGoalie:" );
	_GModText_Send( userID, 2 );
	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.30, 0.005 );
	 _GModText_SetColor( 255, 200, 0, 200 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( "Goals:\nPenalties:\nScore:\nGoalie:" );
	_GModText_Send( userID, 3 );
	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.375, 0.005 );
	 _GModText_SetColor( 255, 255, 255, 200 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( Team[TEAM_YELLOW].Goals .. "\n" .. Team[TEAM_YELLOW].Penalties .. "\n" .. ((Team[TEAM_YELLOW].Goals * 10) - (Team[TEAM_YELLOW].Penalties * 5)) .. "\n" .. GoalieName( TEAM_YELLOW ) );
	_GModText_Send( userID, 4 );
	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.4515, 0.0065 );
	 _GModText_SetColor( 150, 150, 255, 50 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( "Goals:\nPenalties:\nScore:\nGoalie:" );
	_GModText_Send( userID, 5 );
	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.45, 0.005 );
	 _GModText_SetColor( 0, 0, 255, 200 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( "Goals:\nPenalties:\nScore:\nGoalie:" );
	_GModText_Send( userID, 6 );
	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.525, 0.005 );
	 _GModText_SetColor( 255, 255, 255, 200 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( Team[TEAM_BLUE].Goals .. "\n" .. Team[TEAM_BLUE].Penalties .. "\n" .. ((Team[TEAM_BLUE].Goals * 10) - (Team[TEAM_BLUE].Penalties * 5)) .. "\n" .. GoalieName( TEAM_BLUE ) );
	_GModText_Send( userID, 7 );	
	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.65, 0.065 );
	 _GModText_SetColor( 255, 255, 255, 200 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( "F1: Help    F2: Change Team" );
	_GModText_Send( userID, 8 );

	-- RECTANGLES

	_GModRect_Start( "maps/gm_football/logo" );
	 _GModRect_SetPos( 0.22, 0, 0.06, 0.084 );
	 _GModRect_SetColor( 255, 255, 255, 255 );
	 _GModRect_SetTime( 999999, 0, 0 );
	_GModRect_Send( userID, 3 );
	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.0, 0.0, 1, 0.085 );
	 _GModRect_SetColor( 0, 0, 0, 150 );
	 _GModRect_SetTime( 99999, 0, 0 );
	_GModRect_Send( userID, 0 );
	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.0, 0.085, 1, 0.002 );
	 _GModRect_SetColor( 0, 0, 0, 255 );
	 _GModRect_SetTime( 99999, 0, 0 );
	_GModRect_Send( userID, 1 );
	if ( userID == 0 ) then
		for i=1, _MaxPlayers() do
			if ( _PlayerInfo( i, "connected" ) ) then
				_GModRect_Start( Team[ _PlayerInfo( i, "team" ) ].Logo );
				 _GModRect_SetPos( 0.575, 0.00, 0.06, 0.084 );
				 _GModRect_SetColor( 255, 255, 255, 255 );
				 _GModRect_SetTime( 999999, 0, 0 );
				_GModRect_Send( i, 2 );
			end
		end
	elseif ( _PlayerInfo( userID, "connected" ) ) then
		_GModRect_Start( Team[ _PlayerInfo( userID, "team" ) ].Logo )
		 _GModRect_SetPos( 0.575, 0.00, 0.06, 0.084 );
		 _GModRect_SetColor( 255, 255, 255, 255 );
		 _GModRect_SetTime( 999999, 0, 0 );
		_GModRect_Send( userID, 2 );
	end
end

function GoalieText( userID )
	local pTeam = _PlayerInfo( userID, "team" )
	if ( Team[pTeam].Goalie == userID ) then
		_GModText_Start( "ImpactMassive" );
		 _GModText_SetPos( 0.4515, 0.91515 );
		 _GModText_SetColor( 0, 0, 0, 128 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "goalie" );
		_GModText_Send( userID, 9 );
		_GModText_Start( "ImpactMassive" );
		 _GModText_SetPos( 0.45, 0.915 );
		 _GModText_SetColor( 255, 255, 255, 200 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "goalie" );
		_GModText_Send( userID, 10 );
	else
		_GModText_Hide( userID, 9 );
		_GModText_Hide( userID, 10 );
	end
end

CONCOMMAND( "creator", cc_creator )

function cc_creator ( fromplayer, args )
	_PrintMessage( fromplayer, HUD_PRINTCONSOLE, "script created by n42"  );
end

function GoalText( teamID, userID )
	if ( teamID == TEAM_BLUE ) then
		_GModText_Start( "Default" );
		 _GModText_SetPos( 0.6015, 0.0065 );
		 _GModText_SetColor( 0, 0, 0, 128 );
		 _GModText_SetTime( 6, 1, 1 );
		 _GModText_SetText( _PlayerInfo( userID, "name" ) .. " scored for BLUE" );
		_GModText_Send( 0, 35 );
		_GModText_Start( "Default" );
		 _GModText_SetPos( 0.6, 0.005 );
		 _GModText_SetColor( 0, 0, 200, 255 );
		 _GModText_SetTime( 6, 1, 1 );
		 _GModText_SetText( _PlayerInfo( userID, "name" ) .. " scored for BLUE" );
		_GModText_Send( 0, 36 );
	else
		_GModText_Start( "Default" );
		 _GModText_SetPos( 0.6015, 0.0065 );
		 _GModText_SetColor( 0, 0, 0, 128 );
		 _GModText_SetTime( 6, 1, 1 );
		 _GModText_SetText( _PlayerInfo( userID, "name" ) .. " scored for YELLOW" );
		_GModText_Send( 0, 35 );
		_GModText_Start( "Default" );
		 _GModText_SetPos( 0.6, 0.005 );
		 _GModText_SetColor( 200, 156, 0, 255 );
		 _GModText_SetTime( 6, 1, 1 );
		 _GModText_SetText( _PlayerInfo( userID, "name" ) .. " scored for YELLOW" );
		_GModText_Send( 0, 36 );
	end

	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.6, 0.005 );
	 _GModText_SetColor( 255, 255, 255, 255 );
	 _GModText_SetTime( 6, 1, 1 );
	 _GModText_SetText( _PlayerInfo( userID, "name" ) .." scored for " );
	_GModText_Send( 0, 37 );

	if ( _PlayerInfo( userID, "team" ) == TEAM_BLUE ) then
		_GModText_Start( "Default" );
		 _GModText_SetPos( 0.6, 0.005 );
		 _GModText_SetColor( 0, 0, 200, 255 );
		 _GModText_SetTime( 6, 1, 1 );
		 _GModText_SetText( _PlayerInfo( userID, "name" ) );
		_GModText_Send( 0, 38 );
	else
		_GModText_Start( "Default" );
		 _GModText_SetPos( 0.6, 0.005 );
		 _GModText_SetColor( 200, 156, 0, 255 );
		 _GModText_SetTime( 6, 1, 1 );
		 _GModText_SetText( _PlayerInfo( userID, "name" ) );
		_GModText_Send( 0, 38 );
	end
end

function GoalieName( teamID )
	if ( Team[teamID].Goalie ) then
		return _PlayerInfo( Team[teamID].Goalie, "name" )
	else
		return "None"
	end
end

function onShowHelp ( userid )
	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.2, 0.3, 0.0, 0.0 );
	 _GModRect_SetColor( 0, 0, 0, 100 );
	 _GModRect_SetTime( Game.Rules.HelpTimeout, 0, 1 );
	_GModRect_Send( userid, 50 );

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.198, 0.298, 0.604, 0.154 );
	 _GModRect_SetColor( 0, 0, 0, 150 );
	 _GModRect_SetTime( Game.Rules.HelpTimeout, 0, 1 );
	_GModRect_SendAnimate( userid, 50, 1, 0.2 );

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.2, 0.3, 0.0, 0.0 );
	 _GModRect_SetColor( 0, 0, 0, 10 );
	 _GModRect_SetTime( Game.Rules.HelpTimeout, 0, 1 );
	_GModRect_Send( userid, 51 );

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.2, 0.3, 0.6, 0.15 );
	 _GModRect_SetColor( 0, 0, 0, 50 );
	 _GModRect_SetTime( Game.Rules.HelpTimeout, 0, 1 );
	_GModRect_SendAnimate( userid, 51, 1, 0.2 );

	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.21, 0.36 );
	 _GModText_SetColor( 255, 255, 255, 0 );
	 _GModText_SetTime( Game.Rules.HelpTimeout - 2, 1, 1 );
	 _GModText_SetText( "- Shoot the ball to score.\n- Each team has one goalie.\n- The football can only be picked up when it is in the goalie box.\n- You will be penalized if you enter the opposing team's safety zone." );
	_GModText_Send( userid, 50 );

	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.21, 0.36 );
	 _GModText_SetColor( 255, 255, 255, 255 );
	 _GModText_SetTime( Game.Rules.HelpTimeout - 2, 1, 1 );
	_GModText_SendAnimate( userid, 50, 2, 0.2 );

	_GModText_Start( "ImpactMassive" );
	 _GModText_SetPos( 0.41, 0.295 );
	 _GModText_SetColor( 255, 255, 255, 0 );
	 _GModText_SetTime( Game.Rules.HelpTimeout - 2, 1, 1 );
	 _GModText_SetText( "[HELP]" );
	_GModText_Send( userid, 51 );

	_GModText_Start( "ImpactMassive" );
	 _GModText_SetPos( 0.41, 0.295 );
	 _GModText_SetColor( 255, 255, 255, 255 );
	 _GModText_SetTime( Game.Rules.HelpTimeout - 2, 1, 1 );
	_GModText_SendAnimate( userid, 51, 2, 0.2 );

	_PlayerOption( userid, "ChooseHelp", Game.Rules.HelpTimeout );
end

function onShowTeam ( userid )
	_PlayerOption( userid, "ChooseTeam", 22 );

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.0, 0.375, 0.0, 0.15 );
	 _GModRect_SetColor( 0, 0, 0, 150 );
	 _GModRect_SetTime( Game.Rules.TeamTimeout, 1, 1 );
	_GModRect_Send( userid, 20 );

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.0, 0.375, 0.125, 0.15 );
	 _GModRect_SetColor( 0, 0, 0, 150 );
	 _GModRect_SetTime( Game.Rules.TeamTimeout, 1, 1 );
	_GModRect_SendAnimate( userid, 20, .5, 0.2 );

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.0, 0.375, 0.0, 0.002 );
	 _GModRect_SetColor( 0, 0, 0, 255 );
	 _GModRect_SetTime( Game.Rules.TeamTimeout, 1, 1 );
	_GModRect_Send( userid, 21 );

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.0, 0.375, 0.125, 0.002 );
	 _GModRect_SetColor( 0, 0, 0, 255 );
	 _GModRect_SetTime( Game.Rules.TeamTimeout, 1, 1 );
	_GModRect_SendAnimate( userid, 21, .5, 0.2 );

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.0, 0.524, 0.0, 0.002 );
	 _GModRect_SetColor( 0, 0, 0, 255 );
	 _GModRect_SetTime( Game.Rules.TeamTimeout, 1, 1 );
	_GModRect_Send( userid, 22 );

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.0, 0.524, 0.125, 0.002 );
	 _GModRect_SetColor( 0, 0, 0, 255 );
	 _GModRect_SetTime( Game.Rules.TeamTimeout, 1, 1 );
	_GModRect_SendAnimate( userid, 22, .5, 0.2 );

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.0, 0.375, 0.001, 0.15 );
	 _GModRect_SetColor( 0, 0, 0, 255 );
	 _GModRect_SetTime( Game.Rules.TeamTimeout, 1, 1 );
	_GModRect_Send( userid, 23 );

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0.125, 0.375, 0.001, 0.15 );
	 _GModRect_SetColor( 0, 0, 0, 255 );
	 _GModRect_SetTime( Game.Rules.TeamTimeout, 1, 1 );
	_GModRect_SendAnimate( userid, 23, .55, 0.2 );

	_GModText_Start( "Default" );
	 _GModText_AllowOffscreen( true );
	 _GModText_SetPos( 0.015, 0.4 );
	 _GModText_SetColor( 255, 255, 255, 0 );
	 _GModText_SetTime( Game.Rules.TeamTimeout, 1, 1 );
	 _GModText_SetText( "Choose team:\n\n1. Blue Team\n2. Yellow Team\n5. Auto\n6. Spectate" );
	_GModText_Send( userid, 20 );

	_GModText_Start( "Default" );
	 _GModText_AllowOffscreen( true );
	 _GModText_SetPos( 0.015, 0.4 );
	 _GModText_SetColor( 255, 255, 255, 255 );
	 _GModText_SetTime( Game.Rules.TeamTimeout, 1, 1 );
	 _GModText_SetText( "Choose team:\n\n1. Blue Team\n2. Yellow Team\n5. Auto\n6. Spectate" );
	_GModText_SendAnimate( userid, 20, 1.3, 0.2 );
end

function onShowSpare1 ( userid )

end

function onShowSpare2 ( userid )

end

function ChooseTeam ( userID, teamID, secs )
	_GModText_Hide( userID, 20, 1.5 );
	_GModRect_Hide( userID, 20, 1.5 );
	_GModRect_Hide( userID, 21, 1.5 );
	_GModRect_Hide( userID, 22, 1.5 );
	_GModRect_Hide( userID, 23, 1.5 );

	if ( teamID == 1 ) then
		if ( _PlayerInfo( userID, "team" ) == TEAM_BLUE ) then return; end;
		
		_PlayerChangeTeam( userID, TEAM_BLUE );
		_PlayerSilentKill( userID );
		return;
	end

	if ( teamID == 2 ) then
		if ( _PlayerInfo( userID, "team" ) == TEAM_YELLOW ) then return; end;
		_PlayerChangeTeam( userID, TEAM_YELLOW );
		_PlayerSilentKill( userID );
		return;
	end

	if ( teamID == 6 ) then
		if ( _PlayerInfo( userID, "team" ) == TEAM_SPECTATOR ) then return; end;
		_PlayerChangeTeam( userID, TEAM_SPECTATOR );
		_PlayerSilentKill( userID );
		return;
	end
	
	if ( _TeamNumPlayers( TEAM_YELLOW ) > _TeamNumPlayers( TEAM_BLUE ) ) then
		ChooseTeam( userID, 1, 0 );	
	elseif ( _TeamNumPlayers( TEAM_YELLOW ) == _TeamNumPlayers( TEAM_BLUE ) ) then
		if ( _TeamScore( TEAM_YELLOW ) > _TeamScore( TEAM_BLUE ) ) then
			ChooseTeam( userID, 1, 0 );
		else
			ChooseTeam( userID, 2, 0 );
		end 
	else
		ChooseTeam( userID, 2, 0 );
	end
end

function ChooseHelp ( userID, choice, sec )
	_GModRect_Hide( userID, 50 )
	_GModRect_Hide( userID, 51 )
	_GModText_Hide( userID, 50 )
	_GModText_Hide( userID, 51 )
end

function onGravGunPunt( userID, entid )
	if ( entid == Ball.entid ) then
		BallHit( userID, userID )
	end
	return true;
end

function onGravGunPickup( userID, entid )
	if ( entid == Ball.entid ) then
		BallHit( userID, userID )
	end
	return true;
end

function onGravGunLaunch( userID, entid )
	if ( entid == Ball.entid ) then
		BallHit( userID, userID )
	end
	return true;
end
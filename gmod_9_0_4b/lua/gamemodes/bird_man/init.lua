
ROUND_TIME = (5 * 60)
timeUpdater = 0
RT_BUILD = 0
RT_FLYTEST = 1
RT_DEATHMATCH = 2

roundStartTime = 0
roundEndTime = 0
roundEndFunc = nil
roundType = -1

wallEnabled = 0
	
PlayerInfo = {}

Stats = {}

Stats.LongestDistance = {}
Stats.LongestDistance.Player = "nobody"
Stats.LongestDistance.Value = 0
Stats.LongestDistance.Team = TEAM_SPECTATOR

Stats.LongestTime = {}
Stats.LongestTime.Player = "nobody"
Stats.LongestTime.Value = 0
Stats.LongestTime.Team = TEAM_SPECTATOR

Stats.ShortestDistance = {}
Stats.ShortestDistance.Player = "nobody"
Stats.ShortestDistance.Value = 0
Stats.ShortestDistance.Team = TEAM_SPECTATOR

Stats.ShortestTime = {}
Stats.ShortestTime.Player = "nobody"
Stats.ShortestTime.Value = 0
Stats.ShortestTime.Team = TEAM_SPECTATOR


_OpenScript( "includes/Events.lua" )
_OpenScript( "includes/player.lua" )
_OpenScript( "includes/misc.lua" )

_OpenScript( "gamemodes/bird_man/gamerules.lua" );
_OpenScript( "gamemodes/bird_man/events.lua" );
_OpenScript( "gamemodes/bird_man/gui.lua" );
_OpenScript( "gamemodes/bird_man/cmds.lua" );
_OpenScript( "gamemodes/bird_man/adminlist.lua")

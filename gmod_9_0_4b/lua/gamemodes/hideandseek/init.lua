--##############################
--## Hide and Seek
--## fayte, gdhughes@gmail.com
--##############################

TEAM_HIDERS = 2
TEAM_SEEKERS = 4

HSIsFound = {}
HSNumFound = 0
HSRoundNum = 1

_GameSetTargetIDRules( 3 );

_TeamSetScore( TEAM_HIDERS, 0 );
_TeamSetScore( TEAM_SEEKERS, 0 );

for userid=0, _MaxPlayers() do
	_PlayerChangeTeam( userid, TEAM_SPECTATOR );
	_EntSpawn( userid );
end


DrawScores( 0 );
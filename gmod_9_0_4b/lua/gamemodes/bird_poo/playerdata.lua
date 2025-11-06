
PlayerData = {};

for i=0, _MaxPlayers() do

	PlayerData[i] = {}
	PlayerData[i].BirdType = BT_SEAGULL	-- always valid
	PlayerData[i].Bird = 0			-- valid only for TEAM_BIRDS
	PlayerData[i].IsFiring = 0		-- valid only for TEAM_BIRDS
	PlayerData[i].IsAltFiring = 0		-- valid only for TEAM_BIRDS
	PlayerData[i].IsSpeedFiring = 0		-- valid only for TEAM_BIRDS
	PlayerData[i].UpdateTimer = 0		-- valid only for TEAM_BIRDS
	PlayerData[i].Ang = vector3(0,0,0)	-- valid only for TEAM_BIRDS and IsAltFiring is non-zero
	PlayerData[i].LastPoopDecal = 0		-- valid for both teams
	PlayerData[i].RunLeft = MAX_RUN		-- valid only for TEAM_BIRDS	
	
end

FIRST_HUD_POOP_DECAL = 75

function ClearPlayerData(playerid, respawn)

	if (PlayerData[playerid] == nil) then
		return
	end
	
	if (PlayerData[playerid].Bird > 0) then
		HaltTimer( PlayerData[playerid].UpdateTimer )
		
		_EntFire( PlayerData[playerid].Bird, "kill", 0, 0 );
		AddTimer( 2, 1, _EntRemove, PlayerData[playerid].Bird )
	end
	
	PlayerData[playerid].IsAltFiring = 0
	PlayerData[playerid].IsFiring = 0
	PlayerData[playerid].IsSpeedFiring = 0
	PlayerData[playerid].Bird = 0
	PlayerData[playerid].UpdateTimer = 0
	PlayerData[playerid].LastPoopDecal = 0
	PlayerData[playerid].RunLeft = MAX_RUN
	
	local x = FIRST_HUD_POOP_DECAL
	
	while (x <= (FIRST_HUD_POOP_DECAL + MAX_HUD_POOP)) do
	
		_GModRect_Hide( playerid, x, 0.0 );
		
		x = x + 1
	end
	
	if (respawn ~= 0) then

		AddTimer( 3, 1, _EntSpawn, playerid );

	end
end

function AddHUDPoop(playerid)

	if (PlayerData[playerid] == nil) then
		return
	end
	
	if (PlayerData[playerid].LastPoopDecal == 0) then
		PlayerData[playerid].LastPoopDecal = FIRST_HUD_POOP_DECAL
	else
		PlayerData[playerid].LastPoopDecal = PlayerData[playerid].LastPoopDecal + 1
		
		if (PlayerData[playerid].LastPoopDecal > (MAX_HUD_POOP + FIRST_HUD_POOP_DECAL)) then
			PlayerData[playerid].LastPoopDecal = FIRST_HUD_POOP_DECAL
		end
	end
	
	-- This is needed because math.random(x,y) returns an INTERGER, not a FLOAT
	local x_pos = math.random(10, 90) / 100.0
	local y_pos = math.random(10, 90) / 100.0
	
	local poop_tex = "gmod/bird_poo/poop" .. math.random(4)
	
	_GModRect_Start( poop_tex );
	 _GModRect_SetPos( x_pos, -0.5, 0.2, 0.2 );
	 _GModRect_SetColor( 255, 255, 255, 255 );
	 _GModRect_SetTime( 99999, 0, 1.0 );
	_GModRect_Send( playerid, PlayerData[playerid].LastPoopDecal );

	_GModRect_Start( "" );
	 _GModRect_SetPos( x_pos, y_pos, 0.2, 0.2 );
	 _GModRect_SetColor( 255, 255, 255, 255 );
	_GModRect_SendAnimate( playerid, PlayerData[playerid].LastPoopDecal, 0.5, 0.1 );
end



fIntermissionEnd = 0;
bEndRound = false;


function gamerulesThink ()
	if (bEndRound) then
		if (fIntermissionEnd < _CurTime()) then
			RoundRestart();
		end;
		return;
	end;
	
	local i, playerCount, humanCount, zombieCount, winner = 1, 0, 0, 0, -1;
	for i=1, _MaxPlayers() do
		if (_PlayerInfo(i, "connected")) then
			playerCount = playerCount + 1;
			if (zombies[i]) then
				zombieCount = zombieCount + 1;
				if SET_ZOMBIE_SPEED then
					if _PlayerInfo(i, "alive") then
						_PlayerSetMaxSpeed(i, ZOMBIE_SPEED);
					end
				end
				if (not sethealth[i]) then
					local ihumanCount, iZombieCount = 0, 0;
					for i=1, _MaxPlayers() do
						if (_PlayerInfo(i, "connected")) then
							if (zombies[i]) then
								iZombieCount = iZombieCount + 1;
							else
								ihumanCount = ihumanCount + 1;
							end
						end
					end
					if MULTIPLY_ZOMBIE_HEALTH then
						local addHealth = (ZOMBIE_HEALTH * (ihumanCount - iZombieCount));
						if (addHealth > 0) then
							if _PlayerInfo(i, "alive") then
								_PlayerSetHealth(i, ZOMBIE_HEALTH + addHealth);
								sethealth[i] = true;
							end
						else
							if _PlayerInfo(i, "alive") then
								_PlayerSetHealth(i, ZOMBIE_HEALTH);
								sethealth[i] = true;
							end
						end
					else
						if _PlayerInfo(i, "alive") then
							_PlayerSetHealth(i, ZOMBIE_HEALTH);
							sethealth[i] = true;
						end
					end
				end
			else
				if SET_HUMAN_SPEED then
					if _PlayerInfo(i, "alive") then
						_PlayerSetMaxSpeed(i, HUMAN_SPEED);
					end
				end
				humanCount = humanCount + 1;
				winner = i;
			end;
		end;
	end;
	if (humanCount == 1) and (playerCount > 2) and (not bEndRound) then
		if (winner > 0) then
			_ScreenText(0, _PlayerInfo(winner, "name") .. " won the game!",  -1,0.2,   255,100,0,255,  1,3, 6, 1, 1);
			_PlayerAddScore(winner, 1);
		end
		
		StartIntermission();
	elseif ((zombieCount < 1) or (humanCount < 1)) and (playerCount > 1) and (not bEndRound) then
		_ScreenText(0, "Restarting the round...",  -1,0.2,   255,100,0,255,  1,3, 6, 1, 1);
		StartIntermission();
	end
end;

function gamerulesStartMap ()
	bEndRound = false;
	fIntermissionEnd = 0
		
	_TeamSetName(TEAM_BLUE, "Humans");
	_TeamSetName(TEAM_GREEN, "Zombies");

	local playersConnected = {};
	local playerCount = 0;
	for i=1, _MaxPlayers() do
		if (_PlayerInfo(i, "connected")) then
			zombies[i] = false;
			sethealth[i] = false;
			playerZombieModel[i] = RandomZombieModel();
			playerHumanModel[i] = RandomHumanModel();
			table.insert(playersConnected, i);
			playerCount = playerCount + 1;
		end
	end
	
	local Rand = math.random(1, playerCount);
	local rUserID = playersConnected[Rand];
	if (_PlayerInfo(rUserID, "connected" )) then
		SwitchToZombie(rUserID);
	else
		_Msg("Setting default zombie 1\n");
		SwitchToZombie(1);
	end
end


function GiveDefaultItems( playerid )
	if (zombies[playerid]) then
		_PlayerGiveItem( playerid, "weapon_crowbar" );
	else
		_PlayerGiveAmmo( playerid, 27, "357", false );
	
		_PlayerGiveItem( playerid, "weapon_shotgun" );
		_PlayerGiveItem( playerid, "weapon_smg1" );
		_PlayerGiveItem( playerid, "weapon_crowbar" );
		_PlayerGiveItem( playerid, "weapon_physcannon" );
		
		_PlayerGiveAmmo( playerid, 255, "pistol", false );
		_PlayerGiveAmmo( playerid, 255, "SMG1", false );
		_PlayerGiveAmmo( playerid, 128, "Buckshot", false );
	end
end

function StartIntermission ()
	bEndRound = true;
	fIntermissionEnd = _CurTime() + _GetConVar_Float( "mp_chattime" );
	PlayerFreezeAll(true);
end;

function RoundRestart( )
	bEndRound = false;
	fIntermissionEnd = 0;

	-- make everyone human again
	local playersConnected = {};
	local playerCount = 0;
	for i=1, _MaxPlayers() do
		if (_PlayerInfo( i, "connected" )) then
			SwitchToHuman(i);
			table.insert(playersConnected, i);
			playerCount = playerCount + 1;
		end
	end
	
	local Rand = math.random(1, playerCount);
	local rUserID = playersConnected[Rand];
	if (_PlayerInfo(rUserID, "connected" )) then
		SwitchToZombie(rUserID);
	else
		_Msg("Setting default zombie 1\n");
		SwitchToZombie(1);
	end
	
	PlayerFreezeAll( false );
	
	for i=1, _MaxPlayers() do 
		if not zombies[i] then _PlayerRespawn(i);
		else
			if _PlayerInfo(i, "alive") then _PlayerSilentKill(i, 5, true)
			else
				_PlayerRespawn(i);
			end
		end
	end
end

function PickDefaultSpawnTeam( userid )
	if zombies[userid] then
		local zombieModel = SetPlayerZombieModel(userid);
		_PlayerSetModel(userid, zombieModel);
		_PlayerChangeTeam(userid, TEAM_GREEN);
	else
		local humanModel = SetPlayerHumanModel(userid);
		_PlayerSetModel(userid, humanModel);
		_PlayerChangeTeam(userid, TEAM_BLUE);
	end
	return true;
end


function SwitchToZombie(userid)
	zombies[userid], sethealth[userid] = true, false;
	local zombieModel;
	if not playerZombieModel[userid] then
		zombieModel = SetPlayerZombieModel(userid);
	else zombieModel = playerZombieModel[userid]
	end
	_PlayerSetModel(userid, zombieModel);
	_PlayerChangeTeam(userid, TEAM_GREEN);
end

function SwitchToHuman(userid)
	zombies[userid], sethealth[userid] = false, false;
	local humanModel;
	if not playerHumanModel[userid] then
		humanModel = SetPlayerHumanModel(userid);
	else humanModel = playerHumanModel[userid]
	end
	_PlayerSetModel(userid, humanModel);
	_PlayerChangeTeam(userid, TEAM_BLUE);
end


function SetPlayerZombieModel(userid)
	local RandomModel = RandomZombieModel();
	if RandomModel then
		playerZombieModel[userid] = RandomModel;
		return RandomModel;
	else
		playerZombieModel[userid] = "models/player/stripped.mdl";
		return "models/player/stripped.mdl";
	end
end

function SetPlayerHumanModel(userid)
	local RandomModel = RandomHumanModel();
	if RandomModel then
		playerHumanModel[userid] = RandomModel;
		return RandomModel;
	else
		playerHumanModel[userid] = "models/player/stalker.mdl";
		return "models/player/stalker.mdl";
	end
end


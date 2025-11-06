function eventPlayerKilled ( killed, attacker, weapon )
	_PlayerAddDeath(killed, 1)
	if (killed == attacker) then
		_PlayerAddScore(killed, -1)
	end
	
	if (zombies[killed]) then
		sethealth[killed] = false;
	else
		zombies[killed] = true;
		if playerZombieModel[killed] then
			_PlayerSetModel(killed, playerZombieModel[killed]);
		else
			local RandModel = RandomZombieModel();
			playerZombieModel[killed] = RandModel;
			_PlayerSetModel(killed, RandModel);
		end
		_PlayerChangeTeam(killed, TEAM_GREEN);
	end
end

function eventPlayerDisconnect (name, userid, address, steamid, reason)
	zombies[userid] = false;
	sethealth[userid] = false;
	playerZombieModel[userid] = RandomZombieModel();
	playerHumanModel[userid] = RandomHumanModel();
end

function eventPlayerSpawn (userid)
	_PlayerSetDrawTeamCircle(userid, true);
end

function canPlayerHaveItem (playerid, itemname)
	if ((zombies[playerid]) and (_PlayerInfo(playerid, "alive"))) then
		if (itemname == "weapon_crowbar") then return true; end
		return false
	end
	return true
end


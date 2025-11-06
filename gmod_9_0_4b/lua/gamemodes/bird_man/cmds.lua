
-- thanks g33k
function isAdmin(userid)
	for i,v in ipairs(adminlist) do
		if (userid) and (userid > 0) then
			if (string.lower(v) == string.lower(_PlayerInfo(userid,"networkid"))) then
				return true;
			end
		end
	end
	return false;
end

function cc_removeAllProps (fromplayer, args)
	if (isAdmin(fromplayer)) then
		local props = _EntitiesFindByClass("prop_physics");
		for i,v in ipairs(props) do
			_EntRemove(v);
		end
	end
end

function cc_restartRound (fromplayer, args)
	if (isAdmin(fromplayer)) then
		restartRound()
	end
end

function cc_setTimeLimit (fromplayer, args)
	if (isAdmin(fromplayer)) then
		if tonumber(args) then
			ROUND_TIME = args * 60;
		end
	end
end

function cc_set_dm_next (fromplayer, args)
	if (isAdmin(fromplayer)) then
		SetNextRoundDeathMatch()
	end
end


CONCOMMAND("bm_removeallprops", cc_removeAllProps);
CONCOMMAND("bm_restartround", cc_restartRound);
CONCOMMAND("bm_timelimit", cc_setTimeLimit);
CONCOMMAND("bm_dm_next", cc_set_dm_next);


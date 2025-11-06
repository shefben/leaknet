-------------------------------------------
-- Zombie Infestation
-- Created by g33k
-- Version 1.0
--
-- Try to be the last to survive. If a zombie kills you then you respawn as a zombie.
-- The zombies health is dynamic. If you start out as a zombie you will have alot more
-- health to compinsate for the lack of teamates. The more humans that turn into zombies
-- the less health zombies get.
--
-------------------------------------------


-- Add other models as you please. Just make sure each one has a comma after it except the last one
HUMAN_MODELS = {
	"models/player/alyx.mdl",
	"models/player/barney.mdl",
	"models/player/breen.mdl",
	"models/player/eli.mdl",
	"models/player/monk.mdl"
};

ZOMBIE_MODELS = {
	"models/player/corpse1.mdl",
	"models/player/classic.mdl",
	"models/player/stalker.mdl",
	"models/player/charple01.mdl"
};

-- Set this to true if you want to set the humans speed (note: if you set the speed they can't sprint)
SET_HUMAN_SPEED = false;
HUMAN_SPEED = 200;

-- Set this to true if you want to set the zombies speed (note: if you set the speed they can't sprint)
SET_ZOMBIE_SPEED = false;
ZOMBIE_SPEED = 200;
-- This is used to multipy the zombie's health. It multiplys by how many humans there are
-- and then subtracts from the number of zombies. If you don't want it setup like that
-- (you want a set health for zombies) then set this value to false
MULTIPLY_ZOMBIE_HEALTH = true;
ZOMBIE_HEALTH = 150;




----------------------------------------------------------------------------------------
-- Don't edit below this line...
----------------------------------------------------------------------------------------

zombies = {};
sethealth = {};
playerZombieModel = {};
playerHumanModel = {};

for i,v in ipairs(HUMAN_MODELS) do _EntPrecacheModel(v) end

for i,v in ipairs(ZOMBIE_MODELS) do _EntPrecacheModel(v) end

function RandomHumanModel ()
	if table.getn(HUMAN_MODELS) > 0 then
		local Rand = math.random(1,table.getn(HUMAN_MODELS))
		return HUMAN_MODELS[Rand]
	else
		return "models/player/barney.mdl"
	end
end

function RandomZombieModel ()
	if table.getn(ZOMBIE_MODELS) > 0 then
		local Rand = math.random(1,table.getn(ZOMBIE_MODELS))
		return ZOMBIE_MODELS[Rand]
	else
		return "models/player/corpse1.mdl"
	end
end

_OpenScript( "includes/events.lua" );
_OpenScript( "gamemodes/ZombieInfestation/gamerules.lua" );
_OpenScript( "gamemodes/ZombieInfestation/events.lua" );

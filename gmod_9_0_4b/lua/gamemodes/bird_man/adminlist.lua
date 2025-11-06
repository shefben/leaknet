
-- Only add the steam ids of people who you can really trust
-- Each steamid has to be surrounded by quotes and a comma must come after each on except the last one
-- Leave "UNKNOWN" in the list if you are hosting a listen server otherwise you won't be admin
adminlist = {
	"UNKNOWN", -- leave this one
	"STEAM_0:1:1337", -- change/remove this, they are just examples
	"STEAM_0:2:1337" -- change/remove this, they are just examples
}


--[[
Admin commands

bm_removeallprops
Removes all the props that have been spawned

bm_restartround
Restarts the round

bm_timelimit <minutes>
Set's the round time limit in minutes. Example: bg_timelimit 30
You'll have to restart the round for it to take effect

bm_dm_next
Set's the next rount to be deathmatch!

]]--
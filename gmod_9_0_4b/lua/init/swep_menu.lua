
-- SWEP menu, Garry Newman 2005
--
-- To disable this menu just change Enabled to false below.
--
--

g_SWEPMenu = {}
g_SWEPMenu.Enabled			= true
g_SWEPMenu.MenuTitle		= "SWEP Weapons"
g_SWEPMenu.SpawnRate		= 3.0
g_SWEPMenu.Players			= {}
g_SWEPMenu.ShowCategories	= true
g_SWEPMenu.ExcludeFolders	= { ".",
								"..",
								"football", 
								"build", 
								"hideandseek" }

if (g_SWEPMenu.Enabled) then

	g_SWEPs = {}

	function AddSWEPToMenu( category, name, filepath )

		-- Strip some crap from the weapon names..
		--name = string.gsub(name, "weapon_", "")
		--name = string.gsub(name, "tpc_", "")
		name = string.gsub(name, ".lua", "")
		
		filepath = string.gsub(filepath, "lua/weapons/", "")
		filepath = string.gsub(filepath, ".lua", "")
		
		local tWeapon = {}
		tWeapon.category	= category;
		tWeapon.name		= name;
		tWeapon.filename	= filepath;
		
		table.insert( g_SWEPs, tWeapon );
		
	end


	function ProcessSWEPFolder( k, name )
		
		for i=1, table.getn(g_SWEPMenu.ExcludeFolders) do
			if (g_SWEPMenu.ExcludeFolders[i] == name) then 
				return; 
			end
		end
		
		local FileName = "lua/weapons/" .. name ;
		if (_file.IsDir(FileName) == false) then return end;

		local tFiles = _file.Find( FileName .. "/*.lua" );

		for i=1, table.getn(tFiles) do
		
			AddSWEPToMenu( name, tFiles[i], FileName .."/".. tFiles[i] );
			
		end

	end


	function PopulateSWEPList()

		local tFiles = _file.Find( "lua/weapons/*" );
		table.foreach( tFiles, ProcessSWEPFolder );
		
	end


	function PlayerSendSWEPMenu( iPlayer )

		if (iPlayer==0) then return end;
		if (_GetRule( "AllowObjectSpawning" )==false) then return; end;

		local LastCat = ""
		
		-- Set up this player's settings
		g_SWEPMenu.Players[iPlayer] = {}
		g_SWEPMenu.Players[iPlayer].NextSpawn = 0

		for i=1, table.getn(g_SWEPs) do
		
			if ( LastCat ~= g_SWEPs[i].category and g_SWEPMenu.ShowCategories ) then			
				_spawnmenu.AddItem( iPlayer, g_SWEPMenu.MenuTitle, "@" .. g_SWEPs[i].category, "" );
				LastCat = g_SWEPs[i].category
			end
		
			_spawnmenu.AddItem( iPlayer, g_SWEPMenu.MenuTitle, "+" .. g_SWEPs[i].name, "SWEPSpawn " .. g_SWEPs[i].filename );
			
		end

	end


	function cc_SpawnSWEP( iPlayer, strFilename )
	
		-- Time between spawns.
		if ( g_SWEPMenu.Players[iPlayer].NextSpawn > _CurTime() and _MaxPlayers() > 1 ) then
			return;
		end
		
		if ( _GetRule( "AllowObjectSpawning" )==false ) then return end;
		if ( _file.Exists( "lua/weapons/" .. strFilename .. ".lua" ) == false ) then return end;
		if ( not _PlayerInfo( iPlayer, "alive") ) then return end;
		
		-- Make sure this is in our spawnables list
		local bFound = false;
		for i=1, table.getn(g_SWEPs) do
			if ( g_SWEPs[i].filename == strFilename ) then bFound = true; end;
		end
		if ( not bFound ) then return end;
		
		
		PlayerLookTrace( iPlayer, 200 );
		if ( _TraceHit() == false ) then return end;
		
		local vSpawnPos = _TraceEndPos();
		local vNormal = _TraceGetSurfaceNormal();
		
		vSpawnPos = vecAdd( vSpawnPos, vecMul( vNormal, 50 ) )
		
		local iEnt = _EntCreate( "weapon_swep" )
			_EntSetPos( iEnt, vSpawnPos )
			_EntSetKeyValue( iEnt, "Script", strFilename )
		_EntSpawn( iEnt )
		_util.DropToFloor( iEnt )
		

		g_SWEPMenu.Players[iPlayer].NextSpawn = _CurTime() + g_SWEPMenu.SpawnRate;
		
	end


	PopulateSWEPList();
	HookEvent( "eventPlayerInitialSpawn", PlayerSendSWEPMenu );
	CONCOMMAND( "SWEPSpawn", cc_SpawnSWEP, "Spawn SWEP weapon; Syntax: <filename>" );
	
end
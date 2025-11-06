-- =========================
-- football weapon v1.0
-- created by g33k
-- Oct 15, 2005
--
-- You have my permission to edit this script
-- =========================

hasBall = false;

-- Called when the weapon is created.
	
	function onInit( )
		_SWEPSetSound( MyIndex, "single_shot", "Weapon_PhysCannon.Launch" )
	end

	
-- Called every frame

	function onThink( )
		-- nil
	end
	
	
	function onPrimaryAttack( )
		if ( _PlayerInfo( Owner, "alive" ) == false ) then return; end
		
		if hasBall then
			local power = 9999;
			
			local ball = _EntCreate( "prop_physics" );
		
			if (ball > 0) then
				_EntSetKeyValue(ball, "model", "models/bm/basketball.mdl");
				_EntSetKeyValue(ball, "targetname", "theball01");
				local spawnpos = vecAdd( _PlayerGetShootPos(Owner), _PlayerGetShootAng(Owner));
				_EntSetPos(ball, spawnpos);
				_EntSetAng(ball, _PlayerGetShootAng(Owner));
			
				local fireforce = vecMul(_PlayerGetShootAng(Owner), vector3(power, power, power));
				_EntSpawn(ball);
				_PhysApplyForce(ball, fireforce);
			end
			hasBall = false;
			_RunString("setPlayerHasBall(0)");
			return;
		end
		
	   local vecpos = _PlayerGetShootPos( Owner );
	   local plyang = _PlayerGetShootAng( Owner );
	 
	   _TraceLine( vecpos, plyang, 80, Owner );
	   
	   vecpos = vecAdd( vecpos, vector3( 0, 0, -20 ) );
	   
	   local hitpos = _TraceEndPos();
	 
	   if ( _TraceHit() == false )  then
	   
	   hitpos = vecAdd( vecpos, vecMul( plyang, vector3(8000,8000,8000) ) )
	   
	   end
		
		_EffectInit();
			_EffectSetEnt( 255 );
			_EffectSetOrigin( hitpos );
			_EffectSetStart( vecpos );
			_EffectSetScale( 40 );
			_EffectSetMagnitude( 0.5 );
		_EffectDispatch( "FadingLine" );
		
		if ( _TraceHit() == true )  then 
			if (_TraceHitNonWorld()) and (_TraceGetEnt() > _MaxPlayers())
				and (_TraceGetEnt() == _EntGetByName("theball01"))
			then 
				_EntRemove(_TraceGetEnt());
				hasBall = true;
				_RunString("setPlayerHasBall(" .. Owner .. ")");
				--_EntFire(Owner, "color", "255 0 0", 0);
			elseif (_TraceGetEnt() <= _MaxPlayers()) then
				local power = 999999;
				local fireforce = vecMul(_PlayerGetShootAng(Owner), vector3(power, power, power));
				local playerid = _TraceGetEnt();
				_PhysApplyForce(playerid, fireforce);
				if (_PlayerInfo(playerid, "team") ~= _PlayerInfo(Owner, "team")) then
					_PlayerKill(playerid);
				end
			else
				return
			end
		else
			return
		end
	end
	
	
	function onSecondaryAttack( )
		if ( _PlayerInfo( Owner, "alive" ) == false ) then return; end
		
		if hasBall then
			local power = 4000;
			
			local ball = _EntCreate( "prop_physics" );
		
			if (ball > 0) then
				_EntSetKeyValue(ball, "model", "models/bm/basketball.mdl");
				_EntSetKeyValue(ball, "targetname", "theball01");
				local spawnpos = vecAdd( _PlayerGetShootPos(Owner), _PlayerGetShootAng(Owner));
				_EntSetPos(ball, spawnpos);
				_EntSetAng(ball, _PlayerGetShootAng(Owner));
			
				local fireforce = vecMul(_PlayerGetShootAng(Owner), vector3(power, power, power));
				_EntSpawn(ball);
				_PhysApplyForce(ball, fireforce);
			end
			hasBall = false;
			_RunString("setPlayerHasBall(0)");
			return;
		end
		
	   local vecpos = _PlayerGetShootPos( Owner );
	   local plyang = _PlayerGetShootAng( Owner );
	 
	   _TraceLine( vecpos, plyang, 80, Owner );
	   
	   vecpos = vecAdd( vecpos, vector3( 0, 0, -20 ) );
	   
	   local hitpos = _TraceEndPos();
	 
		if ( _TraceHit() == false )  then
			hitpos = vecAdd( vecpos, vecMul( plyang, vector3(8000,8000,8000) ) )
		end
		
		_EffectInit();
			_EffectSetEnt(255);
			_EffectSetOrigin( hitpos );
			_EffectSetStart( vecpos );
			_EffectSetScale( 40 );
			_EffectSetMagnitude( 0.5 );
		_EffectDispatch( "FadingLine" );
		
		if ( _TraceHit() == true )  then 
			if (_TraceHitNonWorld()) and (_TraceGetEnt() > _MaxPlayers())
				and (_TraceGetEnt() == _EntGetByName("theball01"))
			then 
				_EntRemove(_TraceGetEnt());
				hasBall = true;
				_RunString("setPlayerHasBall(" .. Owner .. ")");
				--_EntFire(Owner, "color", "255 0 0", 0);
			elseif (_TraceGetEnt() <= _MaxPlayers()) then
				local power = 999999;
				local fireforce = vecMul(_PlayerGetShootAng(Owner), vector3(power, power, power));
				local playerid = _TraceGetEnt();
				_PhysApplyForce(playerid, fireforce);
				if (_PlayerInfo(playerid, "team") ~= _PlayerInfo(Owner, "team")) then
					_PlayerKill(playerid);
				end
			else
				 return
			end
		else
			return
		end
	end
	
	
	function onReload( )
		return true;
	end
	
	
	
-- Weapon settings.
-- These are only accessed once when setting the weapon up
	
	function getWeaponSwapHands()
		return false;	
	end
	
	function getWeaponFOV()
		return 60;	
	end
	
	function getWeaponSlot()
		return 1;	
	end
	
	function getWeaponSlotPos()
		return 4;	
	end
	
	function getFiresUnderwater()
		return true;
	end
	
	function getReloadsSingly()
		return false;
	end
	
	function getDamage()
		return 5;
	end
	
	function getPrimaryShotDelay()
		return 0.5;
	end
	
	function getSecondaryShotDelay()
		return 0.5;
	end
	
	function getPrimaryIsAutomatic()
		return false;
	end
	
	function getSecondaryIsAutomatic()
		return false;
	end
	
	function getBulletSpread()
		return vector3( 0.00, 0.00, 0.00 );
	end
	
	function getViewKick()
		return vector3( 0.0, 0.0, 0.0);
	end
	
	function getViewKickRandom()
		return vector3( 0.0, 0.0, 0.0 );
	end

	function getViewModel( )
		return "models/weapons/v_physcannon.mdl";
	end
	
	function getWorldModel( )
		return "models/weapons/w_physics.mdl";
	end
	
	function getClassName()
		return "weapon_football_cannon";
	end

	function getPrimaryAmmoType()
		return;
	end
		
	function getSecondaryAmmoType()
		return;
	end
	
	-- return -1 if it doesn't use clips
	function getMaxClipPrimary()
		return 1;
	end
	
	function getMaxClipSecondary()
		return -1;
	end
	
	-- ammo in gun by default
	function getDefClipPrimary()
		return 1;
	end
	
	function getDefClipSecondary()
		return 1;
	end

	-- pistol, smg, ar2, shotgun, rpg, phys, crossbow, melee, slam, grenade
	function getAnimPrefix()
		return "phys";
	end

	function getPrintName()
		return "Football Cannon";
	end
	
	
	-- 0 = Don't override, shoot bullets, make sound and flash
	-- 1 = Don't shoot bullets but do make flash/sounds
	-- 2 = Only play animations
	-- 3 = Don't do anything
	
	function getPrimaryScriptOverride()
		return 1;
	end

	function getSecondaryScriptOverride()
		return 1;
	end
	
	
	
	function getBulletSpreadSecondary()
		return vector3( 0.0, 0.0, 0.0 );
	end
	
	function getViewKickSecondary()
		return vector3( 0.0, 0.0, 0.0);
	end
	
	function getViewKickRandomSecondary()
		return vector3( 0.0, 0.0, 0.0 );
	end

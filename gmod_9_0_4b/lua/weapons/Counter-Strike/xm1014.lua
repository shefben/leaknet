

-- Called when the weapon is created.

	function onInit( )
		_SWEPSetSound( MyIndex, "single_shot", "Weapon_xm1014.Single" );
	end


-- Weapon settings.
-- These are only accessed once when setting the weapon up


	function getWeaponSwapHands()
		return true;	
	end
	

	function getWeaponFOV()

		return 74;	

	end

	

	function getWeaponSlot()

		return 5;	

	end

	

	function getWeaponSlotPos()

		return 7;	

	end

	

	function getFiresUnderwater()

		return true;

	end

	

	function getReloadsSingly()

		return true;

	end

	

	function getDamage()

		return 7.5;

	end

	

	function getPrimaryShotDelay()

		return 0.3;

	end

	
	function getPrimaryIsAutomatic()

		return true;

	end

		

	function getBulletSpread()

		return vector3( 0.1, 0.1, 0.1 );

	end


	function getNumShotsPrimary()

		return 7;

	end
	

	function getViewKick()

		return vector3( -4.0, 0.0, 0.0);

	end

	

	function getViewKickRandom()

		return vector3( 1.6, 2.0, 1.5 );

	end

	


	function getViewModel( )
		return "models/weapons/v_shot_xm1014.mdl";
	end

	function getWorldModel( )
		return "models/weapons/w_shot_xm1014.mdl";
	end

	

	function getClassName()

		return "weapon_autoshotgun";

	end



	function getPrimaryAmmoType()
		return "Buckshot";
	end


	function getSecondaryAmmoType()
		return "Buckshot";
	end

	

	-- return -1 if it doesn't use clips
	function getMaxClipPrimary()
		return 7;
	end

	function getMaxClipSecondary()
		return -1;
	end

	

	-- ammo in gun by default
	function getDefClipPrimary()
		return 64;
	end

	

	function getDefClipSecondary()
		return -1;
	end



	-- pistol, smg, ar2, shotgun, rpg, phys, crossbow, melee, slam, grenade

	function getAnimPrefix()
		return "xm1014";
	end



	function getPrintName()
		return "xm1014";
	end

	


	-- 0 = Don't override, shoot bullets, make sound and flash

	-- 1 = Don't shoot bullets but do make flash/sounds

	-- 2 = Only play animations

	-- 3 = Don't do anything

	

	function getPrimaryScriptOverride()

		return 0;

	end



	function getSecondaryScriptOverride()

		return 3;

	end

	function getDeathIcon( )
		return "d_xm1014";
	end


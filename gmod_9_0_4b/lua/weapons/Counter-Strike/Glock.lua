

-- Called when the weapon is created.

	function onInit( )
		_SWEPSetSound( MyIndex, "single_shot", "Weapon_glock.Single" );
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

		return 10;

	end

	

	function getPrimaryShotDelay()

		return 0.15;

	end

	

	function getSecondaryShotDelay()

		return 0.35;

	end

	function getNumShotsSecondary()

		return 3;

	end
	

	function getPrimaryIsAutomatic()

		return false;

	end

	

	function getSecondaryIsAutomatic()

		return false;

	end

	

	function getBulletSpread()

		return vector3( 0.01, 0.01, 0.01 );

	end

	

	function getViewKick()

		return vector3( -1.2, 0.0, 0.0);

	end

	

	function getViewKickRandom()

		return vector3( 0.3, 0, 0 );

	end

	

	

	function getBulletSpreadSecondary()

		return vector3( 0.2, 0.2, 0.2 );

	end

	

	function getViewKickSecondary()

		return vector3( 0.1, 0.1, 0.1);

	end

	

	function getViewKickRandomSecondary()

		return vector3( 0.3, 0.2, 0.5 );

	end

	



	function getViewModel( )
		return "models/weapons/v_pist_glock18.mdl";
	end

	function getWorldModel( )
		return "models/weapons/w_pist_glock18.mdl";
	end

	

	function getClassName()

		return "weapon_glock18";

	end



	function getPrimaryAmmoType()

		return "357";

	end

		

	function getSecondaryAmmoType()

		return "357";

	end

	

	-- return -1 if it doesn't use clips

	function getMaxClipPrimary()

		return 20;

	end

	

	function getMaxClipSecondary()

		return 20;

	end

	

	-- ammo in gun by default

	function getDefClipPrimary()

		return 64;

	end

	

	function getDefClipSecondary()

		return 64;

	end



	-- pistol, smg, ar2, shotgun, rpg, phys, crossbow, melee, slam, grenade

	function getAnimPrefix()

		return "pistol";

	end



	function getPrintName()

		return "Glock";

	end

	


	-- 0 = Don't override, shoot bullets, make sound and flash

	-- 1 = Don't shoot bullets but do make flash/sounds

	-- 2 = Only play animations

	-- 3 = Don't do anything

	

	function getPrimaryScriptOverride()

		return 0;

	end



	function getSecondaryScriptOverride()

		return 0;

	end

	function getDeathIcon( )
		return "d_glock";
	end


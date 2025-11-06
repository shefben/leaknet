

-- Called when the weapon is created.

	function onInit( )
		_SWEPSetSound( MyIndex, "single_shot", "Weapon_USP.Single" );
	end

	function getDeathIcon( )
		return "d_9mmhandgun";
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

		return 6;	

	end

	

	function getFiresUnderwater()

		return true;

	end

	

	function getReloadsSingly()

		return false;

	end

	

	function getDamage()

		return 30;

	end

	

	function getPrimaryShotDelay()

		return 0.15;

	end

	

	function getSecondaryShotDelay()

		return 100;

	end

	

	function getPrimaryIsAutomatic()

		return false;

	end

	

	function getSecondaryIsAutomatic()

		return true;

	end

	

	function getBulletSpread()

		return vector3( 0.03, 0.05, 0.02 );

	end

	

	function getViewKick()

		return vector3( -3.0, 0.0, 0.0);

	end

	

	function getViewKickRandom()

		return vector3( 0.0, 0.0, 0.0 );

	end

	

	

	function getBulletSpreadSecondary()

		return vector3( 0.001, 0.001, 0.001 );

	end

	

	function getViewKickSecondary()

		return vector3( 0.0, 0.0, 0.0);

	end

	

	function getViewKickRandomSecondary()

		return vector3( 0.0, 0.0, 0.0 );

	end

	



	function getViewModel( )
		return "models/weapons/v_pist_usp.mdl";
	end

	function getWorldModel( )
		return "models/weapons/w_pist_usp.mdl";
	end

	

	function getClassName()

		return "weapon_usp";

	end



	function getPrimaryAmmoType()

		return "357";

	end

		

	function getSecondaryAmmoType()

		return "357";

	end

	

	-- return -1 if it doesn't use clips

	function getMaxClipPrimary()

		return 12;

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

		return "pistol";

	end



	function getPrintName()

		return "USP";

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




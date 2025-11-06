--##############################
--## Hide and Seek
--## fayte, gdhughes@gmail.com
--##############################

--## A lot of stuff taken from gm_laserdance by garry

MyIndex					= 0;
Owner 					= 0;
CurrentTime 				= 0;

function onInit()
	_SWEPSetSound(MyIndex, "single_shot", "Weapon_G3SG1.Single")
end
	
function onPrimaryAttack()
	_SWEPUseAmmo(MyIndex, 0, 1)
	_RunString("ShootPrimarySuperCatcher(" .. Owner .. ", " .. MyIndex .. ")")
end
	
function onSecondaryAttack() return false; end
function getPrintName() return "SuperCatcher 6000"; end
function getClassName() return "supercatchergun"; end
function getPrimaryAmmoType() return "sniperround"; end
function getSecondaryAmmoType() return "sniperround"; end
function getViewModel() return "models/weapons/v_rif_sg552.mdl"; end
function getWorldModel() return "models/weapons/w_rif_sg552.mdl"; end
function getAnimPrefix() return "pistol"; end
function getWeaponSwapHands() return true; end
function getWeaponFOV() return 80; end
function getWeaponSlot() return 1; end
function getWeaponSlotPos() return 2; end
function getFiresUnderwater() return true; end
function getReloadsSingly() return false; end
function getDamage() return 0; end
function getPrimaryShotDelay() return 1; end
function getSecondaryShotDelay() return 1; end
function getPrimaryIsAutomatic() return false; end
function getSecondaryIsAutomatic() return false; end
function getMaxClipPrimary() return 1000; end
function getMaxClipSecondary() return 1000; end
function getDefClipPrimary() return 1; end
function getDefClipSecondary() return 0; end
function getPrimaryScriptOverride() return 1; end
function getSecondaryScriptOverride() return 3; end
function getBulletSpread() return vector3(0.0, 0.0, 0.0); end
function getViewKick() return vector3(0, 0.0, 0.0); end
function getViewKickRandom() return vector3(0.0, 0.0, 0.0); end
